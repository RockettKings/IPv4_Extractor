// src/main.cpp
//
// IPv4 address extractor. Implements the specification in DESIGN.md:
// the grammar of section 3.2, strictness rule S1 (section 3.3, the first
// valid run on the line wins) and strictness rule S2 (section 3.4, a run
// followed by whitespace and then '.' or ':' is a near miss and loses).
//
// Every number is built up by hand, one digit at a time.

#include <iostream>
#include <string>
#include <cstddef>

// True when c is an ASCII decimal digit.
// The byte is compared directly, so a byte such as 0xFF (which is negative
// when char is signed) is never handed to a <cctype> lookup table.
static bool isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

// True when c may appear inside a candidate run: a digit, '.' or ':'.
// Every other byte is garbage and separates one run from the next (3.1).
static bool isTokenChar(char c) {
    return isDigitChar(c) || c == '.' || c == ':';
}

// True for the whitespace bytes that rule S2 steps over after a run (3.4).
// '\n' is not listed because getline never leaves one inside the line.
static bool isSpaceChar(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

// Reads one octet or one port out of s, starting at i and stopping at end.
// Enforces the digit count, the leading zero rule and the value range from
// section 3.2, and advances i past the digits it consumed. Returns false for
// an empty number, a leading zero, too many digits, or a value out of range.
static bool readNumber(const std::string& s, std::size_t& i, std::size_t end,
                       int maxDigits, long maxValue, long& value) {
    value = 0;
    int count = 0;
    std::size_t first = i;

    while (i < end && isDigitChar(s[i])) {
        // Safeguard 1: digit limit before accumulation. The count is checked
        // before the multiply, so a run of any length cannot overflow.
        if (count == maxDigits) {
            return false;
        }
        // Safeguard 2: wide accumulator. value is a long, so a port of up to
        // 99999 is held exactly before the range check below.
        value = value * 10 + (s[i] - '0');
        ++count;
        ++i;
    }

    if (count == 0) {
        return false;                       // empty octet or empty port
    }
    if (count > 1 && s[first] == '0') {
        return false;                       // leading zero: 01, 00, 080
    }
    // Safeguard 4: range check on the full value. Nothing is masked, so 257
    // stays 257 and 65616 stays 65616 instead of wrapping to 1 and 80.
    if (value > maxValue) {
        return false;
    }
    return true;
}

// Applies the section 3.2 grammar to the single run [begin, end).
// The run is judged whole: it is never trimmed, split, or searched for a
// valid substring inside it. On success addr holds the 32 bit value and port
// holds the port number, or -1 when the run carried no port.
static bool validateCandidate(const std::string& s, std::size_t begin, std::size_t end,
                              unsigned long& addr, int& port) {
    std::size_t i = begin;
    long octet[4] = {0, 0, 0, 0};

    for (int k = 0; k < 4; ++k) {
        if (k > 0) {
            if (i >= end || s[i] != '.') {
                return false;               // missing '.', or ':' where '.' belongs
            }
            ++i;
        }
        if (!readNumber(s, i, end, 3, 255, octet[k])) {
            return false;
        }
    }

    if (i == end) {
        port = -1;                          // no port on this run
    } else {
        if (s[i] != ':') {
            return false;                   // a 5th octet, or a trailing '.'
        }
        ++i;
        long p = 0;
        if (!readNumber(s, i, end, 5, 65535, p)) {
            return false;                   // a bad port rejects the whole run
        }
        if (i != end) {
            return false;                   // anything at all after the port
        }
        port = (int)p;
    }

    // Safeguard 3: unsigned shift. Each octet is widened to unsigned long
    // before shifting, because shifting 128 left by 24 in a signed 32 bit
    // int is not defined behaviour.
    addr = ((unsigned long)octet[0] << 24) | ((unsigned long)octet[1] << 16)
         | ((unsigned long)octet[2] << 8)  |  (unsigned long)octet[3];
    return true;
}

// Scans str left to right and reports the first run that passes both the
// grammar and rule S2. Returns true and fills outAddress and outPort on
// success; returns false with outAddress 0 and outPort -1 otherwise.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    // Safeguard 6: outputs set on every path. They are cleared here, so every
    // failure path leaves outAddress 0 and outPort -1, and they change only
    // when a valid run is actually returned.
    outAddress = 0;
    outPort = -1;

    std::size_t n = str.size();
    std::size_t i = 0;

    while (i < n) {
        if (!isTokenChar(str[i])) {
            ++i;
            continue;
        }

        std::size_t start = i;
        while (i < n && isTokenChar(str[i])) {
            ++i;
        }
        // Safeguard 5: no run restarts. i now sits one past the end of the run
        // and is never rewound, so a rejected run is skipped whole and the
        // scan can never begin again inside it.

        unsigned long a = 0;
        int p = -1;
        if (validateCandidate(str, start, i, a, p)) {
            // Rule S2 (3.4): step over whitespace after the run, and if the
            // next byte is '.' or ':' this run is a near miss. It does not
            // count, so the scan carries on with the run after it.
            std::size_t j = i;
            while (j < n && isSpaceChar(str[j])) {
                ++j;
            }
            if (j < n && (str[j] == '.' || str[j] == ':')) {
                continue;
            }
            // Rule S1 (3.3): the first run to pass both rules wins, and the
            // rest of the line is garbage that is never examined.
            outAddress = a;
            outPort = p;
            return true;
        }
    }

    return false;
}

#ifndef UNIT_TEST
// Reads a line at a time until END or the end of input, printing exactly one
// result line for each line of input (sections 2.2 and 2.3).
int main() {
    std::string line;

    for (;;) {
        std::cout << "Enter a string (or 'END' to quit): ";

        if (!std::getline(std::cin, line)) {
            std::cout << "Program terminated.\n";       // end of input
            return 0;
        }

        // Exactly one trailing '\r' is removed, for Windows line endings.
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        // Case sensitive, nothing trimmed: " END ", "  END" and "end" are all
        // ordinary input and fall through to extractIPv4 below.
        if (line == "END") {
            std::cout << "Program terminated.\n";
            return 0;
        }

        unsigned long address = 0;
        int port = -1;

        // Safeguard 7: success comes from the return value alone. address is
        // never tested against 0, because 0.0.0.0 is a valid address.
        if (extractIPv4(line, address, port)) {
            // Safeguard 8: the dotted form is rebuilt out of the 32 bit value
            // and written with std::cout.
            std::cout << "Extracted IPv4 address: "
                      << ((address >> 24) & 0xFF) << "."
                      << ((address >> 16) & 0xFF) << "."
                      << ((address >> 8) & 0xFF) << "."
                      << (address & 0xFF)
                      << " (decimal value: " << address << ", port: ";
            if (port == -1) {
                std::cout << "none";
            } else {
                std::cout << port;
            }
            std::cout << ")\n";
        } else {
            std::cout << "Invalid input: no valid IPv4 address found\n";
        }
    }
}
#endif

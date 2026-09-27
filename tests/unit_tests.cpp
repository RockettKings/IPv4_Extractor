#define UNIT_TEST
#include "../src/main.cpp"

// Unit tests for extractIPv4, taken from DESIGN.md section 5.1.
// Every input literal and every expected value is copied from that table.
// A "fail" row is ok = false, addr = 0, port = -1.

#include <iostream>
#include <string>
#include <cstddef>

struct TestCase {
    const char* id;
    std::string input;
    bool ok;
    unsigned long addr;
    int port;
};

// Renders an input for the failure message: tabs as \t, and any byte outside
// the printable range 0x20-0x7E as \xHH.
static std::string escapeInput(const std::string& s) {
    static const char* hexDigits = "0123456789ABCDEF";
    std::string out;
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\t') {
            out += "\\t";
        } else if (c >= 0x20 && c <= 0x7E) {
            out += (char)c;
        } else {
            out += "\\x";
            out += hexDigits[(c >> 4) & 0x0F];
            out += hexDigits[c & 0x0F];
        }
    }
    return out;
}

int main() {
    const TestCase cases[] = {
        // S: handout samples
        { "S01", "connecting to 192.168.1.1 now",        true,  3232235777UL, -1    },
        { "S02", "server=10.0.0.255:8080end",            true,  167772415UL,  8080  },
        { "S03", "192a168.1.1.1",                        true,  2818638081UL, -1    },
        { "S04", "192.168.1.1.",                         false, 0UL,          -1    },
        { "S05", "Connection from 192.168.1.1 refused",  true,  3232235777UL, -1    },
        { "S06", "192.168.01.1",                         false, 0UL,          -1    },
        { "S07", "1.2.3.4:99999",                        false, 0UL,          -1    },
        { "S08", "12.34.56",                             false, 0UL,          -1    },
        { "S09", "no number here",                       false, 0UL,          -1    },

        // V: valid addresses
        { "V01", "0.0.0.0",                              true,  0UL,          -1    },
        { "V02", "255.255.255.255",                      true,  4294967295UL, -1    },
        { "V03", "255.255.255.255:65535",                true,  4294967295UL, 65535 },
        { "V04", "128.0.0.1",                            true,  2147483649UL, -1    },
        { "V05", "1.2.3.4:0",                            true,  16909060UL,   0     },
        { "V06", "1.2.3.4:80 is open",                   true,  16909060UL,   80    },
        { "V07", "addr=1.2.3.4;",                        true,  16909060UL,   -1    },
        { "V08", "-1.2.3.4",                             true,  16909060UL,   -1    },
        { "V09", "IP: 10.0.0.1",                         true,  167772161UL,  -1    },
        { "V10", "time 12:30:45 host 10.1.2.3",          true,  167838211UL,  -1    },
        { "V11", "1.2.3 4.5.6.7",                        true,  67438087UL,   -1    },
        { "V12", "\t8.8.8.8\t",                          true,  134744072UL,  -1    },
        { "V13", "\xFF" "192.168.1.1" "\xFF",            true,  3232235777UL, -1    },
        { "V14", "1.2.3.4x:80",                          true,  16909060UL,   -1    },
        { "V15", "1.2.3.4 port:80",                      true,  16909060UL,   -1    },
        { "V16", "Step 1. 10.0.0.1",                     true,  167772161UL,  -1    },
        { "V17", "10.20.30.40:1",                        true,  169090600UL,  1     },

        // O: octet rules
        { "O01", "01.2.3.4",                             false, 0UL,          -1    },
        { "O02", "1.2.3.04",                             false, 0UL,          -1    },
        { "O03", "00.1.1.1",                             false, 0UL,          -1    },
        { "O04", "1.2.3.00",                             false, 0UL,          -1    },
        { "O05", "256.1.1.1",                            false, 0UL,          -1    },
        { "O06", "1.2.3.256",                            false, 0UL,          -1    },
        { "O07", "257.1.1.1",                            false, 0UL,          -1    },
        { "O08", "1000.1.1.1",                           false, 0UL,          -1    },
        { "O09", "0001.1.1.1",                           false, 0UL,          -1    },

        // T: structure
        { "T01", "1.2.3.4.5",                            false, 0UL,          -1    },
        { "T02", "1..2.3.4",                             false, 0UL,          -1    },
        { "T03", ".1.2.3.4",                             false, 0UL,          -1    },
        { "T04", "1.2.3.4..",                            false, 0UL,          -1    },
        { "T05", "1.2.3",                                false, 0UL,          -1    },
        { "T06", "1.2.3.",                               false, 0UL,          -1    },
        { "T07", "...",                                  false, 0UL,          -1    },
        { "T08", "1.2.3:4",                              false, 0UL,          -1    },
        { "T09", "1:2.3.4",                              false, 0UL,          -1    },
        { "T10", "",                                     false, 0UL,          -1    },
        { "T11", "   ",                                  false, 0UL,          -1    },

        // P: ports
        { "P01", "1.2.3.4:65536",                        false, 0UL,          -1    },
        { "P02", "1.2.3.4:65616",                        false, 0UL,          -1    },
        { "P03", "1.2.3.4:123456",                       false, 0UL,          -1    },
        { "P04", "1.2.3.4:",                             false, 0UL,          -1    },
        { "P05", "1.2.3.4::80",                          false, 0UL,          -1    },
        { "P06", "1.2.3.4:80:",                          false, 0UL,          -1    },
        { "P07", "1.2.3.4:80.5",                         false, 0UL,          -1    },
        { "P08", "1.2.3.4:080",                          false, 0UL,          -1    },
        { "P09", "1.2.3.4:00",                           false, 0UL,          -1    },
        { "P10", ":1.2.3.4",                             false, 0UL,          -1    },
        { "P11", "1.2.3.4.:80",                          false, 0UL,          -1    },
        { "P12", "1.2.3.4:.80",                          false, 0UL,          -1    },
        { "P13", "1.2.3.4:8:0",                          false, 0UL,          -1    },

        // N: overflow
        { "N01", "99999999999999999999.1.1.1",           false, 0UL,          -1    },
        { "N02", "1.2.3.4:99999999999999999999",         false, 0UL,          -1    },
        { "N03", "1.2.3.4294967297",                     false, 0UL,          -1    },
        { "N04", "1.2.3.4:4294967376",                   false, 0UL,          -1    },

        // M: rule S1
        { "M01", "10.0.0.1 and 10.0.0.2",                true,  167772161UL,  -1    },
        { "M02", "1.1.1.1 1.1.1.1",                      true,  16843009UL,   -1    },
        { "M03", "1.2.3.4:80 5.6.7.8",                   true,  16909060UL,   80    },
        { "M04", "1.2.3.4x5.6.7.8",                      true,  16909060UL,   -1    },
        { "M05", "1.2.3.4.5 6.7.8.9",                    true,  101124105UL,  -1    },
        { "M06", "a 1.1.1.1 b 2.2.2.2 c 3.3.3.3",        true,  16843009UL,   -1    },

        // D: rule S2
        { "D01", "1.2.3.4 :80",                          false, 0UL,          -1    },
        { "D02", "1.2.3.4\t:80",                         false, 0UL,          -1    },
        { "D03", "1.2.3.4 : 80",                         false, 0UL,          -1    },
        { "D04", "1.2.3.4   . end",                      false, 0UL,          -1    },
        { "D05", "1.2.3.4:80 :90",                       false, 0UL,          -1    },
        { "D06", "1.2.3.4 :80 5.6.7.8",                  true,  84281096UL,   -1    },
        { "D07", "1.2.3.4 .",                            false, 0UL,          -1    },

        // ------------------------------------------------------------------
        // Extra cases proposed by Claude Code (audit)
        //
        // Not part of the DESIGN.md section 5.1 table. These came out of the
        // Prompt 4 audit as the inputs most likely to break this parser. Every
        // expected value below is derived from DESIGN.md, not from what the
        // program currently prints.
        //
        // C01  \v is in the S2 whitespace set (3.4), but no 5.1 case uses it.
        //      Runs are "1.2.3.4" and ":80". Run 1 passes the grammar, then S2
        //      steps over the \v, finds ':' and discards it. Run 2 starts with
        //      ':', which 3.2 rejects. No valid run, so 3.5 gives false, 0, -1.
        //      This is D02 with \v in place of the tab.
        // C02  One run, "1.2.3.4:80", which passes the grammar. S2 steps over
        //      the three trailing spaces and reaches the end of the string,
        //      which 3.4 says "has no effect", so the run stands. 2.1 gives
        //      1*16777216 + 2*65536 + 3*256 + 4 = 16909060, with port 80.
        // C03  ':' is a token character (3.1), so "::10.0.0.1" is one single
        //      maximal run. 3.2 rejects a run that starts with ':' and a run
        //      holding two separators in a row. 3.1 forbids searching inside a
        //      run for a valid substring, so 10.0.0.1 is never considered.
        // C04  Runs are "0.0.0.0" and ":80". 3.2 allows an octet that is
        //      exactly "0", so run 1 passes the grammar, but S2 steps over the
        //      space, finds ':' and discards it. Run 2 starts with ':'. The
        //      failure values from 3.5 are the same numbers a successful
        //      0.0.0.0 produces (V01), so only the return value separates them.
        // C05  One run. Its first octet has 40 digits and 3.2 allows at most
        //      3, so the run is rejected whole, and 3.1 forbids restarting
        //      inside it. Same shape as N01, with double the digits.
        // ------------------------------------------------------------------
        { "C01", "1.2.3.4\v:80",                         false, 0UL,          -1    },
        { "C02", "1.2.3.4:80   ",                        true,  16909060UL,   80    },
        { "C03", "::10.0.0.1",                           false, 0UL,          -1    },
        { "C04", "0.0.0.0 :80",                          false, 0UL,          -1    },
        { "C05", "1111111111111111111111111111111111111111.1.1.1",
                                                         false, 0UL,          -1    },

        // ------------------------------------------------------------------
        // Student cases (Jaycob Campos)
        //
        // Three cases of my own, not in the DESIGN.md section 5.1 table and
        // not proposed by an AI tool. Each prediction was written down before
        // the program was run.
        //
        // U01  Runs: "256.15.145.2" only, since "Server (" and ") ok" are
        //      garbage (3.1). Its first octet is 256 and 3.2 caps an octet at
        //      255, so the run is rejected whole. 3.5 gives false, 0, -1.
        // U02  Runs: "167.67.152.9", then ":", then "255". Run 1 passes the
        //      grammar with no port. S2 looks at the next byte, ')', which is
        //      neither whitespace nor '.' nor ':', so the run stands (3.4).
        //      The port is none, not 255, because 3.2 allows a colon only
        //      directly after the fourth octet, and ')' breaks that.
        //      2.1: 167*16777216 + 67*65536 + 152*256 + 9 = 2806224905.
        // U03  Same shape as U02, and the port is none for the same reason.
        //      2.1: 245*16777216 + 99*65536 + 245*256 + 7 = 4116968711.
        // ------------------------------------------------------------------
        { "U01", "Server (256.15.145.2) ok",             false, 0UL,          -1    },
        { "U02", "Server (167.67.152.9): 255",           true,  2806224905UL, -1    },
        { "U03", "Server (245.99.245.7): 1",             true,  4116968711UL, -1    }
    };

    const std::size_t total = sizeof(cases) / sizeof(cases[0]);
    std::size_t passed = 0;

    for (std::size_t i = 0; i < total; ++i) {
        // Seeded with junk so a pass proves extractIPv4 resets both outputs.
        unsigned long outAddress = 12345;
        int outPort = 999;

        bool ok = extractIPv4(cases[i].input, outAddress, outPort);

        if (ok == cases[i].ok && outAddress == cases[i].addr && outPort == cases[i].port) {
            std::cout << "PASS " << cases[i].id << "\n";
            ++passed;
        } else {
            std::cout << "FAIL " << cases[i].id
                      << ": input=\"" << escapeInput(cases[i].input) << "\""
                      << ", expected (" << (cases[i].ok ? "true" : "false")
                      << ", " << cases[i].addr << ", " << cases[i].port << ")"
                      << ", got (" << (ok ? "true" : "false")
                      << ", " << outAddress << ", " << outPort << ")\n";
        }
    }

    std::cout << passed << "/" << total << " passed\n";
    return (passed == total) ? 0 : 1;
}

# IPv4 Extractor — Design Document & AI Prompts

Sep 27, 2026 · @Jaycob

## 1. How to use this document

This document is the full specification for a C++17 program that extracts one IPv4 address (plus optional port) from a line of text. Give the whole document to an AI tool, then run the prompts in Section 6 in order.

**Authority.** Sections 2–5 are the specification. If generated code disagrees with them, the specification wins and the code is wrong. Where the course handout is ambiguous, the strictness rules S1 and S2 in Section 3 are the author's deliberate, documented choices.

**Workflow**

1. Export this document as Markdown (or copy it) and give the entire text to the AI tool together with Prompt 0.
2. Run Prompt 1 to generate `src/main.cpp`.
3. Run Prompt 2 to generate `tests/unit_tests.cpp`.
4. Run Prompt 3 to generate the end-to-end files and `run_tests.sh`.
5. Run `./run_tests.sh`. For every failure, use Prompt 5; never accept a fix that is not explained.
6. Run Prompt 4 (audit), then personally walk the Section 7 checklist against the code.
7. Record every prompt, response problem, and fix in the disclosure log (Section 7).

**Toolchain:** C++17, `g++ -std=c++17 -Wall -Wextra -pedantic`, standard library only.

**Repository layout**

```text
README.md               how to build, run, and test
DESIGN.md               this document
AI_DISCLOSURE.md        disclosure (Section 7 template)
src/main.cpp            extractIPv4 + main
tests/unit_tests.cpp    unit tests (Section 5.1)
tests/e2e_input.txt     end-to-end input (Section 5.2)
tests/e2e_expected.txt  exact expected stdout (Section 5.2)
run_tests.sh            builds and runs everything
```

## 2. Requirements

The program reads lines until `END`, calls `extractIPv4` on each line, and prints exactly one result line per input.

### 2.1 Function prototype (use exactly)

```cpp
// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort);
```

The 32-bit value of `A.B.C.D` is A × 16777216 + B × 65536 + C × 256 + D.

### 2.2 Exact console text

| When | Exact text printed |
| --- | --- |
| Before every read | ` Enter a string (or 'END' to quit):  ` with one trailing space and no newline |
| Address found | `Extracted IPv4 address: A.B.C.D (decimal value: N, port: P)` then newline |
| No address found | `Invalid input: no valid IPv4 address found` then newline |
| `END` entered, or end of input | `Program terminated.` then newline |

N is the unsigned 32-bit value in decimal. P is the port in decimal, or the literal word `none` when `outPort == -1`.

### 2.3 Input loop rules

- Read each line with `std::getline(std::cin, line)`.
- If `getline` fails (end of input), print `Program terminated.` and return 0. The loop must never spin forever.
- Remove exactly one trailing `'\r'` if present (Windows line endings).
- If the line is then exactly `END`, print `Program terminated.` and return 0. Matching is case-sensitive with no trimming: ` END  `, `  END `, and `end` are ordinary input.
- Otherwise call `extractIPv4`. Success is decided only by its return value, never by checking `outAddress != 0`.
- An empty line is ordinary input and prints the failure message.

### 2.4 Forbidden facilities

- **String-to-number:** `atoi`, `atol`, `atoll`, `strtol`, `strtoll`, `strtoul`, `strtoull`, `strtof`, `strtod`, `stoi`, `stol`, `stoll`, `stoul`, `stoull`, `stof`, `stod`, `sscanf`, `scanf`, `std::from_chars`, and numeric extraction with `>>` from any stream.
- **Address parsing:** `inet_aton`, `inet_pton`, `inet_addr`, `inet_network`, `getaddrinfo`, and the headers `<arpa/inet.h>`, `<netinet/in.h>`, `<winsock2.h>`.
- **Regular expressions:** `std::regex`, `<regex.h>`, or any regex library.
- **Allowed:** character tests, `std::string`, `std::getline`, `std::cout`. Every numeric value is accumulated by hand with `value = value * 10 + (c - '0')`.

## 3. Parsing specification

A line succeeds when at least one run fully matches the grammar and passes rule S2; the first such run is returned and the rest of the line is garbage.

### 3.1 Tokenization

1. **Token characters** are `'0'`–`'9'`, `'.'`, and `':'`. Every other byte is garbage and acts as a separator. This includes letters, whitespace, `-`, `=`, `/`, `;`, `,`, and non-ASCII bytes such as `0xFF`.
2. **A candidate** is a maximal run of consecutive token characters. Every run in the line is a candidate, including runs like `:` or `...`.
3. **Whole-run validation.** Each candidate is validated as a whole. A run is never trimmed, split, or searched for a valid substring. Scanning never restarts inside a run: after a run ends, the next run begins at the next token character after at least one garbage byte.

Examples: `192a168.1.1.1` has two runs, `192` and `168.1.1.1`. `1.2.3.4.5` is one run and is rejected; `2.3.4.5` inside it is never considered.

### 3.2 Grammar

```text
candidate := octet "." octet "." octet "." octet [ ":" port ]
octet     := 1-3 digits, value 0-255,   no leading zero unless the octet is exactly "0"
port      := 1-5 digits, value 0-65535, no leading zero unless the port is exactly "0"
```

A run is rejected if any of the following hold:

- It does not have exactly 4 octets, or any octet is empty.
- It starts or ends with `.` or `:`, or contains two separators in a row (`..`, `.:`, `:.`, `::`).
- An octet has more than 3 digits, a value above 255, or a leading zero (`01`, `00`).
- A colon appears anywhere other than directly after the 4th octet, or more than one colon appears.
- A colon is followed by an empty port, a port with more than 5 digits, a value above 65535, or a leading zero (`080`, `00`).
- Any `.` or `:` follows the port.

If a colon is present and the port is invalid, the whole run is rejected, address included.

### 3.3 Strictness rule S1: the first valid address wins

The handout says exactly one valid address may be extracted per line and everything else is garbage or a failed candidate. So the line is scanned left to right, and the first run that passes the grammar and rule S2 is returned. Everything after it is garbage and is never examined: `10.0.0.1 and 10.0.0.2` returns `10.0.0.1`.

- No valid run: failure.
- First valid run: success; output that address and port.
- Any later valid run: ignored as garbage, even if the addresses are identical (`1.1.1.1 1.1.1.1`).

Invalid runs before it are skipped. `192a168.1.1.1` succeeds because `192` is invalid and `168.1.1.1` is the first valid run.

### 3.4 Strictness rule S2: no detached separator after an address

The grammar allows a colon only immediately after the 4th octet. An address followed by whitespace and then a `.` or `:` is a near-miss (`1.2.3.4 :80`, ` 1.2.3.4 .  `), so it is rejected.

- After a run passes the grammar, skip any whitespace (space, `\t`, `\v`, `\f`, `\r`) that follows it.
- If the next character is `.` or `:`, that run is invalid and does not count toward S1.
- End of string, or any other character (a letter, `,`, `;`, a digit), has no effect.

S2 looks forward only. A colon before an address is normal label punctuation, so `IP: 10.0.0.1` succeeds. Non-whitespace garbage breaks the bridge, so `1.2.3.4x:80` succeeds with port `none`.

### 3.5 Output contract

- **Success:** return `true`, `outAddress` = the 32-bit value, `outPort` = the port or -1.
- **Failure:** return `false`, `outAddress` = 0, `outPort` = -1. This holds on every failure path.

## 4. Implementation design

The code is one file, `src/main.cpp`, split into small helpers so each rule in Section 3 lives in exactly one place.

### 4.1 Functions

| Function | Responsibility |
| --- | --- |
| `static bool isDigitChar(char c)` | `c >= '0' && c <= '9'`. Never calls `isdigit` on a plain `char`. |
| `static bool isTokenChar(char c)` | Digit, `'.'`, or `':'`. |
| `static bool isSpaceChar(char c)` | `' '`, `'\t'`, `'\v'`, `'\f'`, `'\r'`. Used only by rule S2. |
| `static bool readNumber(const std::string& s, std::size_t& i, std::size_t end, int maxDigits, long maxValue, long& value)` | Reads one octet or port starting at `i`, enforcing digit count, leading-zero rule, and range. Advances `i`. |
| `static bool validateCandidate(const std::string& s, std::size_t begin, std::size_t end, unsigned long& addr, int& port)` | Applies the Section 3.2 grammar to the run `[begin, end)`. |
| `bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort)` | Finds runs, applies S2, returns the first valid run (S1), sets outputs. |
| `int main()` | Input loop and output formatting. Wrapped in `#ifndef UNIT_TEST` so tests can include the file. |

### 4.2 Algorithm (pseudocode)

```text
readNumber(s, i, end, maxDigits, maxValue, value):
    value = 0; count = 0; first = i
    while i < end and isDigitChar(s[i]):
        if count == maxDigits: return false        # checked BEFORE accumulating: no overflow
        value = value * 10 + (s[i] - '0'); count++; i++
    if count == 0: return false                   # empty octet/port
    if count > 1 and s[first] == '0': return false # leading zero
    if value > maxValue: return false
    return true

validateCandidate(s, begin, end, addr, port):
    i = begin
    for k in 0..3:
        if k > 0:
            if i >= end or s[i] != '.': return false
            i++
        if not readNumber(s, i, end, 3, 255, octet[k]): return false
    if i == end: port = -1
    else:
        if s[i] != ':': return false               # 5th octet, stray '.', etc.
        i++
        if not readNumber(s, i, end, 5, 65535, p): return false
        if i != end: return false                  # anything after the port
        port = (int)p
    addr = ((unsigned long)octet[0] << 24) | ((unsigned long)octet[1] << 16)
         | ((unsigned long)octet[2] << 8)  |  (unsigned long)octet[3]
    return true

extractIPv4(str, outAddress, outPort):
    outAddress = 0; outPort = -1
    i = 0; n = str.size()
    while i < n:
        if not isTokenChar(str[i]): i++; continue
        start = i
        while i < n and isTokenChar(str[i]): i++  # i now = end of run; never rewound
        if validateCandidate(str, start, i, a, p):
            j = i
            while j < n and isSpaceChar(str[j]): j++
            if j < n and (str[j] == '.' or str[j] == ':'): continue   # rule S2
            outAddress = a; outPort = p; return true                   # rule S1: first valid run wins
    return false
```

### 4.3 Safeguards the code must contain

1. **Digit limit before accumulation.** The digit-count check comes before `value * 10`, so a run of any length cannot overflow.
2. **Wide accumulator.** Values accumulate in `long`. The port (up to 99999 before the range check) must not sit in a type that might be 16 bits.
3. **Unsigned shifts.** Each octet is cast to `unsigned long` before shifting. `128 << 24` on a signed 32-bit `int` is undefined behavior.
4. **Range check on the full value.** No masking with `& 0xFF` or `& 0xFFFF` during parsing, which would turn 257 into 1 or 65616 into 80.
5. **No run restarts.** The scan index only moves forward; a rejected run is skipped whole.
6. **Outputs set on every path.** `outAddress = 0` and `outPort = -1` are set at entry; they change only when a valid run is returned.
7. **Success from the return value.** `main` never tests `outAddress != 0`, because `0.0.0.0` is valid.
8. **Printing.** `main` rebuilds the dotted form with `(outAddress >> 24) & 0xFF`, `(outAddress >> 16) & 0xFF`, `(outAddress >> 8) & 0xFF`, `outAddress & 0xFF`, and prints with `std::cout`, not `printf`.

## 5. Test plan

The suite has 76 unit cases, 3 end-to-end runs, a sanitizer build, and a forbidden-function scan; every expected value below was computed by an independent reference model of Sections 3–4.

### 5.1 Unit tests (`tests/unit_tests.cpp`)

The Input column is the exact C++ string literal to use. "fail" means return `false`, `outAddress == 0`, `outPort == -1`. Before every call, set `outAddress = 12345` and `outPort = 999` so the test proves the function resets them.

ID prefixes: S = handout sample, V = valid, O = octet, T = structure, P = port, N = overflow, M = rule S1, D = rule S2.

| ID | Input (C++ literal) | Expected: address · decimal · port |
| --- | --- | --- |
| S01 | `"connecting to 192.168.1.1 now"` | 192.168.1.1 · 3232235777 · none |
| S02 | `"server=10.0.0.255:8080end"` | 10.0.0.255 · 167772415 · 8080 |
| S03 | `"192a168.1.1.1"` | 168.1.1.1 · 2818638081 · none |
| S04 | `"192.168.1.1."` | fail |
| S05 | `"Connection from 192.168.1.1 refused"` | 192.168.1.1 · 3232235777 · none |
| S06 | `"192.168.01.1"` | fail |
| S07 | `"1.2.3.4:99999"` | fail |
| S08 | `"12.34.56"` | fail |
| S09 | `"no number here"` | fail |
| V01 | `"0.0.0.0"` | 0.0.0.0 · 0 · none |
| V02 | `"255.255.255.255"` | 255.255.255.255 · 4294967295 · none |
| V03 | `"255.255.255.255:65535"` | 255.255.255.255 · 4294967295 · 65535 |
| V04 | `"128.0.0.1"` | 128.0.0.1 · 2147483649 · none |
| V05 | `"1.2.3.4:0"` | 1.2.3.4 · 16909060 · 0 |
| V06 | `"1.2.3.4:80 is open"` | 1.2.3.4 · 16909060 · 80 |
| V07 | `"addr=1.2.3.4;"` | 1.2.3.4 · 16909060 · none |
| V08 | `"-1.2.3.4"` | 1.2.3.4 · 16909060 · none |
| V09 | `"IP: 10.0.0.1"` | 10.0.0.1 · 167772161 · none |
| V10 | `"time 12:30:45 host 10.1.2.3"` | 10.1.2.3 · 167838211 · none |
| V11 | `"1.2.3 4.5.6.7"` | 4.5.6.7 · 67438087 · none |
| V12 | `"\t8.8.8.8\t"` | 8.8.8.8 · 134744072 · none |
| V13 | `"\xFF" "192.168.1.1" "\xFF"` | 192.168.1.1 · 3232235777 · none |
| V14 | `"1.2.3.4x:80"` | 1.2.3.4 · 16909060 · none |
| V15 | `"1.2.3.4 port:80"` | 1.2.3.4 · 16909060 · none |
| V16 | `"Step 1. 10.0.0.1"` | 10.0.0.1 · 167772161 · none |
| V17 | `"10.20.30.40:1"` | 10.20.30.40 · 169090600 · 1 |
| O01 | `"01.2.3.4"` | fail |
| O02 | `"1.2.3.04"` | fail |
| O03 | `"00.1.1.1"` | fail |
| O04 | `"1.2.3.00"` | fail |
| O05 | `"256.1.1.1"` | fail |
| O06 | `"1.2.3.256"` | fail |
| O07 | `"257.1.1.1"` | fail |
| O08 | `"1000.1.1.1"` | fail |
| O09 | `"0001.1.1.1"` | fail |
| T01 | `"1.2.3.4.5"` | fail |
| T02 | `"1..2.3.4"` | fail |
| T03 | `".1.2.3.4"` | fail |
| T04 | `"1.2.3.4.."` | fail |
| T05 | `"1.2.3"` | fail |
| T06 | `"1.2.3."` | fail |
| T07 | `"..."` | fail |
| T08 | `"1.2.3:4"` | fail |
| T09 | `"1:2.3.4"` | fail |
| T10 | `""` | fail |
| T11 | `"   "` | fail |
| P01 | `"1.2.3.4:65536"` | fail |
| P02 | `"1.2.3.4:65616"` | fail |
| P03 | `"1.2.3.4:123456"` | fail |
| P04 | `"1.2.3.4:"` | fail |
| P05 | `"1.2.3.4::80"` | fail |
| P06 | `"1.2.3.4:80:"` | fail |
| P07 | `"1.2.3.4:80.5"` | fail |
| P08 | `"1.2.3.4:080"` | fail |
| P09 | `"1.2.3.4:00"` | fail |
| P10 | `":1.2.3.4"` | fail |
| P11 | `"1.2.3.4.:80"` | fail |
| P12 | `"1.2.3.4:.80"` | fail |
| P13 | `"1.2.3.4:8:0"` | fail |
| N01 | `"99999999999999999999.1.1.1"` | fail |
| N02 | `"1.2.3.4:99999999999999999999"` | fail |
| N03 | `"1.2.3.4294967297"` | fail |
| N04 | `"1.2.3.4:4294967376"` | fail |
| M01 | `"10.0.0.1 and 10.0.0.2"` | 10.0.0.1 · 167772161 · none |
| M02 | `"1.1.1.1 1.1.1.1"` | 1.1.1.1 · 16843009 · none |
| M03 | `"1.2.3.4:80 5.6.7.8"` | 1.2.3.4 · 16909060 · 80 |
| M04 | `"1.2.3.4x5.6.7.8"` | 1.2.3.4 · 16909060 · none |
| M05 | `"1.2.3.4.5 6.7.8.9"` | 6.7.8.9 · 101124105 · none |
| M06 | `"a 1.1.1.1 b 2.2.2.2 c 3.3.3.3"` | 1.1.1.1 · 16843009 · none |
| D01 | `"1.2.3.4 :80"` | fail |
| D02 | `"1.2.3.4\t:80"` | fail |
| D03 | `"1.2.3.4 : 80"` | fail |
| D04 | `"1.2.3.4   . end"` | fail |
| D05 | `"1.2.3.4:80 :90"` | fail |
| D06 | `"1.2.3.4 :80 5.6.7.8"` | 5.6.7.8 · 84281096 · none |
| D07 | `"1.2.3.4 ."` | fail |

What the tricky cases catch: V01 (success judged by `outAddress != 0`), V04 (signed shift), V13 (`isdigit` on a negative `char`), O07 and P02 (masking instead of range checks: 257 → 1, 65616 → 80), O01 and T01 (restarting inside a rejected run), N01–N04 (overflow and 32-bit wraparound), M01–M06 and D06 (invalid runs are skipped and the first valid run wins).

### 5.2 End-to-end tests

`tests/e2e_input.txt` (18 lines; line 14 is empty, line 16 starts with a space, and line 18 must never be read because `END` comes first):

```text
connecting to 192.168.1.1 now
server=10.0.0.255:8080end
192a168.1.1.1
192.168.1.1.
Connection from 192.168.1.1 refused
192.168.01.1
1.2.3.4:99999
12.34.56
no number here
0.0.0.0
255.255.255.255:65535
1.2.3.4 :80
10.0.0.1 and 10.0.0.2

end
 END
END
8.8.8.8
```

`tests/e2e_expected.txt` (exact stdout; prompts share a line with results because redirected input is not echoed; file ends with one newline):

```text
Enter a string (or 'END' to quit): Extracted IPv4 address: 192.168.1.1 (decimal value: 3232235777, port: none)
Enter a string (or 'END' to quit): Extracted IPv4 address: 10.0.0.255 (decimal value: 167772415, port: 8080)
Enter a string (or 'END' to quit): Extracted IPv4 address: 168.1.1.1 (decimal value: 2818638081, port: none)
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Extracted IPv4 address: 192.168.1.1 (decimal value: 3232235777, port: none)
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Extracted IPv4 address: 0.0.0.0 (decimal value: 0, port: none)
Enter a string (or 'END' to quit): Extracted IPv4 address: 255.255.255.255 (decimal value: 4294967295, port: 65535)
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Extracted IPv4 address: 10.0.0.1 (decimal value: 167772161, port: none)
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): Program terminated.
```

Two more runs are built into the script rather than stored as files:

- **EOF without END:** input `1.2.3.4` then end of input. Expected: the `1.2.3.4` success line (decimal 16909060, port none), then a prompt followed by `Program terminated.`
- **Windows line endings:** input `192.168.1.1\r\nEND\r\n`. Expected: the `192.168.1.1` success line, then a prompt followed by `Program terminated.`

### 5.3 `run_tests.sh`

The script stops at the first failing step and exits non-zero.

1. Build `ipv4` from `src/main.cpp` with `g++ -std=c++17 -Wall -Wextra -pedantic -Werror`.
2. Build `unit_tests` from `tests/unit_tests.cpp` twice: once normally, once with `-fsanitize=address,undefined -fno-sanitize-recover=all`. Run both; any failed case or sanitizer report fails the step.
3. Run `./ipv4 < tests/e2e_input.txt` and `diff` against `tests/e2e_expected.txt`.
4. Run the EOF and Windows-line-ending checks from 5.2 using `printf` piped into `./ipv4`, diffed against expected text.
5. Scan `src/main.cpp` with `grep -nwE` for every name in Section 2.4 plus `stringstream`, `istringstream`, and `regex`. Any match fails the step.
6. Print `ALL TESTS PASSED`.

## 6. Prompts

Run Prompts 0–5 in order in one conversation, after giving the AI tool this entire document. Copy each prompt exactly and paste the exact text into the disclosure.

### Prompt 0: context (send with the document)

```text
The attached DESIGN.md is the complete, authoritative specification for a C++17 program. Read all of it. Do not write any code yet. Reply with (1) a five-sentence summary of rules S1 and S2 in your own words, and (2) any statement in the document you believe is contradictory or unclear. Do not resolve anything silently.
```

### Prompt 1: generate `src/main.cpp`

```text
Write src/main.cpp exactly as specified in DESIGN.md.

- Implement the functions in Section 4.1 with the exact signatures shown, following the pseudocode in Section 4.2.
- Implement every safeguard in Section 4.3. At each one, add a short comment naming it (for example: // Safeguard 3: unsigned shift).
- Implement rules S1 (Section 3.3) and S2 (Section 3.4) exactly. Do not relax, extend, or reinterpret them.
- main() follows Section 2.3 and prints the strings in Section 2.2 character for character, including the trailing space in the prompt. Wrap main() in #ifndef UNIT_TEST / #endif.
- Use nothing from Section 2.4. Allowed headers: <iostream>, <string>, <cstddef>.
- It must compile with g++ -std=c++17 -Wall -Wextra -pedantic -Werror.
- Put a brief comment above each function describing its job.

Output the complete file with no placeholders. After the code, trace cases S03, V01, V04, O01, M05, and D01 from Section 5.1 by hand: list every run the scanner finds, whether each passes the grammar and S2, the final count, and the result.
```

### Prompt 2: generate `tests/unit_tests.cpp`

```text
Write tests/unit_tests.cpp from DESIGN.md Section 5.1.

- The first two lines are #define UNIT_TEST and #include "../src/main.cpp".
- Use struct TestCase { const char* id; std::string input; bool ok; unsigned long addr; int port; }.
- Include all 76 cases in table order. Copy each Input literal verbatim. For "fail" rows use ok = false, addr = 0, port = -1. For success rows, addr is the decimal value and port is -1 when the table says none.
- Before each call, set outAddress = 12345 and outPort = 999.
- Compare the return value, outAddress, and outPort. Print "PASS <id>" or "FAIL <id>: input=<escaped>, expected (ok, addr, port), got (ok, addr, port)". In the escaped input, show tabs as \t and bytes outside 0x20-0x7E as \xHH.
- End by printing "<passed>/<total> passed" and return 0 only if every case passed.
- Do not add, drop, reorder, or change any expected value. If you think a table value is wrong, keep it and explain why after the code.
```

### Prompt 3: end-to-end files, script, README

```text
Using DESIGN.md Sections 1, 5.2, and 5.3, create:

1. tests/e2e_input.txt: the 18 lines exactly. Line 14 is empty, line 16 begins with one space. End with one newline.
2. tests/e2e_expected.txt: the text exactly, ending with one newline.
3. run_tests.sh: bash with set -euo pipefail, implementing steps 1-6 of Section 5.3 in order, printing a header before each step and stopping on the first failure.
4. README.md: how to build, run interactively, and run the tests.

Show each file in full.
```

### Prompt 4: audit (after all tests pass)

```text
Audit src/main.cpp against DESIGN.md. Do not rewrite it. For each item, answer PASS or FAIL, cite line numbers, and give a one-sentence reason:

1. Each of the 8 safeguards in Section 4.3.
2. Rules S1 and S2.
3. Each rejection condition in Section 3.2.
4. Each string in Section 2.2, compared character by character.
5. Each rule in Section 2.3 (EOF, trailing '\r', exact END match).
6. Absence of every facility in Section 2.4, including indirect ones such as stream >> into a number.
7. Any undefined behavior, signed/unsigned mismatch, or out-of-bounds index.

Then propose five inputs not in Section 5.1 that are most likely to break this code, with the expected result for each according to the specification.
```

### Prompt 5: fix one failure (reuse for each)

```text
Test <ID> failed.
Input: <C++ literal from Section 5.1>
Expected: <from Section 5.1>
Actual: <paste the test output>

Trace this input through src/main.cpp step by step and identify the exact line that produces the wrong result. Explain the root cause. Then give the smallest change that fixes it and list which other Section 5.1 cases the change could affect. Do not change the specification or any expected value.
```

## 7. Review checklist and AI disclosure

The grade depends on catching AI errors and documenting them, so this section is completed by the student, not the AI tool.

### 7.1 Grading map

| Rubric item | Points | Covered by |
| --- | --- | --- |
| Accuracy | 20 | Sections 3–4 (spec), 5 (76 unit cases, e2e, sanitizers), 7.2 checklist |
| GAI disclosure quality | 20 | Section 7.3 template, exact prompts from Section 6, fix log |
| User experience | 10 | Sections 2.2–2.3 (exact strings, loop until `END`), 5.2 end-to-end diff |

### 7.2 Student review checklist

- [x] `./run_tests.sh` prints `ALL TESTS PASSED`, including the sanitizer build.
- [x] The handout's sample run reproduces exactly when typed interactively.
- [x] I walked through S03, O01, M05, and D01 in the code step by step, with Claude (claude.ai) guiding.
- [x] I read every line of `src/main.cpp` and can explain each safeguard in Section 4.3.
- [x] I searched the code myself for every name in Section 2.4.
- [x] I added three test cases of my own (U01-U03 in `tests/unit_tests.cpp`), predicting each result by hand before running it.
- [x] Every AI mistake I found is logged in the disclosure with its test ID, root cause, and fix.

### 7.3 `AI_DISCLOSURE.md` template

```markdown
# AI Disclosure: IPv4 Extractor

## Tools and dates
- Claude Opus 5.5 (Anthropic, claude.ai), 2026-09-27: drafted DESIGN.md, the
  Section 5 test table, and the Section 6 prompts. I chose strictness rules
  S1 (first valid address wins) and S2 (no detached ':' or '.' after an address).
- <code-generation tool + version>, <dates>: generated code from the prompts.

## Prompts
<Exact text of every prompt sent, in order, including follow-ups.>

## Attribution
| File / function | AI-generated | Written or modified by me |
| --- | --- | --- |

## Problems found and fixes
| # | Test ID / symptom | Root cause | Fix | Found by |
| --- | --- | --- | --- | --- |

## Design decisions
<Why S1 and S2 were chosen, quoting the handout sentences they rest on.>

## Verification statement
I understand every line of the submitted code. It has been tested with the
suite in tests/ and works as intended.
Known limitations: <list, or "none known">
```

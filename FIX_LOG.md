# Fix Log — IPv4 Extractor

Log of every failure found while running `./run_tests.sh`, as required by
DESIGN.md section 5 and the disclosure template in section 7.3.

## Result of the first run

**No failures.** The suite passed on its first execution, with no changes made
to `src/main.cpp` after it was generated from DESIGN.md Prompt 1.

| Step | What ran | Result |
| --- | --- | --- |
| 1 | `g++ -std=c++17 -Wall -Wextra -pedantic -Werror src/main.cpp` | built, no warnings |
| 2 | 76 unit cases, plain build | `76/76 passed` |
| 2 | 76 unit cases, `-fsanitize=address,undefined -fno-sanitize-recover=all` | `76/76 passed`, no sanitizer report |
| 3 | `./ipv4 < tests/e2e_input.txt` diffed against `tests/e2e_expected.txt` | identical |
| 4 | end of input without `END`; Windows `\r\n` line endings | both identical to expected |
| 5 | forbidden facility scan of `src/main.cpp` | no match |
| 6 | — | `ALL TESTS PASSED`, exit code 0 |

Because nothing failed, DESIGN.md Prompt 5 was never needed and the table
below is empty.

## Problems found and fixes

| # | Test ID / symptom | Root cause | Fix | Found by |
| --- | --- | --- | --- | --- |
| — | none | — | — | — |

## Notes on things that did not become failures

These were caught while reading DESIGN.md, before any code was written, so
they never produced a failing test. They are recorded because each one is a
trap the test table is specifically aimed at.

1. **`0.0.0.0` (V01).** `main` decides success from the return value of
   `extractIPv4` only. Testing `address != 0` instead would have reported a
   valid address as invalid.
2. **`128.0.0.1` (V04).** Each octet is cast to `unsigned long` before it is
   shifted. Shifting `128` left by 24 inside a signed 32 bit `int` is
   undefined behaviour, and the sanitizer build is what would have caught it.
3. **`257.1.1.1` (O07) and `1.2.3.4:65616` (P02).** The accumulated value is
   range checked whole. Masking with `& 0xFF` or `& 0xFFFF` would have turned
   257 into 1 and 65616 into 80, and both cases would have wrongly passed.
4. **`\xFF192.168.1.1\xFF` (V13).** `isDigitChar` compares the byte directly
   rather than going through a `<cctype>` lookup, which is undefined for a
   negative `char`.
5. **`0001.1.1.1` (O09) and `1.2.3.4:99999999999999999999` (N02).** The digit
   count is checked before the multiply in `readNumber`, so no input length
   can overflow the accumulator.
6. **`:1.2.3.4` (P10) and `1.2.3.4.5` (T01).** A run is validated whole and
   the scan index is never rewound, so the parser cannot find `1.2.3.4` inside
   a run that was already rejected.

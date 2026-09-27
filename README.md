# IPv4 Extractor

A C++17 program that reads lines of text and extracts the first valid IPv4
address, with an optional port, from each one. The full specification lives in
[DESIGN.md](DESIGN.md); this file only covers building, running and testing.

Everything is parsed by hand with character tests. No conversion function,
address parsing function, or regular expression is used anywhere
(DESIGN.md section 2.4).

## Layout

```text
README.md               this file
DESIGN.md               the specification
AI_DISCLOSURE.md        disclosure of how AI was used
src/main.cpp            extractIPv4 and main
tests/unit_tests.cpp    76 unit cases from DESIGN.md section 5.1
tests/e2e_input.txt     end to end input
tests/e2e_expected.txt  exact expected stdout
run_tests.sh            builds and runs everything
build/                  binaries and test output, created by run_tests.sh
```

## Build

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Werror src/main.cpp -o ipv4
```

## Run interactively

```bash
./ipv4
```

The program prints a prompt, reads one line, and prints one result line, over
and over until you type `END` or the input ends. `END` is matched exactly:
`end` and ` END ` are ordinary input.

```text
Enter a string (or 'END' to quit): connecting to 192.168.1.1 now
Extracted IPv4 address: 192.168.1.1 (decimal value: 3232235777, port: none)
Enter a string (or 'END' to quit): server=10.0.0.255:8080end
Extracted IPv4 address: 10.0.0.255 (decimal value: 167772415, port: 8080)
Enter a string (or 'END' to quit): 12.34.56
Invalid input: no valid IPv4 address found
Enter a string (or 'END' to quit): END
Program terminated.
```

You can also pipe a file in:

```bash
./ipv4 < tests/e2e_input.txt
```

## Run the tests

```bash
./run_tests.sh
```

The script stops at the first failure and exits non-zero. It runs six steps:

1. Builds `src/main.cpp` with `-Wall -Wextra -pedantic -Werror`.
2. Builds and runs the 76 unit cases twice, once plainly and once with the
   address and undefined behaviour sanitizers.
3. Diffs a full run against `tests/e2e_expected.txt`.
4. Checks end of input without `END`, and Windows `\r\n` line endings.
5. Scans `src/main.cpp` for every forbidden facility in DESIGN.md section 2.4.
6. Prints `ALL TESTS PASSED`.

To run just the unit tests:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -Werror tests/unit_tests.cpp -o unit_tests
./unit_tests
```

A failing case prints the test ID, the escaped input, and the expected and
actual `(ok, address, port)` triples.

## CMake

`CMakeLists.txt` builds the same program as the `IPv4_Extractor` target, for
use inside CLion. It is a convenience only; `run_tests.sh` does not use it.

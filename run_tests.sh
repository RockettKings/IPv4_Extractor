#!/usr/bin/env bash
#
# Builds and runs everything described in DESIGN.md section 5.3.
# Stops at the first failing step and exits non-zero.

set -euo pipefail

CXX="${CXX:-g++}"
CXXFLAGS="-std=c++17 -Wall -Wextra -pedantic -Werror"
BUILD="build"

cd "$(dirname "$0")"
mkdir -p "$BUILD"

step() {
    echo
    echo "=============================================================="
    echo "$1"
    echo "=============================================================="
}

# ---------------------------------------------------------------- step 1
step "Step 1: build ipv4 from src/main.cpp"
# shellcheck disable=SC2086
"$CXX" $CXXFLAGS src/main.cpp -o "$BUILD/ipv4"
echo "built $BUILD/ipv4"

# ---------------------------------------------------------------- step 2
step "Step 2: build and run the unit tests, plain then sanitized"
# shellcheck disable=SC2086
"$CXX" $CXXFLAGS tests/unit_tests.cpp -o "$BUILD/unit_tests"
echo "--- plain build ---"
"$BUILD/unit_tests"

# shellcheck disable=SC2086
"$CXX" $CXXFLAGS -g -fsanitize=address,undefined -fno-sanitize-recover=all \
    tests/unit_tests.cpp -o "$BUILD/unit_tests_san"
echo "--- address and undefined behaviour sanitizers ---"
"$BUILD/unit_tests_san"

# ---------------------------------------------------------------- step 3
step "Step 3: end to end run against tests/e2e_expected.txt"
"$BUILD/ipv4" < tests/e2e_input.txt > "$BUILD/e2e_actual.txt"
diff -u tests/e2e_expected.txt "$BUILD/e2e_actual.txt"
echo "end to end output matches exactly"

# ---------------------------------------------------------------- step 4
step "Step 4: end of input and Windows line ending checks"

printf '1.2.3.4\n' | "$BUILD/ipv4" > "$BUILD/eof_actual.txt"
printf "%s\n%s\n" \
    "Enter a string (or 'END' to quit): Extracted IPv4 address: 1.2.3.4 (decimal value: 16909060, port: none)" \
    "Enter a string (or 'END' to quit): Program terminated." \
    > "$BUILD/eof_expected.txt"
diff -u "$BUILD/eof_expected.txt" "$BUILD/eof_actual.txt"
echo "end of input without END: matches"

printf '192.168.1.1\r\nEND\r\n' | "$BUILD/ipv4" > "$BUILD/crlf_actual.txt"
printf "%s\n%s\n" \
    "Enter a string (or 'END' to quit): Extracted IPv4 address: 192.168.1.1 (decimal value: 3232235777, port: none)" \
    "Enter a string (or 'END' to quit): Program terminated." \
    > "$BUILD/crlf_expected.txt"
diff -u "$BUILD/crlf_expected.txt" "$BUILD/crlf_actual.txt"
echo "Windows line endings: matches"

# ---------------------------------------------------------------- step 5
step "Step 5: forbidden facility scan of src/main.cpp"

FORBIDDEN='atoi|atol|atoll|strtol|strtoll|strtoul|strtoull|strtof|strtod'
FORBIDDEN="$FORBIDDEN"'|stoi|stol|stoll|stoul|stoull|stof|stod'
FORBIDDEN="$FORBIDDEN"'|sscanf|scanf|from_chars'
FORBIDDEN="$FORBIDDEN"'|inet_aton|inet_pton|inet_addr|inet_network|getaddrinfo'
FORBIDDEN="$FORBIDDEN"'|arpa/inet.h|netinet/in.h|winsock2.h'
FORBIDDEN="$FORBIDDEN"'|regex|stringstream|istringstream'

if grep -nwE "$FORBIDDEN" src/main.cpp; then
    echo "FAILED: a forbidden facility appears above in src/main.cpp"
    exit 1
fi

# Numeric extraction with >> from a stream. Bit shifts on a value are fine,
# so this looks only for an extraction out of an input stream.
if grep -nE '(cin|std::cin|ifstream|istream)[[:space:]]*>>' src/main.cpp; then
    echo "FAILED: extraction from a stream appears above in src/main.cpp"
    exit 1
fi

echo "no forbidden facility found in src/main.cpp"

# ---------------------------------------------------------------- step 6
echo
echo "ALL TESTS PASSED"

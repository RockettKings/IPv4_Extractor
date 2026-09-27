# AI Disclosure — IPv4 Extractor

Sep 27, 2026 · @Jaycob

Student: Jaycob Campos, University of Kansas. This disclosure covers every use of generative AI in this assignment.

## 1. Tools and dates

| Tool | Version | Date(s) | Used for |
| --- | --- | --- | --- |
| Claude (Anthropic), claude.ai app | Claude Opus 5.5 | September 27, 2026 | Drafting the code-generation prompt, the design document (`DESIGN.md`), the 76-case test table with verified expected values, the Section 6 prompts, and a first draft of this disclosure |
| Claude Code (Anthropic) | Claude Opus 5 (model ID `claude-opus-5`), Claude Code CLI 2.1.236 | September 27, 2026 | Generating `src/main.cpp`, `tests/unit_tests.cpp`, and `run_tests.sh` from the `DESIGN.md` prompts |

## 2. How AI was used

I used Claude to design the specification and tests before any code was generated, and I overrode two of its interpretation choices.

1. **Prompt drafting.** I gave Claude the assignment handout and asked for a prompt that would generate the code, plus a walkthrough checking it against the grading requirements. Claude wrote the prompt, listed bug traps (signed shift overflow, restarting a scan inside a rejected run, overflow on long digit runs, treating `0.0.0.0` as failure), and flagged two ambiguities with its own default answers.
2. **My review of those defaults.** I first rejected both defaults because I read them as conflicting with the handout (Section 5, rows 1–2). I told Claude to require exactly one valid address per line and to be strict about detached separators. After re-reading the handout, I changed S1: return the first valid address and treat the rest of the line as garbage.
3. **Design document.** Claude wrote `DESIGN.md`: the full specification, rules S1 and S2, pseudocode, eight required safeguards, a 76-case unit test table, end-to-end tests, and six prompts for a code-generation tool. It computed every expected value with an independent reference model before writing the table.
4. **Deliverables check.** I asked Claude what the rubric requires me to turn in.
5. **Disclosure document.** I asked Claude to create the required documents. It began writing `src/main.cpp`; I clarified that I wanted documents only, no code, and it stopped. That partial code was not used. Claude then drafted this disclosure.
6. **Code generation.** I had Claude Code carry out `DESIGN.md` Prompts 0–4 on September 27, 2026, in one conversation, with the whole of `DESIGN.md` as context. I gave them as instructions rather than pasting each prompt verbatim; Section 3.2 records the messages I actually sent. **Prompt 0** returned the S1/S2 summary and five points it would not resolve on its own: the specification file was not named `DESIGN.md`, this folder sat inside a larger git repository, the CLion starter files did not match the Section 1 layout, the Section 2.2 prompt string appeared to carry different spacing than the Section 5.2 expected output, and Section 5.3 step 2 named no warning flags. I answered those, then **Prompts 1–3** produced `src/main.cpp`, `tests/unit_tests.cpp`, `tests/e2e_input.txt`, `tests/e2e_expected.txt`, `run_tests.sh`, and `README.md`, one round each with no retries. `./run_tests.sh` passed on its first execution, so **Prompt 5 was never used and `src/main.cpp` has not been changed since it was generated**; see `FIX_LOG.md` for the step-by-step result and the empty fix table. **Prompt 4** then audited `src/main.cpp` against the specification, reported PASS on all seven items, and proposed five further inputs, which I had it add to `tests/unit_tests.cpp` as a separate labeled block (Section 7.1). Claude Code also renamed the two documents to `DESIGN.md` and `AI_DISCLOSURE.md`, pointed `CMakeLists.txt` at `src/main.cpp`, and deleted the unused CLion starter `main.cpp`. No AI tool ran any git command; nothing was committed or pushed. At my request, Claude Code also edited the `DESIGN.md` §7.2 student checklist (the walkthrough item's wording, then checking completed boxes) so it matches this disclosure; no specification or test-table text in `DESIGN.md` was changed.

## 3. Exact prompts

### 3.1 Prompts sent to Claude (verbatim, in order)

**Prompt A**, sent with the full assignment handout attached as a document:

```text
I need you to write a prompt that will generate the code for this assignment. After generating it, I want you to walk through it abstractly and make sure the prompt will produce no bugs and meet every grading requirement.
```

**Prompt B:**

```text
For judgment 1: the handout says, "Exactly one valid address may be extracted per input line; everything else in the line is either garbage (skipped) or part of a candidate token that fails validation." So only find one. For judgment 2: reject anything that does not exactly match the grammar. Be very strict. Also, generate the test cases. Then create a design document with prompts that an AI tool can read to generate the code.
```

**Prompt C:**

```text
OK, cool. Can you read the rubric and tell me what I need to turn in from this chat?
```

**Prompt D:**

```text
I need you to create the documents that contain everything needed for the requirements. No code, just documents.
```

**Prompt E**, sent as a comment on `DESIGN.md`:

```text
Can you read the requirements and return the section that talks about it?
```

**Prompt F**, sent as a comment on `DESIGN.md`:

```text
If you find one, return it. Everything else is garbage.
```

### 3.2 Messages sent to Claude Code

`DESIGN.md` Prompts 0–5 were **not** pasted into Claude Code verbatim. The whole of `DESIGN.md` was supplied as context and Claude Code was told to carry out the Section 6 prompts in order as instructions; those prompt texts are in `DESIGN.md` Section 6 and are not repeated here. Prompt 5 was never used, because nothing failed. Below is every message I actually sent in that session, verbatim and in order.

The kickoff instruction (message 1) and the follow-up about the five audit cases (message 3) were drafted for me by Claude (claude.ai) before I sent them.

**Message 1 — kickoff:**

```text
/jaycobcampos/CLionProjects/IPv4 Extractor is the project file. I need you
  to read the IPV4 extractor desgin document and implement it. Read DESIGN.md in this folder in full. It is the authoritative specification for this project. Carry out the prompts in its Section 6, in order, as my instructions.

1. Do Prompt 0 now. Reply with the S1/S2 summary and anything you find unclear or contradictory, then STOP and wait until I say "continue".

2. After I say "continue":
   - Prompt 1: create src/main.cpp.
   - Prompt 2: create tests/unit_tests.cpp.
   - Prompt 3: create tests/e2e_input.txt, tests/e2e_expected.txt, run_tests.sh, and README.md.
   If this folder is not a git repo, run git init first. Commit after each prompt with the message "Prompt N: <what it created>".

3. Run ./run_tests.sh. For every failure, apply Prompt 5 yourself: trace the input, find the exact line and root cause, make the smallest fix, and rerun. Record each failure in FIX_LOG.md: test ID, symptom, root cause, fix, and whether you or the test suite found it. Commit each fix separately. If everything passes on the first run, write that in FIX_LOG.md.

4. When all tests pass, do Prompt 4 (the audit). Report the results; do not rewrite the code.

Rules:
- Never edit DESIGN.md or change any expected value in its test tables. If you believe the spec or a test value is wrong, stop and ask me.
- Never skip or weaken a test step (including the sanitizer build) to make tests pass.
- Use nothing listed in DESIGN.md Section 2.4.
```

**Message 2 — reply to the five questions Claude Code raised in Prompt 0:**

```text
You can rename them so it fits the layout requirements. But done worry about the gitub stuff, just write the code based on the design.md
```

**Message 3 — follow-up, the five audit cases and the first disclosure items:**

```text
Yes, fill in the [TO FILL] items you can answer from this session: §1 model name, and §2 item 6 (what you generated, how many rounds it took, and a pointer to FIX_LOG.md). Facts only. Write UNKNOWN for anything you can't confirm.

Don't list your five audit inputs as my own test cases. They came from you, and the disclosure has to say that. Instead:
1. Add them to tests/unit_tests.cpp as a separate block labeled "Extra cases proposed by Claude Code (audit)". Derive each expected value from DESIGN.md, not from what the code currently outputs, and show me the reasoning for each.
2. Run ./run_tests.sh and confirm everything passes.
3. In AI_DISCLOSURE.md §7.1, add one line saying these five cases were proposed by Claude Code during the audit, and give the result.

Leave every "my own test cases" item blank; I'll write those myself. Don't touch any other [TO FILL] item or the verification statement. Do not commit or push anything. When you're done, list every file you changed and show me the changes.
```

**Message 4 — follow-up, question about the student review checklist:**

```text
For the student review checklist do I have to document any of these? Or is it just a trust me I did this
```

**Message 5 — follow-up, this disclosure update:**

```text
Update AI_DISCLOSURE.md with facts from this session only. Write UNKNOWN for anything you can't confirm.

1. §3.2: Remove the claim that DESIGN.md Prompts 0–5 were sent verbatim. Replace it with every message I actually sent you in this session, verbatim and in order: the kickoff instruction, my "continue" reply, and each follow-up. Add one line saying the kickoff instruction and the follow-up about the five audit cases were drafted for me by Claude (claude.ai) before I sent them.
2. §5 row 4: State that the generated code passed every test on the first run and needed no fixes. List the extra checks run before trusting it: the sanitizer build, the Prompt 4 audit, and the five audit cases you proposed.
3. §1 model name and §2 item 6: fill these in if they are still blank.
4. §7.2: Keep the existing limitations. Add anything the audit found that was not fixed, or "No other known issues."

Do not touch the "my own test cases" items or the §8 verification statement. Don't commit. List every change you made.
```

## 4. Attribution

The specification and tests were AI-drafted from my requirements decisions; the code is generated by the tool in Section 1 and reviewed by me.

| Part | Generated by | My contribution |
| --- | --- | --- |
| Interpretation of the handout (rules S1 and S2) | Claude proposed different defaults | I chose both rules; final S1 matches Claude's original default, S2 overrides it (Section 5) |
| `DESIGN.md` specification, pseudocode, safeguards | Claude | Directed scope; reviewed |
| 76-case unit test table and expected values | Claude, checked against its reference model | Walked through S03, O01, M05, and D01 with Claude guiding and confirmed each matches the table; ran the full suite myself (§7.1) |
| End-to-end input and expected output | Claude | Reviewed |
| Code-generation prompts (Prompts 0–5) | Claude | Sent them; wrote any follow-ups (Section 3.2) |
| `src/main.cpp` | Claude Code | None. I made no hand edits to the generated code; I reviewed it (§7.1) |
| `tests/unit_tests.cpp`, `run_tests.sh`, e2e files, README | Claude Code | No hand edits. I supplied test cases U01-U03 (inputs and predictions mine); Claude Code inserted them into `tests/unit_tests.cpp` |
| My own test cases | Me | U01–U03 in `tests/unit_tests.cpp`; framing adapted from a Claude worked example, addresses and predictions mine |
| This disclosure | Claude drafted | I supplied every remaining fact through progress logs applied by Claude Code, and reviewed every claim before signing |

## 5. Problems found in AI output and fixes

I reviewed two interpretation choices in Claude's first answer before any code existed; code-generation problems are logged from row 4 on.

| # | What the AI produced | Why it was wrong | Found by | Fix |
| --- | --- | --- | --- | --- |
| 1 | Claude's first prompt returned the **first** valid address when a line has several | Not wrong in the end. I first read "exactly one valid address" as rejecting lines with two addresses. After re-reading "everything else in the line is either garbage," I concluded the first valid address should win | Me, reviewing Claude's walkthrough | Briefly made S1 reject lines with 2+ addresses, then reverted: first valid address wins, rest is garbage. Tests M01–M06 updated |
| 2 | Claude's first prompt accepted `1.2.3.4 :80` as `1.2.3.4` with no port, treating the space as garbage | The handout requires rejecting anything that does not exactly match the grammar, including a colon not immediately after the fourth octet | Me, reviewing Claude's walkthrough | Rule S2: whitespace then `.` or `:` after an address → reject. Tests D01–D07 |
| 3 | `DESIGN.md` said to run "five prompts" but listed six (0–5) | Internal inconsistency | Claude, during its own review | Corrected to "Prompts 0–5" before delivery |
| 4 | No code problem found. The generated `src/main.cpp` passed every test on the first run of `./run_tests.sh` | — | — | None. `src/main.cpp` has not been changed since Claude Code generated it, and `DESIGN.md` Prompt 5 was never used |

**Because nothing failed, here are the extra checks that were run before trusting the generated code.**

- **Sanitizer build.** `run_tests.sh` step 2 builds and runs the whole unit suite a second time under AddressSanitizer and UndefinedBehaviorSanitizer with `-fno-sanitize-recover=all`, so any out-of-bounds index or undefined shift aborts the run. Both builds reported `81/81 passed` with no sanitizer output.
- **`DESIGN.md` Prompt 4 audit.** Claude Code audited `src/main.cpp` line by line against the specification and returned PASS on all seven items: the eight safeguards in Section 4.3, rules S1 and S2, every rejection condition in Section 3.2, every string in Section 2.2, every rule in Section 2.3, the absence of every facility in Section 2.4, and undefined behaviour or out-of-bounds indexing. The Section 2.2 strings were compared byte by byte with `od -c` rather than by eye, which is what confirms the single trailing space in the prompt.
- **Five extra cases proposed during that audit.** Claude Code proposed five inputs outside the Section 5.1 table as the ones most likely to break the parser, and they were added to `tests/unit_tests.cpp` as cases C01–C05. All five pass. They are recorded as Claude Code's work, not mine, in Section 7.1.
- **Forbidden-facility scan.** `run_tests.sh` step 5 greps `src/main.cpp` for every name in `DESIGN.md` Section 2.4, plus `stringstream`, `istringstream` and `regex`, plus extraction with `>>` from an input stream. No match.

`FIX_LOG.md` records the same result step by step, with an empty fix table.

## 6. Design decisions

Both strictness rules come directly from handout sentences and are my final interpretation.

**S1: the first valid address wins.** The handout says: "Exactly one valid address may be extracted per input line; everything else in the line is either garbage (skipped) or part of a candidate token that fails validation." Once a valid address is found, the rest of the line is garbage, so a line with several valid addresses returns the first one. `192a168.1.1.1` still succeeds, because `192` is a failed candidate.

**S2: no detached separator after an address.** The handout says to "reject anything that does not exactly match the grammar," including "a colon not immediately after the fourth octet" and "a stray period/colon directly adjacent to an otherwise-valid address." So an address followed by whitespace and then `.` or `:` is rejected (`1.2.3.4 :80`). The rule looks forward only, because a colon before an address is ordinary label punctuation (`IP: 10.0.0.1` succeeds). Non-whitespace garbage ends the check, so `1.2.3.4x:80` succeeds with port `none`, consistent with the handout's `server=10.0.0.255:8080end` sample.

## 7. Verification and known limitations

Every expected value in the test plan was checked by a reference model before any code was written; the code itself must pass the checks below before submission.

### 7.1 Verification performed

- **Expected values:** Claude implemented the `DESIGN.md` rules as an independent reference model and ran all 76 unit inputs and the end-to-end input through it; every result matched the intended behavior. Claude also cross-checked several decimal values by hand, e.g. 168.1.1.1 = 168 × 16777216 + 1 × 65536 + 1 × 256 + 1 = 2818638081.
- **Five extra cases proposed by Claude Code, not by me:** during the Prompt 4 audit, Claude Code proposed five inputs outside the `DESIGN.md` Section 5.1 table as the ones most likely to break the parser, and I had it add them to `tests/unit_tests.cpp` as cases C01–C05 under the heading "Extra cases proposed by Claude Code (audit)". They are `"1.2.3.4\v:80"` (fail), `"1.2.3.4:80   "` (1.2.3.4 · 16909060 · 80), `"::10.0.0.1"` (fail), `"0.0.0.0 :80"` (fail), and a 40-digit first octet (fail); each expected value is derived from `DESIGN.md` in a comment above the block. All five pass, in both the plain and the sanitized build, and adding them took the unit suite from 76 cases to 81. **These are not my own test cases** and do not count toward the separate requirement below.

* [x] `./run_tests.sh` passes: 84/84 unit cases (76 from `DESIGN.md`, 5 audit cases C01–C05 proposed by Claude Code, and my own U01–U03), the sanitizer build (AddressSanitizer + UndefinedBehaviorSanitizer), the end-to-end diff, the EOF and Windows-line-ending runs, and the forbidden-function scan. Run by me on September 27, 2026.
* [x] The handout's sample run reproduces exactly when typed interactively.
* [x] I walked through cases S03, O01, M05, and D01 in `src/main.cpp` step by step with Claude (claude.ai) guiding; each result matched the expected value.
* [x] I reviewed the eight safeguards in `DESIGN.md` §4.3 with Claude (claude.ai) guiding. I then ran greps myself on `src/main.cpp` for every banned function, class, and header name in `DESIGN.md` §2.4: no matches.
* [x] I ran my own test cases, U01–U03. I wrote each prediction down before running the program, and all three matched:

| ID | Input | My prediction | Actual |
| --- | --- | --- | --- |
| U01 | `Server (256.15.145.2) ok` | invalid, because the first octet 256 is over 255 | `Invalid input: no valid IPv4 address found` — matches |
| U02 | `Server (167.67.152.9): 255` | valid | `Extracted IPv4 address: 167.67.152.9 (decimal value: 2806224905, port: none)` — matches |
| U03 | `Server (245.99.245.7): 1` | valid | `Extracted IPv4 address: 245.99.245.7 (decimal value: 4116968711, port: none)` — matches |

  The port is `none` in U02 and U03 because the `)` separates the address from the colon; a port counts only when `:` directly follows the fourth octet. The command I ran was:

  ```text
  printf 'Server (256.15.145.2) ok\nServer (167.67.152.9): 255\nServer (245.99.245.7): 1\nEND\n' | ./build/ipv4
  ```

### 7.2 Known limitations

- **Interpretation risk.** Rules S1 and S2 are my reading of the handout. If the instructor intended lines with two or more valid addresses to be rejected, or treated whitespace as fully separating tokens, tests M01–M04, M06, and D01–D05, D07 would expect different results.
- **ASCII whitespace only in S2.** Only space, tab, `\v`, `\f`, and `\r` count as whitespace. A Unicode space such as a non-breaking space is garbage, so `1.2.3.4`, non-breaking space, `:80` is accepted with port `none`.
- **Bytes, not characters.** Input is processed byte by byte; any non-ASCII character is garbage.
- **End of input.** On Ctrl-D (EOF) without `END`, `Program terminated.` prints on the same line as the last prompt.
- **Audit result.** The `DESIGN.md` Prompt 4 audit returned PASS on all seven items and found no defect, so nothing was left unfixed. It raised one observation that is not a defect: on failure, `readNumber` leaves its index and its partial value where they stopped. That is what the Section 4.2 pseudocode does, and it is harmless, because `validateCandidate` returns immediately and `extractIPv4` copies to the outputs only on success. **No other known issues.**

## 8. Verification statement

> I confirm that I understand every line of the submitted code and did not copy any of it blindly. The code has been tested with the suite in `tests/` and the checks in Section 7.1, and it works as intended. All known bugs, limitations, and unexpected behavior are listed in Section 7.2.
>
> Signed: Jaycob Campos, September 27, 2026

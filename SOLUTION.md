# NFA solution

`src.hpp` implements the assignment's `Grammar` API using only the four
permitted headers. Every graph stores one adjacency list per state.

Literal, optional, positive repetition, and star factories build constant-size
graphs. Concatenation copies disjoint graphs and links every left accepting
state to the right start by epsilon edges. Union adds a new start with epsilon
edges into both operands. Neither operation modifies its operands.

The regex parser creates separate chains for alternatives, with a shared start.
Quantifiers modify the character edge with a skip edge or a repetition loop.
This preserves quantifier, concatenation, and alternation precedence and builds
the graph in O(m) time and space for a pattern of length m.

Each advance takes the epsilon closure, follows matching character edges, then
takes the epsilon closure again. Breadth-first search visits each state once
per closure, including in cyclic graphs. Matching a string of length n takes
O((n + 1)m) expected time and O(m) auxiliary space with hash-based state sets.
An accepting state after all characters have been consumed proves a full match.

## Validation

Run `python3 tests/verify.py` to compile all seven supplied drivers and compare
their outputs with the supplied answers. It also checks 72,072 deterministic
pattern/string pairs against Python's independent `re.fullmatch` implementation.
All checks passed before submission.

`cmake -S . -B build && cmake --build build` builds the standalone `code` runner.
Its input consists of pattern/candidate pairs and its output is 1 or 0 per pair.
The actual problem API accepts C++ source and compiles its own drivers against
the submitted `src.hpp`; it does not list Git as an accepted language.

The provided submission client was repaired by adding its missing `submit_code`
method, preserving submission-ID logging.

## Judge result

- Submission: 879493, C++ header submission for problem 2684.
- Result: Accepted, 100/100, all seven groups passed.
- Reported total runtime: 11 ms; peak memory: 4,009,984 bytes.
- Attempts used: 1 of 5.
- Submitted implementation commit: 81874b962156b83ed1d57311d5ac111b82da66f7.

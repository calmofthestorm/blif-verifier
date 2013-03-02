blif-verifier
=============

Simple utility that generates C equivalence checkers for two circuits. [Current status: WRITTEN, UNTESTED]

This is a simple program to verify that two combinational circuits expressed in
a restricted subset of the BLIF format are equivalent by simulating all
combinations of inputs (brute force). Circuit-SAT is NP-complete, but for many
real world circuits more efficient approaches exist than my brute force
implementation.

It works by generating a C implementation of the circuit, which can then be
compiled with a harness to perform the actual verification. Since the
verification is one tight loop and a bunch of logic operations, this has a
substantial speed improvement over simulating the circuit using indirect
data structures (similar to compiling vs interpreting); about two orders
of magnitude faster than my simulation verifier (written for a course at
Michigan and therefore not publishable).

A cleaner solution might be to cut out the compiler and just emit machine
code to create the circuit evaluations, then put the tight loop in the
program, thus removing the need for a user to perform a compile-run-compile
action, but this is difficult to implement in a platform-agnostic way.

ISSUES:
* Code still needs some cleanup (assertions -> exceptions; better messages)
* Infinite loop on non-combinational circuits (-> error)
* Only supports small subset of BLIF.
  (wontfix; it's sufficient for combinational circuits)
* BLIF requires that truth tables be unambiguous insofar as input vectors must
  uniquely specify an output. Currently, if an input vector is ambiguous it will
  be treated as 1; I would like this to instead be an error. (-> error)
* Needs thorough unit and integration test suite.

LEGAL:
This is a hobby project. It is published in the hope of being useful but I make
no guarantees whatsoever about its functionality. There are better algorithms and
better tools for production use, and you would be wise not to trust a single tool
no matter its quality for anything really important. You bear all responsibility
for consequences arising from your use of this tool.

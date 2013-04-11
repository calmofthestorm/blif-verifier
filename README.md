blif-verifier
=============

Simple utility that generates C equivalence checkers for two circuits. [Current status: COMPLETE]

To use:

    make
    ./blif-verifier circuit1.blif circuit2.blif verifier.c
    cc verifier.c harness.c -lm -o verifier
    ./verifier

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

Note: Compiler optimizations are designed with the idea that the code will
be compiled once and run many times. Presumably, the verifier is a one-shot
program. As such, I have found that it is typically faster to compile the
verifier WITHOUT compiler optimizations -- the verifier will run much, much
slower, but the compile will finish much faster as well, and you win on
balance.

    alexr@autumn:~/projects/blif-verifier$ time gcc harness.c a.c -lm  -O3
    real  0m57.275s
    user  0m56.808s
    sys   0m0.260s
    
    alexr@autumn:~/projects/blif-verifier$ time ./a.out
    Circuits are equivalent. 65536 combinations explored.
    real  0m0.030s
    user  0m0.028s
    sys   0m0.000s
    
    alexr@autumn:~/projects/blif-verifier$ time gcc harness.c a.c -lm 
    real  0m17.571s
    user  0m17.005s
    sys   0m0.372s
    
    alexr@autumn:~/projects/blif-verifier$ time ./a.out
    Circuits are equivalent. 65536 combinations explored.
    
    real  0m0.976s
    user  0m0.944s
    sys   0m0.020s
    
A cleaner solution might be to cut out the compiler and just emit machine
code to create the circuit evaluations, then put the tight loop in the
program, thus removing the need for a user to perform a compile-run-compile
action, but this is difficult to implement in a platform-agnostic way.

Note that the primary purpose of writing this was to play with all the cool
new toys in C++11, and so I'm taking the excuse to use them wherever possible.

ISSUES:
* Only supports small subset of BLIF.
  (wontfix; it's sufficient for combinational circuits)
* BLIF requires that truth tables be unambiguous insofar as input vectors must
  uniquely specify an output. Currently, if an input vector is ambiguous it will
  be treated as 1; I would like this to instead be an error. (-> error)
* Test/configure build on Windows/Mac
* Migrate test suite to an actual framework

LEGAL:
This is a hobby project. It is published in the hope of being useful but I make
no guarantees whatsoever about its functionality. There are better algorithms and
better tools for production use, and you would be wise not to trust a single tool
no matter its quality for anything really important. You bear all responsibility
for consequences arising from your use of this tool.

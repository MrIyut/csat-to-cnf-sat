CSAT-to-CNF-SAT reduction, outputs the formula in DIMACS format:

- apply the reduction of CSAT to SAT.
- apply the SAT reduction to CNF SAT:
  - solving De Morgan's laws
  - if possible, eliminating redundant ports (x1 && (x2 && x3) = x1 && x2 && x3).
  - the distribution of brackets according to the method presented in the course x1 || x2 || (x3 && x4) = (x1 || x5 || x6) && (x2 || !x5 || x6) && (x3 || !x6) && (x4 || !x6)
  - converting the circuit in DIMACS format

To ease the implementation, all the initial CSAT nodes created are kept in an array. Later, a general tree was used for the SAT tree.

The functions for solving De Morgan's laws are collectively grouped into a function for simplifying the current tree. Considering that the simplification function is recursive, in order to increase the efficiency of the program, it is applied as soon as a new gate has been introduced into the circuit, and it is applied only to the respective subtree.

At the end, there is an asterix, namely, if the main gate of the initial tree is OR, then after applying the reduction, the tree will end up in DNF form, to solve this problem lightly, an AND gate is created that has only the OR gate as input and the distribution is applied once more on this AND gate.

# USAGE

`./main input-file output-file`

There is also a cadical checker to validate the reduction is correct.

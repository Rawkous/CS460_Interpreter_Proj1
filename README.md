#Interpreter Phase 4
Names: Mel Aveo, Brian Axel Martinez

## Contributions:
### 11/11/26

Axel:
- Reformated functions in Range, Statements, and Expr to now contain only raw pointers, which we now are responsible for deleting or transferring ownership elsewhere.
- Started filling in the ListObject.hpp/cpp class files.


### 11/12/26

Axel:
- Finished declaring the ListObject classes and functions in the cpp.
- Added ListObject to Makefile
- Added the new ListLiteral class to Expr.hpp. 

### 11/13/26

Mel:

- Updated Token.hpp and Token.cpp to support the new Phase 4 reserved keywords and square bracket tokens.
- Updated Tokenizer.cpp to recognize if, elif, else, while, input, int, float, len, split, append, pop, and list brackets.

Axel:

- Implemented the ListValueDescriptor functions needed in ListObject.cpp
- finished implementing ListObject functions and added the necessary code in parser.
- Implemented the ListLiteral class functions in Expr.cpp
- merged Mel's code to my home repo.


### 11/14/26

Mel:

- Updated Parser.hpp and Parser.cpp to recognize the new Phase 4 grammar.
- Added parsing for if/elif/else conditional statements and while loops.
- Updated Statements.hpp and Statements.cpp to support conditional execution and while loops.
- Implemented evaluation of Boolean conditions, including re-evaluating while conditions after each iteration.

Axel:

- Added Element ownership operation classes and functions to Expr.hpp/cpp
- Added the AppendStatement and IndexAssignmentStatement class definitions to hpp.
- finished implementing default constructor/destructors for the newly added node classes in Expr.
- working on as many print and evaluate functions.


### 11/15/26

Mel:
- Implemented the print() and evaluate() functions for InputExpr, PopExpr, and SubscriptExpr in Expr.cpp.
- Updated Expr.cpp to improve memory cleanup for list literals, list equality comparisons, and split operations.
- Added finite-value validation for float conversions.
- Created Phase 4 test files
- Compiled and tested the Phase 4 interpreter on Blue using the Makefile.
- Ran Valgrind tests to check for memory leaks and invalid memory accesses.
- Ran Phase 1–3 regression tests
- Verified that the interpreter builds successfully on Blue using make clean and make without compiler warnings or errors.

## Joint Contributions
- Integrated the Phase 4 tokenizer, parser, expression nodes, statement classes, and list runtime.
- Reviewed and corrected memory ownership and error handling for raw pointers.
- Tested the completed interpreter to verify that Phase 4 features work together without breaking earlier functionality.
- Verified list aliasing, reference counting, and runtime error handling.
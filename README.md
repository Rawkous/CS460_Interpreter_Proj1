#Interpreter Phase 4
Names: Mel Aveo, Brian Axel Martinez

Contributions
10/11/26

Axel:
-Reformated functions in Range, Statements, and Expr to now contain only raw pointers, which we now are responsible for deleting or transferring ownership elsewhere.
-Started filling in the ListObject.hpp/cpp class files.


10/12/26

Axel:
-Finished declaring the ListObject classes and functions in the cpp.
-Added ListObject to Makefile
-Added a the new ListLiteral class to Expr.hpp. 

10/13/26

Axel:
-Implemented the ListValueDescriptor functions needed in ListObject.cpp
-finished implementing ListObject functions and added the necessary code in parser.
-Implemented the ListLiteral class functions in Expr.cpp
-merged Mel's code to my home repo.


10/14/26

Axel:

-Added Element ownership operation classes and functions to Expr.hpp/cpp
-Added the AppendStatement and IndexAssignmentStatement class definitions to hpp.
-finished implementing default constructor/destructors for the newly added node classes in Expr.
-working on as many print and evaluate functions.



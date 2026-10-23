#Interpreter Phase 2
Names: Mel Aveo, Brian Axel Martinez

Contributions
10/3/26 11am - 1pm:
Mel:
Updated the Token.hpp to recognize new keywords "in" and "range". Also recognizes new token types of indent/dedent, colon and comma. Updated Token.cpp with the appropriate new tokens.

Axel:
Created Range.hpp with the EvaluatedRange and RangeExpression class declarations from the specification. Started Range.cpp with the three RangeExpression constructors (range(stop), range(start, stop) and range(start, stop, step)), the destructor that releases the owned expression trees, evaluate() and print(). Added Range.o to the Makefile.

10/5/26 11am - 1pm:
Mel:
Updated the tokenizer to include a queue for the indentation levels as well as handling the new tokens added to Token.hpp. Fixed syntax issue in enum class Keyword in token.hpp. In range.cpp I fixed the returns relating to start, stop and step and the calls to print in RangeExpression::print

Axel:
Implemented the EvaluatedRange member functions in Range.cpp: the constructor that reports a runtime error when the step is zero (even for an otherwise empty range), the start/stop/step getters, hasIteration() and shouldContinue(). Corrected the order of evaluation in RangeExpression::evaluate() so the arguments are evaluated once, in source order (start, stop, step), with the defaults start = 0 and step = 1.

10/7/26 11am - 1pm:
Mel:
Updated statements.hpp and statements.cpp ForStatement class to be python-style which involved a new constructor, destructor, evaluate and print functions.

Axel:
Implemented the range grammar in Parser.cpp: range() and rangeArguments(). rangeArguments() reads one to three <rel-expr> arguments separated by commas and calls the matching RangeExpression constructor, treating a single argument as the stop value. Added the matching declarations to Parser.hpp.

10/8/26 8 pm - 10 pm
Axel:
Rewrote forStatement() in Parser.cpp for the Python-style grammar ("for" <id> "in" <range> ":" <suite>) and added suite() (NEWLINE INDENT <statements> DEDENT). Restructured statements() and statement() so that the NEWLINE is consumed only after simple statements (assignment and print) and not after a for statement, and so that the loop in statements() continues only on an identifier, "for" or "print" rather than on any keyword. Debugged the "expected 'in' keyword" parse failure and worked with the tokenizer changes so that "in", "range", ":" and "," are recognized.



10/9/26 10am - 3pm
Axel: 
Finished adding and testing all relevant test files for the range section. Updated the forStatement print function to account for indentation when printing out results after parsing.

Mel:
Cleaned up memory allocation in Parser.cpp and added relevant phase 2 test files. Tested on blue with no issues.

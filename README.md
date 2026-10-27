# Interpreter Phase 3

**Names:** Mel Aveo, Brian Axel Martinez

## Contributions

**10/23/26: 3pm - 5pm, 8pm - 10pm**

**Mel:**
- Added support in Token.hpp for 64-bit integers, doubles, Boolean and string literals, the reserved keywords True, False, and str, and the // operator.
- Updated Token.cpp to print the new tokens correctly.

**9am - 10am**

**Axel:**
- Created ValueDescriptor.hpp and ValueDescriptor.cpp for the new value descriptor classes.
- Set up the include guard for the base class and all four derived classes (Integer, Double, Boolean, String).

**10/24/26: 11am - 2pm** 

**Axel:**
- Finished implementing all classes in ValueDescriptor.cpp.
- Implemented toString() using default C++ stream formatting for doubles and True/False for Booleans.
- Updated the Makefile to support automatic header dependency tracking using -MMD -MP.
- Integrated Mel's updated Token files and confirmed the project compiled without warnings.

  **9pm - 11pm**
- Updated SymbolTable.hpp and SymbolTable.cpp so the symbol table owns its ValueDescriptor objects: the map now stores ValueDescriptor pointers, and a new destructor deletes every stored descriptor.
- Deleted the four copy and move operations so two symbol tables can never own the same descriptors.
- setValueFor() now takes ownership of the descriptor it is given and deletes the old descriptor when a variable is reassigned.
- getValueFor() now returns a const ValueDescriptor * (read-only; the table keeps ownership) and still reports undefined variables. print() uses toString().

**10/24/26: 1pm - 4pm**

**Mel:**
- Updated getToken() to recognize integer and double literals, the new reserved keywords, and the // operator.
- Implemented 64-bit integer overflow detection.
- Added validation for malformed numeric literals, unsupported exponent notation, invalid string characters, and unterminated strings.

**10/25/26: 1pm - 3pm, 4pm - 7:30pm**

**Mel:**
- Updated Parser.cpp to recognize double, boolean and string literals.
- Updated arithAtom() for the new expression nodes for the data types and str() conversion.
- arithTerm() now recognizes floor division operator //.
- Added error handling for phase 3 grammar
- Range.hpp and Range.cpp now use int_64t for values
- RangeExpression::evaluate() evaluates range arguments using ValueDescriptor objects
- Statements.cpp now works with ValueDescriptor
- PrintStatement::evaluate uses toString() to print integers, doubles, bool. and string
- ForStatement::evaluate uses 64-bit integers and loop variables are now IntegerValueDescriptor objects

**10/25/26: 12pm - 3pm**

**Axel:**
- Updated Expr.hpp and Expr.cpp so every evaluate() returns a newly allocated ValueDescriptor * that the caller owns.
- Variable::evaluate() returns a clone of the descriptor in the symbol table. Unary and binary expressions hold their operand descriptors in std::unique_ptr so they are deleted after the result is produced, including when an error is thrown.
- Added the DoubleLiteral, BooleanLiteral, StringLiteral, and StrConversion (str) expression nodes.
- Implemented the operator rules in helper functions (add, subtract, multiply, divide, floorDivide, modulo, equals, compare): integer-to-double promotion, string concatenation, string and numeric ordering, equality between unrelated types, and Python floor-division and remainder behavior for // and %.
- Division, // and % check for a zero divisor (including 0.0 and -0.0) before dividing. Operand type errors name the operator, the operand types, and the line and column.

**10/26/26: 3pm - 6pm**

**Axel:**
- Wrote the Phase 3 test programs in tests/ with descriptive names: literals, assignment and reassignment across types, every operator, str() and print formatting, loops, runtime errors, and invalid literals. Added expected outputs and a runner script (run_phase3_tests.sh).
- Fixed the tokenizer so blank lines with Windows (\r\n) line endings and whitespace-only lines containing tabs are ignored instead of causing indentation errors.

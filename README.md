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

**10/24/26: 1pm - 4pm**

**Mel:**
- Updated getToken() to recognize integer and double literals, the new reserved keywords, and the // operator.
- Implemented 64-bit integer overflow detection.
- Added validation for malformed numeric literals, unsupported exponent notation, invalid string characters, and unterminated strings.

**10/25/26: 1pm - 3pm, 4pm - 7:30pm

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

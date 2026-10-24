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
- Updated Tokenizer.hpp and Tokenizer.cpp to support 64-bit integers, doubles, and strings.
- Implemented readDouble() and readString() in Tokenizer.cpp.
- Updated getToken() to recognize integer and double literals, the new reserved keywords, and the // operator.
- Implemented 64-bit integer overflow detection.
- Added validation for malformed numeric literals, unsupported exponent notation, invalid string characters, and unterminated strings.
- Tested the updated tokenizer on Blue using valid and invalid Phase 3 inputs and verified that Phase 2 indentation handling still worked.
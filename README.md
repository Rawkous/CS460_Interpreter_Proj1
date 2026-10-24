#Interpreter Phase 3
Names: Mel Aveo, Brian Axel Martinez

Contributions
10/23/26: 3pm - 5pm, 8pm - 10pm
Mel: 
- Added new tokens in Token.hpp for 64-bit integers, doubles, booleans, strings, new keywords
'True', 'False' and 'str', and // operator.
- Updated Token.cpp to recognize these new tokens and print their respective string.

9am - 10 am
Axel: 
- Created new cpp/hpp files for our new ValueDescriptor classes and copied over given descriptors.
- Set up the include guard so the base class and all four derived classes (Integer, Double, Boolean, String) sit inside a single header guard.

10/24/26 11am - 2 pm 
Axel: 
- Finished implementation of all classes within the ValueDescriptor.cpp file. 
- Implemented toString() so doubles use default C++ stream formatting (via ostringstream) instead of std::to_string, and Booleans print as True/False, so print and str() will share the same output format.
- Switched the Makefile to automatic header dependency tracking (-MMD -MP) so changes to headers like ValueDescriptor.hpp trigger the right rebuilds.
- Integrated the updated Token files from Mel, and confirmed the whole project builds cleanly with no warnings.

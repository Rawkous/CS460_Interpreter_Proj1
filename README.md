#Interpreter Phase 3
Names: Mel Aveo, Brian Axel Martinez

Contributions
10/23/26: 3pm - 5pm, 8pm - 10pm
Mel: Added new tokens in Token.hpp for 64-bit integers, doubles, booleans, strings, new keywords
'True', 'False' and 'str', and // operator. Updated Token.cpp to recognize these new tokens and 
print their respective string.

10/24/26: 1pm - 4pm
Mel: 
- Updated tokenizer to read 64int integers, doubles and strings. 
- Implemented readDouble and readString in Tokenizer.cpp. 
- Updated getToken to scan ints/doubles. 
- Implemented 64-bit integer overflow detection to reject values
outside the supported range. 
- Added validation to reject malformed numeric literals, unsupported exponent notation, invalid string characters,
and unterminated strings.
#Interpreter Phase 1
---------------------
Names: Mel Aveo, Brian Axel Martinez

## Contributions:
### 9/18/26 11am - 12pm:
#### Mel
I got started by extending the token and tokenizer to accept relational operators and equality operators
#### Axel
- Extended Statements header file to include declarations for the PrintStatement and ForStatement classes which are derived from the Statement super class.
- Added the appropriate virtual functions to the derived classes which override the Statement pure virtual functions. 
- Started the Implementation the derived classes
### 9/19/26: 11am - 1pm: 
#### Mel
I finished extending the token and tokenizer and worked out the token to be recognized with two symbols with help
from Axel. I began working on the parser file to handle relational expressions.
#### Axel
- Revised the Token class and defined the second char named nextSymbol.
- Finished implementing the constructor, destructor, and functions for the PrintStatement and ForStatement classes. 


### 9/20/26: 11am - 1pm: 
#### Mel
I worked on the expression file by expanding evaluate to handle the new operators.
#### Axel
-Revised and updated the Parser files, by adding declarations and implementations of the derived classes we implemented.
- This allows the parser to not die and be able to recognize the print-statement and for-statement class functions when needed. 

### 9/21/26 11am - 1pm:
#### Mel
I added some code to help the print statements come out cleaner. I created new input files within the tests folder 
and tested on blue with no issues.
#### Axel
- Did further testing on blue with no issues.
- Extended the README to include my own documentation 


AI Use: None
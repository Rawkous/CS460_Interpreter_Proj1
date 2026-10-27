#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <stdexcept>


#include "Tokenizer.hpp"


bool Tokenizer::isDigit(char character) {
   return std::isdigit(static_cast<unsigned char>(character)) != 0;
}


bool Tokenizer::isIdentifierStart(char character) {
   return std::isalpha(static_cast<unsigned char>(character)) != 0 || character == '_';
}


bool Tokenizer::isIdentifierPart(char character) {
   return std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_';
}


bool Tokenizer::isDiscardedWhitespace(char character) {
   return character != '\n' && std::isspace(static_cast<unsigned char>(character)) != 0;
}


bool Tokenizer::getCharacter(char &character) {
   if (!inputStream.get(character))
       return false;


   if (character == '\n') {
       ++lineNumber;
       columnNumber = 1;
   } else {
       ++columnNumber;
   }
   return true;
}


std::string Tokenizer::readIdentifier(char firstCharacter) {
   std::string identifier{firstCharacter};
   while (inputStream.peek() != std::char_traits<char>::eof()) {
       char character = static_cast<char>(inputStream.peek());
       if (!isIdentifierPart(character))
           break;
       getCharacter(character);
       identifier += character;
   }
   return identifier;
}

// Phase 3: Updated all references to int to now be int64
std::int64_t Tokenizer::readInteger(char firstDigit) {
   std::int64_t value = firstDigit - '0';
   while (inputStream.peek() != std::char_traits<char>::eof()) {
       char character = static_cast<char>(inputStream.peek());
       if (!isDigit(character))
           break;


       const int digit = character - '0';
       if (value > (std::numeric_limits<std::int64_t>::max() - digit) / 10) {
           std::cerr << "Invalid integer literal at line " << lineNumber
                     << ", column " << columnNumber << ".\n";
           std::exit(EXIT_FAILURE);
       }


       getCharacter(character);
       value = value * 10 + digit;
   }
   return value;
}

// Phase 3: readDouble recognizes double literals
double Tokenizer::readDouble(std::string integerPart) {
    std::string number = std::move(integerPart);

    // Consume the decimal point
    char decimalPoint;
    getCharacter(decimalPoint);
    number += decimalPoint;

    // digit must follow the decimal point
    if (inputStream.peek() == std::char_traits<char>::eof() ||
        !isDigit(static_cast<char>(inputStream.peek()))) {

        std::cerr << "Invalid double literal at line "
                  << lineNumber << ", column "
                  << columnNumber << ".\n";
        std::exit(EXIT_FAILURE);
    }

    // Read the fractional digits
    while (inputStream.peek() != std::char_traits<char>::eof() &&
           isDigit(static_cast<char>(inputStream.peek()))) {

        char digit;
        getCharacter(digit);
        number += digit;
    }

    // Reject invalid exponent notation and suffixes
    if (inputStream.peek() != std::char_traits<char>::eof()) {
        char next = static_cast<char>(inputStream.peek());

        if (next == 'e' || next == 'E' ||
            next == '.' || isIdentifierPart(next)) {

            std::cerr << "Invalid double literal at line "
                      << lineNumber << ", column "
                      << columnNumber << ".\n";
            std::exit(EXIT_FAILURE);
        }
    }

    try {
        return std::stod(number);
    } catch (const std::exception &) {
        std::cerr << "Invalid double literal at line "
                  << lineNumber << ", column "
                  << columnNumber << ".\n";
        std::exit(EXIT_FAILURE);
    }
}

// Phase 3: Read single-quoted and double-quoted strings
std::string Tokenizer::readString(char openingQuote) {
    std::string value;

    while (inputStream.peek() != std::char_traits<char>::eof()) {
        char character;
        getCharacter(character);

        // Matching quote ends the string
        if (character == openingQuote) {
            return value;
        }

        // Only printable ASCII characters are allowed
        const unsigned char code = static_cast<unsigned char>(character);

        if (code < 32 || code > 126 || character == '\\') {
            std::cerr << "Invalid string literal at line "
                      << lineNumber << ", column "
                      << columnNumber << ".\n";
            std::exit(EXIT_FAILURE);
        }

        value += character;
    }

    // EOF before the closing quote
    std::cerr << "Unterminated string literal at line "
              << lineNumber << ", column "
              << columnNumber << ".\n";
    std::exit(EXIT_FAILURE);
}


Tokenizer::Tokenizer(std::ifstream &stream) : inputStream{stream} {}


Token Tokenizer::getToken() {
   if (ungottenToken) {
       ungottenToken = false;
       return lastToken;
   }

    if (!pendingTokens.empty()) {
        Token token = pendingTokens.front();
        pendingTokens.pop_front();

        tokens.push_back(token);
        return lastToken = token;
    }

    if (atLineStart) {
        std::size_t indentation = 0;

        // Count leading spaces.
        while (inputStream.peek() == ' ') {
            char space;
            getCharacter(space);
            ++indentation;
        }
        
        // Phase 3: fix
        // Skip tabs, carriage returns and extra spaces so that whitespace-only lines
        // (including lines ending in \r\n) are recognized as blank below.
        bool sawTab = false;
        while (inputStream.peek() == '\t' || inputStream.peek() == '\r' || inputStream.peek() == ' ') {
            char skipped;
            getCharacter(skipped);
            if (skipped == '\t')
                sawTab = true;
        }

        // Tabs are illegal in leading indentation.
        if (sawTab && inputStream.peek() != '\n' && inputStream.peek() != std::char_traits<char>::eof()) {
            std::cerr << "Tab used in leading indentation at line "
                      << lineNumber << ".\n";
            std::exit(EXIT_FAILURE);
        }

        // Blank/whitespace-only lines do not affect indentation.
        if (inputStream.peek() == '\n') {
            char newline;
            getCharacter(newline);
            atLineStart = true;
            return getToken();
        }

        // Only compare indentation if this is a real source line.
        if (inputStream.peek() != std::char_traits<char>::eof()) {

            // Increased indentation -> INDENT.
            if (indentation > indentationLevels.back()) {
                indentationLevels.push_back(indentation);

                Token indentToken;
                indentToken.setLocation(lineNumber, columnNumber);
                indentToken.markAsIndent();

                pendingTokens.push_back(indentToken);
            }

                // Decreased indentation -> one or more DEDENTs.
            else if (indentation < indentationLevels.back()) {
                bool matchingLevel = false;

                for (std::size_t level : indentationLevels) {
                    if (level == indentation) {
                        matchingLevel = true;
                        break;
                    }
                }

                if (!matchingLevel) {
                    std::cerr << "Indentation at line "
                              << lineNumber
                              << " does not match an earlier indentation level.\n";
                    std::exit(EXIT_FAILURE);
                }

                while (indentationLevels.back() > indentation) {
                    indentationLevels.pop_back();

                    Token dedentToken;
                    dedentToken.setLocation(lineNumber, columnNumber);
                    dedentToken.markAsDedent();

                    pendingTokens.push_back(dedentToken);
                }
            }
        }

        atLineStart = false;

        // If indentation processing created a token return it before the actual source token.
        if (!pendingTokens.empty()) {
            Token token = pendingTokens.front();
            pendingTokens.pop_front();

            tokens.push_back(token);
            return lastToken = token;
        }
    }


   while (inputStream.peek() != std::char_traits<char>::eof()) {
       char character = static_cast<char>(inputStream.peek());


       if (isDiscardedWhitespace(character)) {
           getCharacter(character);
           continue;
       }


       if (character == '\n') {
           const auto newlineLine = lineNumber;
           const auto newlineColumn = columnNumber;
           getCharacter(character);

           atLineStart = true;

           if (lineContainsToken) {
               Token token;
               token.setLocation(newlineLine, newlineColumn);
               token.markAsNewline();
               lineContainsToken = false;
               tokens.push_back(token);
               return lastToken = token;
           }


           // Newlines on blank or whitespace-only lines are insignificant.
           continue;
       }


       break;
   }


   Token token;
   token.setLocation(lineNumber, columnNumber);


    if (inputStream.peek() == std::char_traits<char>::eof()) {

        if (lineContainsToken) {
            Token newlineToken;
            newlineToken.setLocation(lineNumber, columnNumber);
            newlineToken.markAsNewline();

            lineContainsToken = false;

            tokens.push_back(newlineToken);
            return lastToken = newlineToken;
        }

        // Close any remaining indentation levels before EOF.
        if (indentationLevels.size() > 1) {
            indentationLevels.pop_back();

            Token dedentToken;
            dedentToken.setLocation(lineNumber, columnNumber);
            dedentToken.markAsDedent();

            tokens.push_back(dedentToken);
            return lastToken = dedentToken;
        }

        if (inputStream.bad()) {
            std::cerr << "Error while reading the input stream in Tokenizer.\n";
            std::exit(EXIT_FAILURE);
        }

        token.markAsEof();
   } else {
       char character;
       getCharacter(character);


       // Phase 3: integer/double scanner
        if (isDigit(character)) {
            std::string digits{character};

            // Read all digits before a possible decimal point
            while (inputStream.peek() != std::char_traits<char>::eof() &&
                   isDigit(static_cast<char>(inputStream.peek()))) {

                char digit;
                getCharacter(digit);
                digits += digit;
            }

            if (inputStream.peek() == '.') {
                token.setDoubleValue(readDouble(digits));
            } else {
                if (inputStream.peek() != std::char_traits<char>::eof()) {
                    char next = static_cast<char>(inputStream.peek());

                    if (isIdentifierPart(next)) {
                        std::cerr << "Invalid integer literal at line "
                                  << token.lineNumber() << ", column "
                                  << token.columnNumber() << ".\n";
                        std::exit(EXIT_FAILURE);
                    }
                }
                // Convert an integer literal with overflow detection
                std::int64_t value = 0;

                for (char digitChar : digits) {
                    const int digit = digitChar - '0';

                    if (value >
                        (std::numeric_limits<std::int64_t>::max() - digit) / 10) {

                        std::cerr << "Invalid integer literal at line "
                                  << token.lineNumber() << ", column "
                                  << token.columnNumber() << ".\n";
                        std::exit(EXIT_FAILURE);
                    }
                    value = value * 10 + digit;
                }
                token.setIntegerValue(value);
            }

        } else if (character == '=' || character == '>' ||
                   character == '<' || character == '!') {


       token.setSymbol(character);


       if (inputStream.peek() == '=') {
           char nextCharacter;
           getCharacter(nextCharacter);
           token.setNextSymbol(nextCharacter);
       } else if (character == '!') {
           std::cerr << "Unknown character in input at line "
                     << token.lineNumber()
                     << ", column " << token.columnNumber()
                     << ": '!'.\n";
           std::exit(EXIT_FAILURE);
       }


       } else if (character == '+' || character == '-' ||
                  character == '*' || character == '/' ||
                  character == '%' || character == ';' ||
                  character == '(' || character == ')' ||
                  character == '{' || character == '}' ||
                  character == ':' || character == ',' ){

           token.setSymbol(character);
            // Phase 3: Recognize integer floor division //
            if (character == '/' && inputStream.peek() == '/') {
                char nextCharacter;
                getCharacter(nextCharacter);
                token.setNextSymbol(nextCharacter);
            }

        } else if (character == '"' || character == '\'') {
            // Phase 3: String literal
            token.setStringValue(readString(character));
        } else if (isIdentifierStart(character)) {
            std::string identifier = readIdentifier(character);
           if (identifier == "for")
               token.setKeyword(Keyword::forKeyword);
           else if (identifier == "print")
               token.setKeyword(Keyword::printKeyword);
           else if (identifier == "in")
               token.setKeyword(Keyword::inKeyword);
           else if (identifier == "range")
               token.setKeyword(Keyword::rangeKeyword);
           // Phase 3: New reserved words
           else if (identifier == "True")
               token.setKeyword(Keyword::trueKeyword);
           else if (identifier == "False")
               token.setKeyword(Keyword::falseKeyword);
           else if (identifier == "str")
               token.setKeyword(Keyword::strKeyword);
           else
               token.setIdentifier(std::move(identifier));
       } else {
           std::cerr << "Unknown character in input at line " << token.lineNumber()
                     << ", column " << token.columnNumber() << ": '"
                     << character << "'.\n";
           std::exit(EXIT_FAILURE);
       }


       lineContainsToken = true;
   }


   tokens.push_back(token);
   return lastToken = token;
}


void Tokenizer::ungetToken() {
   if (ungottenToken) {
       std::cerr << "Tokenizer supports only one ungotten token.\n";
       std::exit(EXIT_FAILURE);
   }
   ungottenToken = true;
}


void Tokenizer::printProcessedTokens(std::ostream &output) const {
   for (const auto &token : tokens) {
       token.print(output);
       output << '\n';
   }
}

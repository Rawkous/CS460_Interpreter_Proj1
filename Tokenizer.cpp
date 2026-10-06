#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>


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


int Tokenizer::readInteger(char firstDigit) {
   int value = firstDigit - '0';
   while (inputStream.peek() != std::char_traits<char>::eof()) {
       char character = static_cast<char>(inputStream.peek());
       if (!isDigit(character))
           break;


       const int digit = character - '0';
       if (value > (std::numeric_limits<int>::max() - digit) / 10) {
           std::cerr << "Integer literal is too large at line " << lineNumber
                     << ", column " << columnNumber << ".\n";
           std::exit(EXIT_FAILURE);
       }


       getCharacter(character);
       value = value * 10 + digit;
   }
   return value;
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

        // Tabs are illegal in leading indentation.
        if (inputStream.peek() == '\t') {
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


       if (isDigit(character)) {
           token.setIntegerValue(readInteger(character));
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




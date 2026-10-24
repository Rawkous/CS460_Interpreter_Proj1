#ifndef ALEXICALANALYZER_TOKENIZER_HPP
#define ALEXICALANALYZER_TOKENIZER_HPP

#include <cstddef>
#include <fstream>
#include <iosfwd>
#include <string>
#include <vector>
#include <deque>
#include "Token.hpp"
#include <cstdint>

class Tokenizer {
public:
    explicit Tokenizer(std::ifstream &inputStream);

    Token getToken();
    void ungetToken();
    void printProcessedTokens(std::ostream &output) const;

private:
    Token lastToken{};
    bool ungottenToken{false};
    std::ifstream &inputStream;
    std::vector<Token> tokens{};
    std::size_t lineNumber{1};
    std::size_t columnNumber{1};
    std::deque<Token> pendingTokens{};
    std::vector<std::size_t> indentationLevels{0};
    bool atLineStart{true};
    bool lineContainsToken{false};

    bool getCharacter(char &character);
    std::string readIdentifier(char firstCharacter);
    // Phase 3: readInteger now supports int64, added readDouble and readString
    std::int64_t readInteger(char firstDigit);
    double readDouble(std::string integerPart);
    std::string readString(char openingQuote);

    static bool isDigit(char character);
    static bool isIdentifierStart(char character);
    static bool isIdentifierPart(char character);
    static bool isDiscardedWhitespace(char character);
};

#endif // ALEXICALANALYZER_TOKENIZER_HPP

#include <iostream>

#include "Token.hpp"

void Token::print(std::ostream &output) const {
    if (isNewline())
        output << "NEWLINE";
    else if (isIndent())
        output << "INDENT";
    else if (isDedent())
        output << "DEDENT";
    else if (isEof())
        output << "EOF";
    else if (isForKeyword())
        output << "for";
    else if (isPrintKeyword())
        output << "print";
    else if (isInKeyword())
        output << "in";
    else if (isRangeKeyword())
        output << "range";
    // Phase 3: added checks for true, false, and str
    else if (isTrueKeyword())
        output << "True";
    else if (isFalseKeyword())
        output << "False";
    else if (isStrKeyword())
        output << "str";
    // Phase 4: conditional and while keywords
    else if (isIfKeyword())
        output << "if";
    else if (isElifKeyword())
        output << "elif";
    else if (isElseKeyword())
        output << "else";
    else if (isWhileKeyword())
        output << "while";
// Phase 4: input and conversion keywords
    else if (isInputKeyword())
        output << "input";
    else if (isIntKeyword())
        output << "int";
    else if (isFloatKeyword())
        output << "float";
// Phase 4: string and list operation keywords
    else if (isLenKeyword())
        output << "len";
    else if (isSplitKeyword())
        output << "split";
    else if (isAppendKeyword())
        output << "append";
    else if (isPopKeyword())
        output << "pop";
    else if (isOpenParen())
        output << '(';
    else if (isCloseParen())
        output << ')';
    // Phase 4: list brackets
    else if (isOpenBracket())
        output << '[';
    else if (isCloseBracket())
        output << ']';
    else if (isOpenBrace())
        output << '{';
    else if (isCloseBrace())
        output << '}';
    else if (isEqualityOperator() || isRelationalOperator()) {
        output << ' ' << symbol();
        if (nextSymbol() != '\0')
            output << nextSymbol();
        output << ' ';
    }
    else if (isAssignmentOperator())
        output << " = ";
    else if (isSemicolon())
        output << ';';
    else if (isColon())
        output << ':';
    else if (isComma())
        output << ',';
    else if (isMultiplicationOperator())
        output << " * ";
    else if (isAdditionOperator())
        output << " + ";
    else if (isSubtractionOperator())
        output << " - ";
    else if (isModuloOperator())
        output << " % ";
    else if (isDivisionOperator())
        output << " / ";
    // Phase 3: added floor division check
    else if (isFloorDivisionOperator())
        output << " // ";
    else if (isIdentifier())
        output << identifier();
    else if (isInteger())
        output << integerValue();
    // Phase 3: added double and string checks
    else if (isDouble())
        output << doubleValue();
    else if (isString())
        output << '"' << stringValue() << '"';
    else
        output << "uninitialized token";
}

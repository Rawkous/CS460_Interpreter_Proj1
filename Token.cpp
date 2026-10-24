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
    else if (isOpenParen())
        output << '(';
    else if (isCloseParen())
        output << ')';
        // added openbrace, closebrace, and equality and relational operator checks
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

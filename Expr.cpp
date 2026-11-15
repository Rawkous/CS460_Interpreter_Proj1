#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <cmath>

#include "Expr.hpp"
#include "ListObject.hpp"

using namespace std;


ExprNode::ExprNode(Token token) : expressionToken{std::move(token)} {}


const Token &ExprNode::token() const {
   return expressionToken;
}


BinaryExprNode::BinaryExprNode(Token token, ExprNode *left, ExprNode *right)
   : ExprNode{std::move(token)},
     leftOperand{left},
     rightOperand{right} {}


BinaryExprNode::~BinaryExprNode() {
   delete leftOperand;
   delete rightOperand;
}

// Phase 3: Using a few small accessor methods and helper functions in a namespace
//  to make evaluate code more readable and maintainable.
namespace {
    using Type = ValueDescriptor::Type;

    bool isNumeric(const ValueDescriptor &value) {
        return value.type() == Type::integer || value.type() == Type::doubleValue;
    }
    std::int64_t isInt(const ValueDescriptor &value) {
        return static_cast<const IntegerValueDescriptor &>(value).value();
    }
    double isDouble(const ValueDescriptor &value) {
        if (value.type() == Type::integer)
            return static_cast<double>(isInt(value));
        return static_cast<const DoubleValueDescriptor &>(value).value();
    }
    const string &isString(const ValueDescriptor &value) {
        return static_cast<const StringValueDescriptor &>(value).value();
    }
    bool isBool(const ValueDescriptor &value) {
        return static_cast<const BooleanValueDescriptor &>(value).value();
    }
    [[noreturn]] void typeError(const Token &op, const ValueDescriptor &left,
                                                        const ValueDescriptor &right) {
        string text(1, op.symbol());
        if (op.nextSymbol() != '\0') {
            text += op.nextSymbol();
        }
        throw std::runtime_error("type error at line " + to_string(op.lineNumber()) +
            ", column " + to_string(op.columnNumber()) +
        ": operator '" + text + "' cannot be applied to " +
        left.typeName() + " and " + right.typeName());
    }
    [[noreturn]] void zeroDivisor(const Token &op, const string &what) {
        throw std::runtime_error(what + " by zero at line " + to_string(op.lineNumber()) +
            ", column " + to_string(op.columnNumber()));
    }
    
    ValueDescriptor *add(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() == Type::string && right.type() == Type::string) {
            return new StringValueDescriptor(isString(left) + isString(right));
        }
        if (left.type() == Type::integer && right.type() == Type::integer) {
            return new IntegerValueDescriptor(isInt(left)+ isInt(right));
        }
        if (isNumeric(left) && isNumeric(right)) {
            return new DoubleValueDescriptor(isDouble(left)+ isDouble(right));
        }
        typeError(op, left, right);
    }

    ValueDescriptor *subtract(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() == Type::integer && right.type() == Type::integer) {
            return new IntegerValueDescriptor(isInt(left)- isInt(right));
        }
        if (isNumeric(left) && isNumeric(right)) {
            return new DoubleValueDescriptor(isDouble(left)- isDouble(right));
        }
        typeError(op, left, right);
    }

    ValueDescriptor *multiply(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() == Type::integer && right.type() == Type::integer) {
            return new IntegerValueDescriptor(isInt(left)* isInt(right));
        }
        if (isNumeric(left) && isNumeric(right)) {
            return new DoubleValueDescriptor(isDouble(left)* isDouble(right));
        }
        typeError(op, left, right);
    }

    ValueDescriptor *divide(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (!isNumeric(left) || !isNumeric(right)) {
            typeError(op, left, right);
        }
        if (isDouble(right) == 0.0) {
            zeroDivisor(op, "division");
        }
        return new DoubleValueDescriptor(isDouble(left) / isDouble(right));
    }
    // Floor division and remainder share one quotient calculation (b must be nonzero)
    std::int64_t floorQuotient(std::int64_t a, std::int64_t b) {
        std::int64_t q = a / b;
        // if inexact, signs differ: so round down                         
        if (a % b != 0 && ((a < 0) != (b < 0)))         
            --q;
        return q;
    }
    
    ValueDescriptor *floorDivide(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() != Type::integer || right.type() != Type::integer) {
            typeError(op, left, right);
        }
        if (isInt(right) == 0) {
            zeroDivisor(op, "integer division");
        }
        return new IntegerValueDescriptor(floorQuotient(isInt(left), isInt(right)));
    }

    ValueDescriptor *modulo(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() != Type::integer || right.type() != Type::integer) {
            typeError(op, left, right);
        }
        const std::int64_t a = isInt(left);
        const std::int64_t b = isInt(right);
        
        if (b == 0) {
            zeroDivisor(op, "modulo");
        }
        return new IntegerValueDescriptor(a - floorQuotient(a, b) * b);
    }

    // Phase 4 statement, "List Printing and Equality":
    //   "Two lists are equal when they have the same length and corresponding elements are
    //   equal according to the existing equality rules. Numeric equality permits an integer
    //   and double with the same numeric value to compare equal. Lists of different lengths
    //   or incompatible element categories are unequal. Two empty lists compare equal,
    //   regardless of whether either empty list has already established an element category."
    ValueDescriptor *equals(const ValueDescriptor &left, const ValueDescriptor &right, bool invert) {
        bool same = false;
        if (left.type() == Type::integer && right.type() == Type::integer) {
            same = isInt(left) == isInt(right);
        } else if (isNumeric(left) && isNumeric(right)) {
            same = isDouble(left) == isDouble(right);
        } else if (left.type() == Type::string && right.type() == Type::string) {
            same = isString(left) == isString(right);
        } else if (left.type() == Type::boolean && right.type() == Type::boolean) {
            same = isBool(left) == isBool(right);
        }
        else if (left.type() == Type::list && right.type() == Type::list) {
           // Both operands are lists, so the casts below are safe.
           const ListObject *leftList = static_cast<const ListValueDescriptor &>(left).object();
           const ListObject *rightList = static_cast<const ListValueDescriptor &>(right).object();

           // "Lists of different lengths ... are unequal."
             same = leftList->size() == rightList->size();

            // "corresponding elements are equal according to the existing equality rules"
            // Ownership guide, "Reading an Element": "The list retains ownership of the stored
            // descriptor. ... The caller owns the clone." Each clone is deleted after comparing.
            for (std::size_t i = 0; same && i < leftList->size(); ++i) {
                ValueDescriptor *leftElement = nullptr;
                ValueDescriptor *rightElement = nullptr;
                ValueDescriptor *result = nullptr;

                bool elementsEqual = false;

                try {
                    leftElement = leftList->cloneElementAt(i);
                    rightElement = rightList->cloneElementAt(i);

                    result = equals(*leftElement, *rightElement, false);
                    elementsEqual = isBool(*result);

                } catch (...) {
                    delete leftElement;
                    delete rightElement;
                    delete result;
                    throw;
                }

                delete leftElement;
                delete rightElement;
                delete result;

                if (!elementsEqual) {
                    same = false;
                    break;
                }
            }
        }
        return new BooleanValueDescriptor(same != invert);
    }

    template <typename T>
    bool ordered(const Token &op, const T &a, const T &b) {
        if (op.isLessThanOperator())           return a < b;
        if (op.isLessThanOrEqualOperator())    return a <= b;
        if (op.isGreaterThanOperator())        return a > b;
        if (op.isGreaterThanOrEqualOperator()) return a >= b;
        throw std::logic_error("unsupported infix operator");
    }

    ValueDescriptor *compare(const Token &op, const ValueDescriptor &left, const ValueDescriptor &right) {
        if (left.type() == Type::integer && right.type() == Type::integer) {
            return new BooleanValueDescriptor(ordered(op, isInt(left), isInt(right)));
        }
        if (isNumeric(left) && isNumeric(right)) {
            return new BooleanValueDescriptor(ordered(op, isDouble(left), isDouble(right)));
        }
        if (left.type() == Type::string && right.type() == Type::string) {
            return new BooleanValueDescriptor(ordered(op, isString(left), isString(right)));
        }
        typeError(op, left, right);
    }
}
// evaluate now depends on the namespace for most of the actual evaluating process. 
ValueDescriptor *BinaryExprNode::evaluate(const SymbolTable &symbolTable) const {
    ValueDescriptor *left = leftOperand->evaluate(symbolTable);
    ValueDescriptor *right = nullptr;
    ValueDescriptor *result = nullptr;
    const Token &op = token();

    try {
        right = rightOperand->evaluate(symbolTable);

        if (op.isAdditionOperator ())        {result = add(op, *left, *right);}
        else if (op.isSubtractionOperator()) {result = subtract(op, *left, *right);}
        else if (op.isMultiplicationOperator()) {result = multiply(op, *left, *right);}
        else if (op.isFloorDivisionOperator())  {result = floorDivide(op, *left, *right);}
        else if (op.isDivisionOperator())       {result = divide(op, *left, *right);}
        else if (op.isModuloOperator())         {result = modulo(op, *left, *right);}
        else if (op.isEqualOperator())          {result = equals(*left, *right, false);}
        else if (op.isNotEqualOperator())       {result = equals(*left, *right, true);}
        else                                    {result = compare(op, *left, *right);}
    } catch (...) {
        delete left;
        delete right;   // still nullptr if evaluating the right operand failed
        throw;
    }

    delete left;
    delete right;
    return result;
}


void BinaryExprNode::print() const {
   std::cout << '(';
   leftOperand->print();
   token().print(std::cout);
   rightOperand->print();
   std::cout << ')';
}


UnaryExprNode::UnaryExprNode(Token token, ExprNode *operand)
   : ExprNode{std::move(token)}, expression{operand} {}


UnaryExprNode::~UnaryExprNode() {
   delete expression;
}


ValueDescriptor *UnaryExprNode::evaluate(const SymbolTable &symbolTable) const {
    // phase 3: must delete descriptor after use to avoid memory leaks
    ValueDescriptor *value = expression->evaluate(symbolTable);
    const bool negate = token().isSubtractionOperator();

    if (token().isAdditionOperator() || negate) {
        if (value->type() == Type::integer) {
            const std::int64_t number = isInt(*value);
            delete value;
            return new IntegerValueDescriptor(negate ? -number : number);
        }
        if (value->type() == Type::doubleValue) {
            const double number = isDouble(*value);
            delete value;
            return new DoubleValueDescriptor(negate ? -number : number);
        }
        const string valueType = value->typeName();
        delete value;
        throw std::runtime_error("type error at line " + to_string(token().lineNumber()) +
            ", column " + to_string(token().columnNumber()) +
            ": unary operator '" + string(1, token().symbol()) +
            "' cannot be applied to " + valueType);
    }

   delete value;
   throw std::logic_error("unsupported prefix operator");
}


void UnaryExprNode::print() const {
   std::cout << token().symbol();
   expression->print();
}


IntegerLiteral::IntegerLiteral(Token token) : ExprNode{std::move(token)} {}


void IntegerLiteral::print() const {
   token().print(std::cout);
}


ValueDescriptor *IntegerLiteral::evaluate(const SymbolTable &) const {
   return new IntegerValueDescriptor(token().integerValue());
}


DoubleLiteral::DoubleLiteral(Token token) : ExprNode{std::move(token)} {}


void DoubleLiteral::print() const {
   token().print(std::cout);
}


ValueDescriptor *DoubleLiteral::evaluate(const SymbolTable &) const {
   return new DoubleValueDescriptor(token().doubleValue());
}


BooleanLiteral::BooleanLiteral(Token token) : ExprNode{std::move(token)} {}


void BooleanLiteral::print() const {
   token().print(std::cout);
}


ValueDescriptor *BooleanLiteral::evaluate(const SymbolTable &) const {
   return new BooleanValueDescriptor(token().isTrueKeyword());
}


StringLiteral::StringLiteral(Token token) : ExprNode{std::move(token)} {}


void StringLiteral::print() const {
   token().print(std::cout);
}


ValueDescriptor *StringLiteral::evaluate(const SymbolTable &) const {
   return new StringValueDescriptor(token().stringValue());
}


StrConversion::StrConversion(Token token, ExprNode *operand)
   : ExprNode{std::move(token)}, expression{operand} {}


StrConversion::~StrConversion() {
   delete expression;
}


void StrConversion::print() const {
   std::cout << "str(";
   expression->print();
   std::cout << ')';
}


ValueDescriptor *StrConversion::evaluate(const SymbolTable &symbolTable) const {
   ValueDescriptor *value = expression->evaluate(symbolTable);
   ValueDescriptor *result = new StringValueDescriptor(value->toString());
   delete value;
   return result;
}


Variable::Variable(Token token) : ExprNode{std::move(token)} {}


void Variable::print() const {
    // Phase 3: now prints the variable name instead of the token
    std::cout << token().identifier();
}

// Phase 3: now Returns a clone of the value descriptor associated with the variable in the symbol table
ValueDescriptor *Variable::evaluate(const SymbolTable &symbolTable) const {
   return symbolTable.getValueFor(token().identifier())->clone();
}

// Phase 4: index check shared by subscripting and indexed assignment.
// Phase 4 statement, "String Subscripting" / "List Subscripting":
//   "The index must satisfy: 0 <= index < len(string)"
//   "A non-integer or out-of-range index is a runtime error. Negative indexes are not supported."
// This helper checks "integer" and "non-negative"; the upper bound is checked by the caller.
std::size_t requireIndex(const ValueDescriptor *value, const std::string &operation) {
    if (value->type() != Type::integer) {
        throw runtime_error("type error: " + operation + ": index must be int, got " + value->typeName());
    }
    int64_t n = dynamic_cast<const IntegerValueDescriptor*>(value)->value();
    if (n < 0) {
        throw std::out_of_range(operation + ": index must be non-negative");
    }
    return static_cast<std::size_t>(n);
}


// Phase 4: list literal
// Grammar: <list-literal> → [ [ <rel-expr> { , <rel-expr> } ] ]
// Phase 4 statement, "Element Categories": "A nonempty list literal establishes its category
//   from its elements. All elements must be compatible."
ListLiteral::ListLiteral(Token token, std::vector<ExprNode *> elements)
    : ExprNode{std::move(token)}, elementExprs{std::move(elements)} {}

ListLiteral::~ListLiteral() {
    for (ExprNode *element : elementExprs) {
        delete element;
    }
}
// prints in the form [<element1>, <element2>, ..., <elementN>]
void ListLiteral::print() const {
    std::cout << '[';
    for (std::size_t i = 0; i < elementExprs.size(); ++i) {
        if (i > 0)
            std::cout << ", ";
        elementExprs[i]->print();
    }
    std::cout << ']';
}

// Ownership guide, "Error Paths Are Ownership Paths": "Organize the code so that an error
//   path either deletes every temporary it owns or reports the error before ownership is
//   acquired." Example given there: "A list literal whose third element has an incompatible type".
ValueDescriptor *ListLiteral::evaluate(const SymbolTable &symbolTable) const {
    ListValueDescriptor *listValue = new ListValueDescriptor(new ListObject());

    try {
        for (ExprNode *elementExpr : elementExprs) {
            ValueDescriptor *elementValue = nullptr;

            try {
                elementValue = elementExpr->evaluate(symbolTable);
                listValue->object()->append(elementValue);

            } catch (...) {
                delete elementValue;
                throw;
            }
        }
    }
    catch (...) {
        // Deletes the list and all successfully appended elements.
        delete listValue;
        throw;
    }
    return listValue;
}

// Phase 4: one-argument calls. Each node owns its operand expression.
// Grammar: <one-argument-call> → <one-argument-name> ( <rel-expr> )
//          <one-argument-name> → "int" | "float" | "str" | "len" | "split" | "pop"
// Phase 3/4 rule (Phase 4 statement, "Runtime Representation and Ownership"):
//   "Every expression's evaluate function returns a newly allocated descriptor owned by its caller."

// int(x)
IntConversion::IntConversion(Token token, ExprNode *operand) 
: ExprNode{std::move(token)}, expression{operand} {}


IntConversion::~IntConversion() {
    delete expression;
}

// print for int(x) is just "int(<operand>)"
void IntConversion::print() const {
    cout << "int(";
    expression->print();
    cout << ")";
}

// Phase 4 statement, "int, float, and str": "int(expression) accepts:
//   An integer, which produces the same integer value
//   A string containing an optional leading + or - followed by one or more decimal digits
//   It does not accept a double, Boolean, list, empty string, surrounding whitespace, decimal
//   point, or exponent notation. The converted value must fit in std::int64_t."
//   "An invalid conversion is a runtime error. The error message should identify the
//   conversion and the source value or datatype."
ValueDescriptor *IntConversion::evaluate(const SymbolTable &symbolTable) const {
   // The operand's descriptor is owned here and must be deleted on every path.
   ValueDescriptor *value = expression->evaluate(symbolTable);
   std::string where = "type error at line " + std::to_string(token().lineNumber()) +
                        ", column " + std::to_string(token().columnNumber()) + ": ";
    
    // "An integer, which produces the same integer value"
    if (value->type() == ValueDescriptor::Type::integer) {
        std::int64_t intValue = static_cast<IntegerValueDescriptor *>(value)->value();
        delete value;
        return new IntegerValueDescriptor(intValue);
    }
    // "It does not accept a double, Boolean, list"
    if (value->type() != ValueDescriptor::Type::string) {
        std::string typeName = value->typeName();
        delete value;
        throw std::runtime_error(where + "int() cannot be applied to " + typeName);
    }
    // Copy the text, then free the operand; nothing below needs it.
    string text = static_cast<StringValueDescriptor *>(value)->value();
    delete value;

    // "an optional leading + or - followed by one or more decimal digits"
    // (rejects "empty string, surrounding whitespace, decimal point, or exponent notation")
    size_t pos = 0;
    if (pos < text.size() && (text[pos] == '+' || text[pos] == '-')) {
        ++pos;
    }
    bool valid = pos < text.size();
    for (; pos < text.size(); ++pos) {
        if (text[pos] < '0' || text[pos] > '9') {
            valid = false;
            break;
        }
    }
    // "The converted value must fit in std::int64_t." (stoll throws out_of_range otherwise)
    try {
        if (!valid) {
            throw std::invalid_argument("invalid integer string");
        }
        std::int64_t intValue = std::stoll(text);
        return new IntegerValueDescriptor(intValue);
    } catch (const std::exception &e) {
        throw std::runtime_error(where + "invalid integer string: " + text);
    }
}

//Phase 4: float(x)
FloatConversion::FloatConversion(Token token, ExprNode *operand)
: ExprNode(std::move(token)), expression{operand} {}

FloatConversion::~FloatConversion() {
    delete expression;
}


void FloatConversion::print() const {
    cout << "float(";
    expression->print();
    cout << ")";
}

// Phase 4 statement, "int, float, and str": "float(expression) accepts:
//   A double, which produces the same double value
//   An integer, which produces the corresponding double value
//   A string containing an optional leading + or -, one or more decimal digits, and an
//   optional fractional part containing a decimal point followed by one or more digits"
//   "Forms such as float(".5"), float("5."), and float("1e3") are runtime conversion errors.
//   A successful float conversion produces a DoubleValueDescriptor ... The result must be a
//   finite value representable by double."
ValueDescriptor *FloatConversion::evaluate(const SymbolTable &symbolTable) const {
    ValueDescriptor *value = expression->evaluate(symbolTable);
    std::string where = "type error at line " + std::to_string(token().lineNumber()) +
                        ", column " + std::to_string(token().columnNumber()) + ": ";

    // "An integer, which produces the corresponding double value"
    if (value->type() == ValueDescriptor::Type::integer) {
        double d = static_cast<double>(static_cast<IntegerValueDescriptor *>(value)->value());
        delete value;
        return new DoubleValueDescriptor(d);
    }
    // "A double, which produces the same double value"
    if (value->type() == ValueDescriptor::Type::doubleValue) {
        double d = static_cast<DoubleValueDescriptor *>(value)->value();
        delete value;

        if (!std::isfinite(d)) {
            throw std::runtime_error(where + "float() result must be finite");
        }

        return new DoubleValueDescriptor(d);
    }

    // Booleans and lists are not in the accepted list above.
    if (value->type() != ValueDescriptor::Type::string) {
        std::string typeName = value->typeName();
        delete value;
        throw std::runtime_error(where + "float() cannot be applied to " + typeName);
    }

    // Copy the text, then free the operand; nothing below needs it.
    std::string text = static_cast<StringValueDescriptor *>(value)->value();
    delete value;

    // "an optional leading + or -, one or more decimal digits, and an optional fractional
    // part containing a decimal point followed by one or more digits"
    // "both float("42") and float("42.5") are valid"
    std::size_t i = 0;
    if (i < text.size() && (text[i] == '+' || text[i] == '-'))
        i++;

    std::size_t digitsBefore = 0;
    while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
        i++;
        digitsBefore++;
    }

    bool valid = digitsBefore > 0;
    // "one or more decimal digits": rejects ".5"                
    if (valid && i < text.size()) {
        // skip the '.'
        if (text[i] != '.') {
            valid = false;                        
        } else {
            i++;                                  
            std::size_t digitsAfter = 0;
            while (i < text.size() && text[i] >= '0' && text[i] <= '9') {
                i++;
                digitsAfter++;
            }
            // "a decimal point followed by one or more digits": rejects "5."
            valid = digitsAfter > 0 && i == text.size();
        }
    }
    if (!valid)
        throw std::runtime_error(where + "float() cannot convert \"" + text + "\"");

    // "The result must be a finite value representable by double." (stod throws out_of_range)
    try {
        double result = std::stod(text);

        if (!std::isfinite(result)) {
            throw std::runtime_error(
                    where + "float() result must be finite"
            );
        }

        return new DoubleValueDescriptor(result);
    }
    catch (const std::out_of_range &) {
        throw std::runtime_error(where + "float() value out of range: \"" + text + "\"");
    }
}

// Phase 4: len(x)
LenExpr::LenExpr(Token token, ExprNode *operand) 
: ExprNode(std::move(token)), expression{operand} {}

LenExpr::~LenExpr() {
    delete expression;
}

void LenExpr::print() const {
    cout << "len(";
    expression->print();
    cout << ")";
}

// Phase 4 statement, "Length": "len(expression) accepts a string or a list and returns an
//   integer. For a string, the result is the number of characters. Source strings and test
//   data for this project use ASCII, so one stored byte represents one character."
ValueDescriptor *LenExpr::evaluate(const SymbolTable &symbolTable) const {
    ValueDescriptor *value = expression->evaluate(symbolTable);
    if (value->type() == ValueDescriptor::Type::string) {
        std::size_t length = static_cast<StringValueDescriptor *>(value)->value().size();
        delete value;
        return new IntegerValueDescriptor(static_cast<std::int64_t>(length));
    } else if (value->type() == ValueDescriptor::Type::list) {
        std::size_t length = static_cast<ListValueDescriptor *>(value)->object()->size();
        delete value;
        return new IntegerValueDescriptor(static_cast<std::int64_t>(length));
    } else {
        std::string typeName = value->typeName();
        delete value;
        throw std::runtime_error("len: argument must be string or list, got " + typeName);
    }
}

// Phase 4: split(s)
SplitExpr::SplitExpr(Token token, ExprNode *operand)
: ExprNode(std::move(token)), expression{operand} {}

SplitExpr::~SplitExpr() {
    delete expression;
}

void SplitExpr::print() const {
    cout << "split(";
    expression->print();
    cout << ")";
}

// Phase 4 statement, "Splitting a String": "split(expression) requires a string and returns a
//   list of strings. It splits the string at runs of spaces or horizontal tab characters.
//     Leading and trailing separators are discarded.
//     Consecutive separators behave as one separator.
//     No empty fields are produced.
//     An empty or whitespace-only string produces an empty list."
ValueDescriptor *SplitExpr::evaluate(const SymbolTable &symbolTable) const {
    ValueDescriptor *value = expression->evaluate(symbolTable);

    if (value->type() != ValueDescriptor::Type::string) {
        std::string typeName = value->typeName();
        delete value;
        throw std::runtime_error("type error at line " + std::to_string(token().lineNumber()) +
                                 ", column " + std::to_string(token().columnNumber()) +
                                 ": split() cannot be applied to " + typeName);
    }

    // Copy (not a reference): value is deleted on the next line.
    std::string text = static_cast<StringValueDescriptor *>(value)->value();
    delete value;

    // "The returned list has the string element category even when it is empty. Therefore,
    // appending a numeric or Boolean value to the result of split is a runtime type error."
    ListValueDescriptor *listValue = new ListValueDescriptor(new ListObject(ListElementType::string));

    try {
        std::string word;
        for (char c: text) {
            if (c == ' ' || c == '\t') {
                // "Consecutive separators behave as one separator." / "No empty fields are produced."
                if (!word.empty()) {
                    ValueDescriptor *element = new StringValueDescriptor(word);

                    try {
                        listValue->object()->append(element);
                    } catch (...) {
                        delete element;
                        throw;
                    }
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        // The last field has no separator after it ("trailing separators are discarded").
        if (!word.empty()) {
            ValueDescriptor *element = new StringValueDescriptor(word);

        try {
            listValue->object()->append(element);
        } catch (...) {
            delete element;
            throw;
        }
    }
} catch (...) {
        delete listValue;     // the list object owns, and deletes, the words already appended
        throw;
    }
    return listValue;
}

// Phase 4: pop(list)
PopExpr::PopExpr(Token token, ExprNode *operand)
: ExprNode(std::move(token)), expression{operand} {}

PopExpr::~PopExpr() {
    delete expression;
}


void PopExpr::print() const {
    std::cout << "pop(";
    expression->print();
    std::cout << ")";
}
// Phase 4 statement, "Appending and Popping": "pop(list) is a built-in expression. It removes
//   and returns the final element ... Popping an empty list is a runtime error. Removing the
//   final element does not erase the list's established element category."
// Ownership guide, "Popping": "pop removes the pointer from the vector and returns it.
//   Ownership transfers from the list object to the caller of pop. The descriptor must not be
//   cloned and the removed pointer must not be deleted by the list."
ValueDescriptor *PopExpr::evaluate(const SymbolTable &symbolTable) const {
    // Evaluating the argument gives a new descriptor for the same shared list object
    // (Phase 4 statement: "For a list, clone() creates another descriptor for the same
    // shared object."), so popping through it is visible through every alias.
    ValueDescriptor *value = expression->evaluate(symbolTable);

    if (value->type() != ValueDescriptor::Type::list) {
        std::string typeName = value->typeName();
        delete value;

        throw std::runtime_error(
                "pop: expected list, got " + typeName
        );
    }

    try {
        auto *list =
                static_cast<ListValueDescriptor *>(value);

        ValueDescriptor *result = list->object()->pop();

        delete value;
        return result;

    } catch (...) {
        delete value;
        throw;
    }
}

// Phase 4: subscript expression (token = the '[')
// Grammar: <postfix-expression> → <atom> [ [ <rel-expr> ] ]
//   "The optional subscript in <postfix-expression> permits one indexing operation.
//   Chained subscripts are not part of Phase 4."
SubscriptExpr::SubscriptExpr(Token token, ExprNode *target, ExprNode *index)
: ExprNode(std::move(token)), targetExpr{target}, indexExpr{index} {}


SubscriptExpr::~SubscriptExpr() {
    delete targetExpr;
    delete indexExpr;
}                                    


void SubscriptExpr::print() const {
    targetExpr->print();
    std::cout << '[';
    indexExpr->print();
    std::cout << ']';
}


// Phase 4 statement, "String Subscripting": "The result is a new one-character string. The
//   index must satisfy: 0 <= index < len(string). A non-integer or out-of-range index is a
//   runtime error. Negative indexes are not supported."
// Phase 4 statement, "List Subscripting": "The result is a newly allocated descriptor
//   representing the selected value. The list continues to own the descriptor stored in its vector."
// Ownership guide, "Reading an Element": "The subscript operation returns elements[i]->clone().
//   The caller owns the clone. Returning the stored pointer directly would give two owners the
//   same pointer and eventually cause a double deletion."
ValueDescriptor *SubscriptExpr::evaluate(const SymbolTable &symbolTable) const {
    ValueDescriptor *target = targetExpr->evaluate(symbolTable);
    ValueDescriptor *index = nullptr;

    try {
        index = indexExpr->evaluate(symbolTable);

        std::size_t i = requireIndex(index, "subscript");

        ValueDescriptor *result = nullptr;

        if (target->type() == ValueDescriptor::Type::list) {
            auto *list = static_cast<ListValueDescriptor *>(target);

            result = list->object()->cloneElementAt(i);
        }
        else if (target->type() == ValueDescriptor::Type::string) {
            auto *str = static_cast<StringValueDescriptor *>(target);

            const std::string &text = str->value();

            if (i >= text.size()) {
                throw std::out_of_range("string subscript: index " + std::to_string(i) + " out of range");
            }

            result = new StringValueDescriptor(std::string(1, text[i]));
        }
        else {
            throw std::runtime_error("subscript: cannot subscript " + target->typeName());
        }

        delete target;
        delete index;

        return result;

    } catch (...) {
        delete target;
        delete index;
        throw;
    }
}

// Phase 4: input()
// Grammar: <input-expression> → "input" ( )
//   Phase 4 statement, "input()": "This phase supports only the zero-argument form ...
//   Supplying an argument is a syntax error." The parser therefore always passes nullptr.
InputExpr::InputExpr(Token token, ExprNode *prompt)
: ExprNode(std::move(token)), promptExpr{prompt} {}  // prompt may be nullptr

InputExpr::~InputExpr() {
    delete promptExpr;
}                     

void InputExpr::print() const {
    std::cout << "input()";
}

// Phase 4 statement, "input()": "When your interpreter evaluates input(), it reads one complete
//   line from standard input and returns a string descriptor. The line terminator is not part
//   of the returned string. Other characters, including leading and trailing spaces, are
//   preserved." "input() always returns a string. It does not infer a datatype from the text
//   that it reads." "Reaching the end of standard input when input() attempts to read another
//   line is a runtime error."
ValueDescriptor *InputExpr::evaluate(const SymbolTable &symbolTable) const {
    (void)symbolTable;

    std::string line;

    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("input: end of input reached");
    }

    return new StringValueDescriptor(line);
    
}

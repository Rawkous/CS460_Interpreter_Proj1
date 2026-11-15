#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <cstdint>

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

// Phase 4: helper function implementation
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


// Phase 4: new ListLiteral class
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

ValueDescriptor *ListLiteral::evaluate(const SymbolTable &symbolTable) const {
    ListValueDescriptor *listValue = new ListValueDescriptor(new ListObject());

    for (ExprNode *elementExpr : elementExprs) {
        ValueDescriptor *elementValue = nullptr;
        try {
            elementValue = elementExpr->evaluate(symbolTable);
            listValue->object()->append(elementValue);
        } catch (...) {
            delete elementValue;
            delete listValue;
            throw;
        }
    }
    return listValue;
}

// Phase 4: one-argument calls. Each node owns its operand expression.
// int(x)
IntConversion::IntConversion(Token token, ExprNode *operand) 
: ExprNode{std::move(token)}, expression{operand} {}


IntConversion::~IntConversion() {
    delete expression;
}

void IntConversion::print() const {}


ValueDescriptor *IntConversion::evaluate(const SymbolTable &symbolTable) const {
    /*
    int → copy; double → truncate; string → strict parse (the whole string must be an integer
    , otherwise throw); anything else → type error
    */
}

//Phase 4: float(x)
FloatConversion::FloatConversion(Token token, ExprNode *operand)
: ExprNode(std::move(token)), expression{operand} {}

FloatConversion::~FloatConversion() {
    delete expression;
}


void FloatConversion::print() const {}
ValueDescriptor *FloatConversion::evaluate(const SymbolTable &symbolTable) const {
    /*
    int/double → double; 
    string → strict parse (std::stod plus a check that every character was consumed);
     otherwise type error*/
}

// Phase 4: len(x): x is a string or a list
LenExpr::LenExpr(Token token, ExprNode *operand) 
: ExprNode(std::move(token)), expression{operand} {}

LenExpr::~LenExpr() {
    delete expression;
}

void LenExpr::print() const {}
ValueDescriptor *LenExpr::evaluate(const SymbolTable &symbolTable) const {
    /*
    tring → value().size(); 
    list → object()->size(); 
    otherwise type error → IntegerValueDescriptor*/
}

// Phase 4: split(s): returns a new string-category list
SplitExpr::SplitExpr(Token token, ExprNode *operand)
: ExprNode(std::move(token)), expression{operand} {}

SplitExpr::~SplitExpr() {
    delete expression;
}

void SplitExpr::print() const {}
ValueDescriptor *SplitExpr::evaluate(const SymbolTable &symbolTable) const {
    /*must be a string → new ListValueDescriptor(new ListObject(ListElementType::string)),
     then append(new StringValueDescriptor(word))
     for each whitespace-separated word. 
     If an append throws, delete the list.*/
}

// Phase 4: pop(list): removes the last element and transfers it to the caller
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
ValueDescriptor *PopExpr::evaluate(const SymbolTable &symbolTable) const {
    /* must be a list → result = handle->object()->pop(),
     delete the handle, 
     return result (the caller now owns it).
      If the list is empty, 
      pop throws, 
      so delete the handle before rethrowing.*/
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

// Phase 4: subscript expression
// token = the '['
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


ValueDescriptor *SubscriptExpr::evaluate(const SymbolTable &symbolTable) const {
    /*
    target = targetExpr->evaluate          (evaluate the target first)
    index  = nullptr
    try:
        index = indexExpr->evaluate
        i = requireIndex(index, "subscript")
        if target is a list   → result = object()->cloneElementAt(i)   (it does its own range check)
        if target is a string → range-check i against size → new StringValueDescriptor(string(1, s[i]))
        else                  → type error "cannot subscript <type>"
    catch(...): delete target; delete index; throw;
    delete target; delete index; return result
    */
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
InputExpr::InputExpr(Token token, ExprNode *prompt)
: ExprNode(std::move(token)), promptExpr{prompt} {}  // prompt may be nullptr

InputExpr::~InputExpr() {
    delete promptExpr;
}                     

void InputExpr::print() const {
    std::cout << "input()";
}

ValueDescriptor *InputExpr::evaluate(const SymbolTable &symbolTable) const {
    /*evaluate: if prompt → evaluate it, print toString() with no newline, delete it
          std::getline(std::cin, line) → return new StringValueDescriptor(line)*/
    (void)symbolTable;

    std::string line;

    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("input: end of input reached");
    }

    return new StringValueDescriptor(line);
    
}

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <memory>
#include <cstdint>

#include "Expr.hpp"

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
   // Making Value Descriptor unique pointers so no need to delete
    std::unique_ptr<ValueDescriptor> left{leftOperand->evaluate(symbolTable)};
    std::unique_ptr<ValueDescriptor> right{rightOperand->evaluate(symbolTable)};
    const Token &op = token();

    if (op.isAdditionOperator ())        {return add(op, *left, *right);}
    if (op.isSubtractionOperator())      {return subtract(op, *left, *right);}
    if (op.isMultiplicationOperator())   {return multiply(op, *left, *right);}
    if (op.isFloorDivisionOperator())    {return floorDivide(op, *left, *right);}
    if (op.isDivisionOperator())         {return divide(op, *left, *right);}
    if (op.isModuloOperator())           {return modulo(op, *left, *right);}
    if (op.isEqualOperator())            {return equals(*left, *right, false);}
    if (op.isNotEqualOperator())         {return equals(*left, *right, true);}
    return compare(op, *left, *right);
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
    std::unique_ptr<ValueDescriptor> value{expression->evaluate(symbolTable)};
    const bool negate = token().isSubtractionOperator();

    if (token().isAdditionOperator() || negate) {
        if (value->type() == Type::integer) {
            const std::int64_t number = isInt(*value);
            return new IntegerValueDescriptor(negate ? -number : number);
        }
        if (value->type() == Type::doubleValue) {
            const double number = isDouble(*value);
            return new DoubleValueDescriptor(negate ? -number : number);
        }
        throw std::runtime_error("type error at line " + to_string(token().lineNumber()) +
            ", column " + to_string(token().columnNumber()) +
            ": unary operator '" + string(1, token().symbol()) +
            "' cannot be applied to " + value->typeName());
    }

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
   std::unique_ptr<ValueDescriptor> value{expression->evaluate(symbolTable)};
   return new StringValueDescriptor(value->toString());
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

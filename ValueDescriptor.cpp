#include "ValueDescriptor.hpp"
#include <sstream>

using namespace std;

// Constructor: stores value using initializer list
IntegerValueDescriptor::IntegerValueDescriptor(std::int64_t value) : integerValue{value} { }

// selects the type of value it is from the enum list, here it's integer
ValueDescriptor::Type IntegerValueDescriptor::type() const {
    return Type::integer;
}
// returns the literal lower case text for integer
string IntegerValueDescriptor::typeName() const {
    return "integer";
}
// returns decimal notation used by print and str conversion
string IntegerValueDescriptor::toString() const {
    return to_string(integerValue);
}
// dynamically allocates brand new descriptor with the same value
ValueDescriptor *IntegerValueDescriptor::clone() const {
    // caller owns:
    return new IntegerValueDescriptor(integerValue);
}
// finally returns the integer value stored in the descriptor
std::int64_t IntegerValueDescriptor::value() const {
    return integerValue;
}


DoubleValueDescriptor::DoubleValueDescriptor(double value) : doubleValue{value} {}

ValueDescriptor::Type DoubleValueDescriptor::type() const {
    return Type::doubleValue;
}
string DoubleValueDescriptor::typeName() const {
    return "double";
}
// Print doubles using the default formatting provided by C++ stream output.
string DoubleValueDescriptor::toString() const {
    ostringstream out;
    out << doubleValue;
    return out.str();
}
ValueDescriptor *DoubleValueDescriptor::clone() const {
    return new DoubleValueDescriptor(doubleValue);
}
double DoubleValueDescriptor::value() const {
    return doubleValue;
}

BooleanValueDescriptor::BooleanValueDescriptor(bool value) : booleanValue{value} {}

ValueDescriptor::Type BooleanValueDescriptor::type() const {
    return Type::boolean;
}
string BooleanValueDescriptor::typeName() const {
    return "boolean";
}
// Print Booleans as True or False.
string BooleanValueDescriptor::toString() const {
    return booleanValue ? "True" : "False";
}
ValueDescriptor *BooleanValueDescriptor::clone() const {
    return new BooleanValueDescriptor(booleanValue);
}
bool BooleanValueDescriptor::value() const {
    return booleanValue;
}

StringValueDescriptor::StringValueDescriptor(std::string value) : stringValue{value} {}

ValueDescriptor::Type StringValueDescriptor::type() const {
    return Type::string;
}
string StringValueDescriptor::typeName() const {
    return "string";
}
// Print strings without their surrounding quotation marks.
string StringValueDescriptor::toString() const {
    return stringValue;
}
ValueDescriptor *StringValueDescriptor::clone() const {
    return new StringValueDescriptor(stringValue);
}
const string &StringValueDescriptor::value() const {
    return stringValue;
}

#include "ListObject.hpp"
#include <stdexcept>
#include <string>
using namespace std;




ListObject::ListObject(ListElementType initialType)
: references(0), category(initialType) {}

ListObject::~ListObject() {

    for (ValueDescriptor *i : elements) {

        delete i;
    }
}

void ListObject::retain() {

    references++;
}

// Decrements the reference count and returns true when it reaches zero.

bool ListObject::release() {
    references--;

    return 0 == references;
}


std::size_t ListObject::referenceCount() const {

    return references;
}

std::size_t ListObject::size() const {

    return elements.size();
}
// helper function to return the type of the list elements
ListElementType ListObject::classifyElementType(const ValueDescriptor *value) const {
    switch (value->type()) {
        case ValueDescriptor::Type::integer:
        case ValueDescriptor::Type::doubleValue:
            return ListElementType::numeric;
        case ValueDescriptor::Type::boolean:
            return ListElementType::boolean;
        case ValueDescriptor::Type::string:
            return ListElementType::string;
        default:
            return ListElementType::unset;
    }
}
// helper function to check compatibility of the list elements
static void compatCheck(const ValueDescriptor *value, ListElementType category, ListElementType valueCategory, const string &operation) {
    // throws if values are incompatible, or if the value is a list
    if (value->type() == ValueDescriptor::Type::list) {
        throw std::runtime_error("type error: " + operation + ": a list cannot be an element of another list");
    }
    if (category != ListElementType::unset && valueCategory != category)
        throw runtime_error("type error: " + operation + ": cannot store " + value->typeName() + " in a " + (category == ListElementType::numeric ? "numeric" : category == ListElementType::boolean ? "boolean" : "string") + " list");
}

// helper function to build index error messages that name the operation, index, and length
static string indexError(const string &operation, std::size_t index, std::size_t length) {
    return operation + ": index " + to_string(index) + " out of range for list of length " + to_string(length);
}

ListElementType ListObject::elementType() const {

    return category;
}

// Returns a newly allocated descriptor owned by the caller.

ValueDescriptor* ListObject::cloneElementAt(std::size_t index) const {
    if (index >= elements.size()) {
        throw std::out_of_range(indexError("list subscript", index, elements.size()));
    }
    return elements[index]->clone();
}

// On success, each of these functions takes ownership of value.
void ListObject::setElement(std::size_t index, ValueDescriptor *value) {
    //bounds check
    if (index >= elements.size()) {
        throw std::out_of_range(indexError("list assignment", index, elements.size()));
    }

    const ListElementType valueCategory = classifyElementType(value);

    // call compatCheck to ensure the new descriptor is compatible with the list category
    compatCheck(value, category, valueCategory, "list assignment");

    // if the list category is unset, set it to the type of the first element
    if (category == ListElementType::unset) {
        category = valueCategory;
    }
    //Delete the descriptor currently stored at the index.
    delete elements[index];
    //Store the new descriptor and transfer ownership to the list object.
    elements[index] = value;
}


// Examine list, if argument is there, evaluate and append
void ListObject::append(ValueDescriptor *value) {
    // check element category, if unset, set to the type of the first element 
    const ListElementType valueType = classifyElementType(value);
    
    // evaluate compatibility of the new descriptor with the list category'
    compatCheck(value, category, valueType, "append");

    // if the list category is unset, set it to the type of the first element
    if (category == ListElementType::unset) {
        category = valueType;
    }
    elements.push_back(value);
}

// Returns the removed descriptor and transfers ownership to the caller.
ValueDescriptor* ListObject::pop() {
    // if its out of bounds, throw an exception
    if (elements.empty()) {
        throw std::out_of_range("pop: cannot pop from an empty list");
    }
    // get the last element
    ValueDescriptor *lastElement = elements.back();
    // remove the last element from the vector
    elements.pop_back();
    // return the last element
    return lastElement;
}


ListValueDescriptor::ListValueDescriptor(ListObject *object) {
    listObject = object;
    listObject->retain();
}


ListValueDescriptor::~ListValueDescriptor() {

    if (listObject->release()) {

        delete listObject;

    }
}

ValueDescriptor *ListValueDescriptor::clone() const {

    return new ListValueDescriptor(listObject);
}
ValueDescriptor::Type ListValueDescriptor::type() const {
    return Type::list;
}

string ListValueDescriptor::typeName() const {
    return "list";
}

string ListValueDescriptor::toString() const {
    string result = "[";
    for (size_t i = 0; i < listObject->size(); i++) {
        if (i > 0) {
            result += ", ";
        }
        ValueDescriptor *element = listObject->cloneElementAt(i);
        if (element->type() == ValueDescriptor::Type::string) {
        result += "\"" + element->toString() + "\"";
        }
        else {
            result += element->toString();
        }
        delete element;
    }
    result += "]";
    return result;
}

ListObject *ListValueDescriptor::object() {
    return listObject;
}
const ListObject *ListValueDescriptor::object() const {
    return listObject;
}

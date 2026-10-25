#include <iostream>
#include <stdexcept>

#include "SymbolTable.hpp"

// Phase 3: setValueFor now takes a ValueDescriptor pointer instead of an int
void SymbolTable::setValueFor(const std::string &variableName, ValueDescriptor *value) {
    // Must delete old descriptor, before replacing it with new assignment.
    ValueDescriptor *oldValue = values[variableName];
    if (oldValue == value) {
        return;
    }
    delete oldValue;
    
    values[variableName] = value;
}

bool SymbolTable::isDefined(const std::string &variableName) const {
    return values.contains(variableName);
}

// Phase 3: added the ValueDescriptor return type
const ValueDescriptor *SymbolTable::getValueFor(const std::string &variableName) const {
    const auto entry = values.find(variableName);
    if (entry == values.end())
        throw std::runtime_error("use of undefined variable '" + variableName + "'");
    return entry->second;
}

   
// Phase 3: Updated print() method to use ValueDescriptor's toString() method
void SymbolTable::print() const {
    for (const auto &[variable, value] : values)
        std::cout << variable << " = " << value->toString() << '\n';
}

// Phase 3: Symbol table must also delete its remaining descriptors when destroyed.
SymbolTable::~SymbolTable() {
    for (const auto &[variable, value] : values)
        delete value;
}

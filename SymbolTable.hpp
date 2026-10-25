#ifndef EXPRINTER_SYMBOLTABLE_HPP
#define EXPRINTER_SYMBOLTABLE_HPP

#include <map>
#include <string>

#include "ValueDescriptor.hpp"

class SymbolTable {
public:
    // Phase 3: added Constructor and Destructor
    SymbolTable() = default;
    ~SymbolTable();

    SymbolTable(const SymbolTable &) = delete;
    SymbolTable &operator=(const SymbolTable &) = delete;
    SymbolTable(SymbolTable &&) = delete;
    SymbolTable &operator=(SymbolTable &&) = delete;

    // Phase 3: SetValueFor now takes a ValueDescriptor pointer instead of an int
    void setValueFor(const std::string &variableName, ValueDescriptor *value);
    [[nodiscard]] bool isDefined(const std::string &variableName) const;
    // Phase 3: read-only access; Table still owns it. Caller must not delete it.
    [[nodiscard]] const ValueDescriptor *getValueFor(const std::string &variableName) const;
    void print() const;

    
private:
    std::map<std::string, ValueDescriptor *> values;
};

#endif // EXPRINTER_SYMBOLTABLE_HPP

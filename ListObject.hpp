#ifndef LISTOBJECT_HPP
#define LISTOBJECT_HPP

#include "ValueDescriptor.hpp"
#include <cstddef>
#include <vector>


enum class ListElementType {
    unset,
    numeric,
    boolean,
    string
};

class ListObject {
public:
    explicit ListObject(
        ListElementType initialType = ListElementType::unset);
    ~ListObject();

    ListObject(const ListObject &) = delete;
    ListObject &operator=(const ListObject &) = delete;

    void retain();

    // Decrements the reference count and returns true when it reaches zero.
    [[nodiscard]] bool release();

    [[nodiscard]] std::size_t referenceCount() const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] ListElementType elementType() const;

    // Returns a newly allocated descriptor owned by the caller.
    [[nodiscard]] ValueDescriptor *cloneElementAt(std::size_t index) const;

    // On success, each of these functions takes ownership of value.
    void setElement(std::size_t index, ValueDescriptor *value);
    void append(ValueDescriptor *value);

    // Returns the removed descriptor and transfers ownership to the caller.
    [[nodiscard]] ValueDescriptor *pop();

private:
    std::size_t references{0};
    ListElementType category{ListElementType::unset};
    std::vector<ValueDescriptor *> elements;

    ListElementType classifyElementType(const ValueDescriptor *value) const;

};

class ListValueDescriptor final : public ValueDescriptor {
public:
    explicit ListValueDescriptor(ListObject *object);
    ~ListValueDescriptor() override;

    ListValueDescriptor(const ListValueDescriptor &) = delete;
    ListValueDescriptor &operator=(const ListValueDescriptor &) = delete;

    [[nodiscard]] Type type() const override;
    [[nodiscard]] std::string typeName() const override;
    [[nodiscard]] std::string toString() const override;
    [[nodiscard]] ValueDescriptor *clone() const override;

    [[nodiscard]] ListObject *object();
    [[nodiscard]] const ListObject *object() const;

private:
    ListObject *listObject;
};

#endif // LISTOBJECT_HPP

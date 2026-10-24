CXX      = c++
CXXFLAGS = -g -std=c++20 -Wall -Wextra -Wpedantic -MMD -MP
TARGET   = interpreter.x
OBJECTS  = Token.o Tokenizer.o Parser.o Range.o Expr.o ValueDescriptor.o SymbolTable.o Statements.o main.o

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pull in the dependency files the compiler generates (-MMD).
-include $(OBJECTS:.o=.d)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) $(TARGET)

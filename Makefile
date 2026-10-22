CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

TARGET := stringcalc
SRC    := main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(TARGET)
	./$(TARGET) tests.txt

clean:
	rm -f $(TARGET)

.PHONY: run clean
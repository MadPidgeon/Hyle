CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g -O2 -fwrapv
TARGET = $(shell basename $(PWD))

OBJDIR = obj

.PHONY: all clean

.SUFFIXES:

all: $(TARGET)

clean:
	@echo "Cleaning"
	@rm -f $(TARGET)
	@rm -rf $(OBJDIR)


$(OBJDIR)/%.o: %.cpp $(wildcard *.h)
	@mkdir -p $(OBJDIR)
	@echo "CXX $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(patsubst %.cpp,$(OBJDIR)/%.o,$(wildcard *.cpp))
	@echo "LD -o $@"
	@$(CXX) -o $@ $^

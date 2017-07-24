CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++14 -g -O3 -fwrapv
TARGET = competition

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

$(TARGET): %: $(OBJDIR)/%.o
	@echo "LD -o $@"
	@$(CXX) -o $@ $^

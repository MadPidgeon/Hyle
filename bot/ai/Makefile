CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -g -O2 -fwrapv
TARGETS = mc mm
LIBFILES = lib.cpp lib.h

OBJDIR = obj

.PHONY: all clean

.SUFFIXES:

all: $(TARGETS)

clean:
	@echo "Cleaning"
	@rm -f $(TARGETS)
	@rm -rf $(OBJDIR)


$(OBJDIR)/%.o: %.cpp $(wildcard *.h)
	@mkdir -p $(OBJDIR)
	@echo "CXX $<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGETS): %: $(OBJDIR)/%.o $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(LIBFILES)))
	@echo "LD -o $@"
	@$(CXX) -o $@ $^

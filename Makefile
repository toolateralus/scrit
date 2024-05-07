# Compiler
CXX := clang++

# Compiler flags
CXXFLAGS := -O3 -std=c++2b -Iinclude

# Linker flags
LDFLAGS := -lraylib

# Directories
SRCDIR := src
OBJDIR := obj
BINDIR := bin

# Source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)

# Object files
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Targets
all: $(BINDIR)/app

$(BINDIR)/app: $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(BINDIR)/app
	@./$(BINDIR)/app
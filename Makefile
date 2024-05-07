# Compiler
CXX := clang++

# Compiler flags
CXXFLAGS := -fPIC -Iinclude
DEBUGFLAGS := -g -std=c++2b
RELEASEFLAGS := -O3 -std=c++2b

# Linker flags
LDFLAGS := -lraylib

# Directories
SRCDIR := src
OBJDIR := obj
BINDIR := bin

# Source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)

# Object files
DEBUGOBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/debug/%.o,$(SRCS))
RELEASEOBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/release/%.o,$(SRCS))

# Targets
all: debug release

debug: $(BINDIR)/debug/app

release: $(BINDIR)/release/app

$(BINDIR)/debug/app: $(DEBUGOBJS)
	@mkdir -p $(BINDIR)/debug
	$(CXX) $(DEBUGFLAGS) $^ -o $@ $(LDFLAGS)

$(BINDIR)/release/app: $(RELEASEOBJS)
	@mkdir -p $(BINDIR)/release
	$(CXX) $(RELEASEFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/debug/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/debug
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/release/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/release
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $< -o $@

run: run_debug

run_debug: $(BINDIR)/debug/app
	@./$(BINDIR)/debug/app test.wire

run_release: $(BINDIR)/release/app
	@./$(BINDIR)/release/app test.wire

clean:
	rm -rf $(OBJDIR) $(BINDIR)/debug $(BINDIR)/release

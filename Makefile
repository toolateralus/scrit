# Compiler
CXX := clang++

# Compiler flags
CXXFLAGS := -fPIC -std=c++2b -Iinclude -Wall -Wextra
DEBUGFLAGS := -g
RELEASEFLAGS := -O3 
TESTFLAGS := -g

# Linker flags
LDFLAGS := # no flags currently
TESTLINKERFLAGS := -lgtest -lgtest_main

# Directories
SRCDIR := src
TESTDIR := test
OBJDIR := obj
BINDIR := bin

# Source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)
TESTSRCS := $(wildcard $(TESTDIR)/*.cpp)

# Object files
DEBUGOBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/debug/%.o,$(SRCS))
RELEASEOBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/release/%.o,$(SRCS))
TESTOBJS := $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/test/%.o,$(TESTSRCS))
NOMAINDEBUGOBJS := $(filter-out $(OBJDIR)/debug/main.o, $(DEBUGOBJS))

all: debug release

test: test-build
	@./$(BINDIR)/test/app
	
debug: $(BINDIR)/debug/app

release: $(BINDIR)/release/app

test-build: $(BINDIR)/test/app

$(BINDIR)/debug/app: $(DEBUGOBJS)
	@mkdir -p $(BINDIR)/debug
	$(CXX) $(DEBUGFLAGS) $^ -o $@ $(LDFLAGS)

$(BINDIR)/release/app: $(RELEASEOBJS)
	@mkdir -p $(BINDIR)/release
	$(CXX) $(RELEASEFLAGS) $^ -o $@ $(LDFLAGS)

$(BINDIR)/test/app: $(NOMAINDEBUGOBJS) $(TESTOBJS)
	@mkdir -p $(BINDIR)/test
	$(CXX) $(TESTFLAGS) $^ -o $@ $(LDFLAGS) $(TESTLINKERFLAGS)

$(OBJDIR)/debug/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/debug
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/release/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)/release
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $< -o $@

$(OBJDIR)/test/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(OBJDIR)/test
	$(CXX) $(CXXFLAGS) $(TESTFLAGS) -c $< -o $@

run_debug: $(BINDIR)/debug/app
	@./$(BINDIR)/debug/app $(filter-out $@,$(MAKECMDGOALS))

run_release: $(BINDIR)/release/app
	@./$(BINDIR)/release/app $(filter-out $@,$(MAKECMDGOALS))

run:
	$(MAKE) run_debug $(filter-out $@,$(MAKECMDGOALS))
	
clean:
	rm -rf $(OBJDIR) $(BINDIR)

COMPILER:=clang++
COMPILER_FLAGS:= -g -std=c++23
LINKER_FLAGS:= -lscrit -lraylib -lm

MODULE_NAME:=raylib

all: $(MODULE_NAME).dll

$(MODULE_NAME).dll: $(MODULE_NAME).cpp
	sudo $(COMPILER) -shared -fPIC -o /usr/local/scrit/modules/$(MODULE_NAME).dll $< $(COMPILER_FLAGS) $(LINKER_FLAGS)

clean:
	sudo rm -f /usr/local/scrit/modules/$(MODULE_NAME).dll

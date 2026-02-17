.PHONY: challenge dist clean

CXXFLAGS=-std=c++23 -ggdb -O2 -I/usr/include/hyprutils

challenge: main.cpp
	${CXX} ${CXXFLAGS} -o $@ $^ -lhyprutils -Wl,-dynamic-linker=./lib/ld-linux-x86-64.so.2 -Wl,-rpath=./lib

lsp:
	make -C . clean
	bear -- make -C . challenge

clean:
	${RM} challenge compile_commands.json

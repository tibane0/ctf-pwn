.PHONY: challenge dist clean

ZIG=zig
ZIGFLAGS=-Doptimize=ReleaseFast

challenge: src/*.zig build.zig build.zig.zon
	${ZIG} build ${ZIGFLAGS}

lsp:
	make -C . clean
	bear -- make -C . challenge

clean:
	${RM} challenge compile_commands.json

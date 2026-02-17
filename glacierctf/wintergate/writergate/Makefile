.PHONY: challenge dist clean

ZIG=zig

challenge: main.zig
	${ZIG} build-exe -O ReleaseFast -lc --name challenge main.zig

lsp:
	make -C . clean
	bear -- make -C . challenge

clean:
	${RM} challenge compile_commands.json

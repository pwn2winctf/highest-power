CC = arm-linux-gnueabi-gcc
CFLAGS = -Os -Wall
STRIP = arm-linux-gnueabi-strip
QEMU = qemu-arm
QFLAGS = -L /usr/arm-linux-gnueabi
LUA = luajit

highest-power: main.dynasm.o
	$(CC) -o $@ $^
	$(STRIP) $@

main.dynasm.c: main.c bytecode.h opcodes.h
	$(LUA) luajit-2.0/dynasm/dynasm.lua $< > $@ || rm $@

bytecode.h: src.4th src2bytecode.py datatable.py
	./src2bytecode.py < $< > $@ || rm $@

clean:
	rm -f *.dynasm.c *.o bytecode.h highest-power

run: highest-power
	$(QEMU) $(QFLAGS) $<

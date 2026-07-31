all: r8 checker.rom

r8: r8.c fake6502.o
	cc -I./raylib-6.0_linux_amd64/include -Wall -Wextra -ggdb -o r8 r8.c fake6502.o -L./raylib-6.0_linux_amd64/lib/ -l:libraylib.a -lm -lX11

fake6502.o: fake6502.c
	cc -Wall -Wextra -ggdb -c fake6502.c

checker.rom: checker.asm
	./vasm6502_oldstyle/linux/vasm6502_oldstyle checker.asm -Fbin -o checker.rom

#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-c");
    cmd_append(&cmd, "fake6502.c");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "./vasm6502_oldstyle/linux/vasm6502_oldstyle");
    cmd_append(&cmd, "checker.asm");
    cmd_append(&cmd, "-Fbin");
    cmd_append(&cmd, "-o", "checker.rom");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-I./raylib-6.0_linux_amd64/include");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", "r8");
    cmd_append(&cmd, "r8.c");
    cmd_append(&cmd, "fake6502.o");
    cmd_append(&cmd, "-L./raylib-6.0_linux_amd64/lib/");
    cmd_append(&cmd, "-l:libraylib.a");
    cmd_append(&cmd, "-lm");
    cmd_append(&cmd, "-lX11");
    if (!cmd_run(&cmd)) return 1;

    return 0;
}

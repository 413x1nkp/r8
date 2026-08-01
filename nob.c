#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define EXAMPLES_FOLDER "examples/"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    const char *program_name = shift(argv, argc);

    bool run = false;

    while (argc > 0) {
        const char *flag = shift(argv, argc);
        if (strcmp(flag, "-run") == 0) {
            run = true;
        } else {
            fprintf(stderr, "ERROR: unknown flag \"%s\"\n", flag);
            return 1;
        }
    }

    if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;
    if (!mkdir_if_not_exists(BUILD_FOLDER EXAMPLES_FOLDER)) return 1;

    Cmd cmd = {0};
    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-c");
    cmd_append(&cmd, SRC_FOLDER"fake6502.c");
    cmd_append(&cmd, "-o", BUILD_FOLDER"fake6502.o");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "./vasm6502_oldstyle/linux/vasm6502_oldstyle");
    cmd_append(&cmd, EXAMPLES_FOLDER"checker.asm");
    cmd_append(&cmd, "-Fbin");
    cmd_append(&cmd, "-o", BUILD_FOLDER EXAMPLES_FOLDER"checker.rom");
    if (!cmd_run(&cmd)) return 1;

    cmd_append(&cmd, "cc");
    cmd_append(&cmd, "-I./raylib-6.0_linux_amd64/include");
    cmd_append(&cmd, "-I.");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", BUILD_FOLDER"r8");
    cmd_append(&cmd, SRC_FOLDER"r8.c");
    cmd_append(&cmd, BUILD_FOLDER"fake6502.o");
    cmd_append(&cmd, "-L./raylib-6.0_linux_amd64/lib/");
    cmd_append(&cmd, "-l:libraylib.a");
    cmd_append(&cmd, "-lm");
    cmd_append(&cmd, "-lX11");
    if (!cmd_run(&cmd)) return 1;

    if (run) {
        cmd_append(&cmd, BUILD_FOLDER"r8", BUILD_FOLDER EXAMPLES_FOLDER "checker.rom");
        if (!cmd_run(&cmd)) return 1;
    }

    return 0;
}

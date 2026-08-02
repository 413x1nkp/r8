#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define EXAMPLES_FOLDER "examples/"
#define TARGET_NAME "linux_amd64"
#define RAYLIB_SRC_FOLDER "raylib-6.0/src/"

bool build_raylib(void);

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

    if (!build_raylib()) return 1;

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
    cmd_append(&cmd, "-I./raylib-6.0/src/");
    cmd_append(&cmd, "-I.");
    cmd_append(&cmd, "-Wall");
    cmd_append(&cmd, "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-o", BUILD_FOLDER"r8");
    cmd_append(&cmd, SRC_FOLDER"r8.c");
    cmd_append(&cmd, BUILD_FOLDER"fake6502.o");
    cmd_append(&cmd, "-L./"BUILD_FOLDER"raylib_"TARGET_NAME"/");
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

static const char *raylib_modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
};

bool build_raylib(void)
{
    bool result = true;
    Cmd cmd = {0};
    File_Paths object_files = {0};

    Procs procs = {0};

    const char *build_path = BUILD_FOLDER "raylib_" TARGET_NAME;

    if (!mkdir_if_not_exists(build_path)) {
        return_defer(false);
    }

    for (size_t i = 0; i < ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = temp_sprintf(RAYLIB_SRC_FOLDER"%s.c", raylib_modules[i]);
        const char *output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
        output_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);

        da_append(&object_files, output_path);

        if (needs_rebuild(output_path, &input_path, 1)) {
            cmd_append(&cmd, "cc",
                "-ggdb", "-DPLATFORM_DESKTOP", "-D_GLFW_X11", "-fPIC", "-DSUPPORT_FILEFORMAT_FLAC=1",
                "-I"RAYLIB_SRC_FOLDER"external/glfw/include",
                "-c", input_path,
                "-o", output_path);
            if (!cmd_run(&cmd, .async = &procs)) return_defer(false);
        }
    }

    if (!procs_flush(&procs)) return_defer(false);

    const char *libraylib_path = temp_sprintf("%s/libraylib.a", build_path);
    delete_file(libraylib_path);
    cmd_append(&cmd, "ar", "-crs", libraylib_path);
    for (size_t i = 0; i < ARRAY_LEN(raylib_modules); ++i) {
        const char *input_path = temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
        cmd_append(&cmd, input_path);
    }
    if (!cmd_run(&cmd)) return_defer(false);

defer:
    cmd_free(cmd);
    da_free(object_files);
    return result;
}

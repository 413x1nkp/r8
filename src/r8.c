#include <stdio.h>
#include <stdint.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "raylib.h"
#include "raymath.h"
#include "no_rom.c"

#define UI_FPS 60 // The FPS the UI is running at
#define EMU_FPS 5 // The FPS the 6502 emulator is running at
#define EMU_DELTA_TIME (1.0/EMU_FPS)
#define VECTOR_EXIT   0x6969
#define ENTRY_POINT   0x8000
#define UPDATE_VECTOR 0xFFFE
#define CANVAS        0x1000
#define CANVAS_WIDTH  64
#define CANVAS_HEIGHT 64
#define KEYBOARD      (CANVAS + CANVAS_WIDTH*CANVAS_HEIGHT)
#define MAX_VECTOR_STEPS (100*1000)

// 0x1000 .. 0x2000

static uint8_t MEMORY[1<<16];

// Forward declarations for definitions from fake6502.c
void reset6502();
void step6502();
void rts();
void irq6502();
void push16(uint16_t pushval);
extern uint16_t pc;
extern uint8_t sp, a, x, y, status;
#define FLAG_INTERRUPT 0x04

uint8_t read6502(uint16_t address)
{
    return MEMORY[address];
}

void write6502(uint16_t address, uint8_t value)
{
    MEMORY[address] = value;
}

uint16_t read16(uint16_t address)
{
    return (uint16_t)read6502(address) | ((uint16_t)read6502(address + 1) << 8);
}

void load_rom_at(uint8_t *rom_bytes, size_t rom_count, uint16_t offset)
{
    for (size_t i = 0; i < rom_count; ++i) {
        uint8_t x = rom_bytes[i];
        assert(i + offset < sizeof(MEMORY));
        MEMORY[i + offset] = x;
    }
}

bool call_vector(uint16_t vector_address)
{
    push16(VECTOR_EXIT);
    pc = vector_address;
    for (size_t i = 0; pc != VECTOR_EXIT + 1 && i < MAX_VECTOR_STEPS; ++i) {
        step6502();
    }
    if (pc != VECTOR_EXIT + 1) {
        nob_log(ERROR, "Vector at $%04X took more than %d iterations to execute so we cancelled it", vector_address, MAX_VECTOR_STEPS);
        return false;
    }
    return true;
}

bool reload_rom(String_Builder *rom, const char *rom_path)
{
    memset(MEMORY, 0, sizeof(MEMORY));

    if (rom_path == NULL) {
        load_rom_at(no_rom, ARRAY_LEN(no_rom), ENTRY_POINT);
        reset6502();
        return true;
    }

    rom->count = 0;
    if (!read_entire_file(rom_path, rom)) return false;
    load_rom_at((uint8_t*)rom->items, rom->count, ENTRY_POINT);
    reset6502();

    return true;
}

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);
    UNUSED(program_name);

    char *rom_path = NULL;
    String_Builder rom = {0};

    if (argc > 0) {
        rom_path = strdup(shift(argv, argc));
    }

    reload_rom(&rom, rom_path);
    call_vector(ENTRY_POINT);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "r8");
    SetTargetFPS(UI_FPS);


    Texture canvas = LoadTextureFromImage((Image) {
        .data    = MEMORY + CANVAS,
        .width   = CANVAS_WIDTH,
        .height  = CANVAS_HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    });
    SetTextureWrap(canvas, TEXTURE_WRAP_CLAMP);

    float global_timer = 0;
    bool pause = false;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            pause = !pause;
        }

        if (IsKeyPressed(KEY_R) && IsKeyDown(KEY_LEFT_CONTROL)) {
            reload_rom(&rom, rom_path);
            call_vector(ENTRY_POINT);
        }

        if (IsFileDropped()) {
            FilePathList files = LoadDroppedFiles();
            assert(files.count > 0);

            free(rom_path);
            rom_path = strdup(files.paths[files.count-1]);
            reload_rom(&rom, rom_path);
            call_vector(ENTRY_POINT);

            UnloadDroppedFiles(files);
        }

        if (!pause) {
            float a = fmodf(global_timer, EMU_DELTA_TIME);
            global_timer += GetFrameTime();
            float b = fmodf(global_timer, EMU_DELTA_TIME);
            if (b < a) {
                for (int c = 0; c < 128; ++c) {
                    MEMORY[KEYBOARD + c] = IsKeyDown(c);
                }

                call_vector(read16(UPDATE_VECTOR));
            }
        }

        UpdateTexture(canvas, MEMORY + CANVAS);

        float w  = GetScreenWidth();
        float h  = GetScreenHeight();

        Camera2D camera = {0};
        camera.offset = (Vector2){w*0.5, h*0.5};
        if (w < h) {
            camera.zoom = w/canvas.width;
        } else {
            camera.zoom = h/canvas.height;
        }

        BeginDrawing(); {
            ClearBackground(GetColor(0x181818FF));
            BeginMode2D(camera); {
                Vector2 position = {-canvas.width*0.5, -canvas.height*0.5};
                DrawTextureEx(canvas, position, 0, 1, WHITE);
            } EndMode2D();
        } EndDrawing();
    }

    CloseWindow();

    return 0;
}

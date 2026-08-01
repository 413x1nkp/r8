#include <stdio.h>
#include <stdint.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "raylib.h"
#include "raymath.h"

#define ENTRY_POINT   0x8000
#define CANVAS        0x1000
#define CANVAS_WIDTH  64
#define CANVAS_HEIGHT 64

static uint8_t MEMORY[1<<16];

// Forward declarations for definitions from fake6502.c
void reset6502();
void step6502();
void rts();
void irq6502();
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

void load_rom_at(String_Builder rom, uint16_t offset)
{
    for (size_t i = 0; i < rom.count; ++i) {
        uint8_t x = rom.items[i];
        assert(i + offset < sizeof(MEMORY));
        MEMORY[i + offset] = x;
    }
}

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <rom>\n", program_name);
        fprintf(stderr, "ERROR: no rom is provided\n");
        return 1;
    }

    const char *rom_path = shift(argv, argc);

    InitWindow(800, 600, "r8");
    SetTargetFPS(5);

    String_Builder rom = {0};
    if (!read_entire_file(rom_path, &rom)) return 1;

    load_rom_at(rom, ENTRY_POINT);

    // Set the init routine
    MEMORY[0xFFFC] = (ENTRY_POINT>>0)&0xFF;
    MEMORY[0xFFFD] = (ENTRY_POINT>>8)&0xFF;

    // Set the update routine
    reset6502();
    while (pc != 0x6969) {
        step6502();
    }

    Texture canvas = LoadTextureFromImage((Image) {
        .data    = MEMORY + CANVAS,
        .width   = CANVAS_WIDTH,
        .height  = CANVAS_HEIGHT,
        .mipmaps = 1,
        .format  = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    });

    while (!WindowShouldClose()) {
        irq6502();
        while (status&FLAG_INTERRUPT) {
            step6502();
        }

        UpdateTexture(canvas, MEMORY + CANVAS);

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawTextureEx(canvas, Vector2Zero(), 0, 10, WHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

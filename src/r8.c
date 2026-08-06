#include <stdio.h>
#include <stdint.h>
#define NOB_IMPLEMENTATION
#include "nob.h"

#include "raylib.h"
#include "raymath.h"
#include "no_rom.c"

#define UI_FPS 60 // The FPS the UI is running at
#define DEFAULT_EMU_FPS 5 // Default FPS the 6502 emulator is running at
#include "layout.h"
#define MAX_VECTOR_STEPS (10*1000*1000)
#define BACKGROUND_COLOR 0x181818FF

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
    } else {
        rom->count = 0;
        if (!read_entire_file(rom_path, rom)) return false;
        load_rom_at((uint8_t*)rom->items, rom->count, ENTRY_POINT);
    }

    reset6502();
    MEMORY[FPS_CONFIG] = DEFAULT_EMU_FPS;

    return true;
}

void box_pad(Rectangle *a, float pad);
void box_merge(Rectangle *a, Rectangle b);
void play_icon(Rectangle button_box, Color color);
void pause_icon(Rectangle button_box, Color color);
void reset_icon(Rectangle button_box, Color color, Color background_color);
bool button(Rectangle button_box, Camera2D camera, Color color);

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
    if (!IsWindowReady()) {
        TraceLog(LOG_ERROR, "Could not initialize the Window. See error messages above.");
        return 1;
    }
    SetTargetFPS(UI_FPS);
    SetExitKey(KEY_NULL);


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
    Rectangle ui_box = {0};
    while (!WindowShouldClose()) {
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
            int emu_fps = MEMORY[FPS_CONFIG] & 31;
            if (emu_fps == 0) { emu_fps = DEFAULT_EMU_FPS; }
            float emu_delta_time = 1.f / (float) emu_fps;
            float a = fmodf(global_timer, emu_delta_time);
            global_timer += GetFrameTime();
            float b = fmodf(global_timer, emu_delta_time);
            if (b < a) {
                for (int c = 0; c < 128; ++c) {
                    MEMORY[KEYBOARD + c] = IsKeyDown(c);
                }
                MEMORY[KEYBOARD + '\b'] = IsKeyDown(KEY_BACKSPACE);
                MEMORY[KEYBOARD + '\n'] = IsKeyDown(KEY_ENTER);
                MEMORY[KEYBOARD + 0x1B] = IsKeyDown(KEY_ESCAPE);

                call_vector(read16(UPDATE_VECTOR));
            }
        }

        UpdateTexture(canvas, MEMORY + CANVAS);

        float w  = GetScreenWidth();
        float h  = GetScreenHeight();

        Camera2D camera = {0};
        camera.offset = (Vector2){w*0.5, h*0.5};
        camera.target = (Vector2){
            ui_box.x + ui_box.width*0.5,
            ui_box.y + ui_box.height*0.5,
        };
        if (w/h < ui_box.width/ui_box.height) {
            camera.zoom = w/ui_box.width;
        } else {
            camera.zoom = h/ui_box.height;
        }

        BeginDrawing(); {
            ClearBackground(GetColor(BACKGROUND_COLOR));
            BeginMode2D(camera); {
                memset(&ui_box, 0, sizeof(ui_box));

                DrawTextureEx(canvas, Vector2Zero(), 0, 1, WHITE);
                Rectangle canvas_box = {0, 0, canvas.width, canvas.height};
                box_merge(&ui_box, canvas_box);

                float margin_between_canvas_and_buttons = 0.03*canvas.width;
                Rectangle button_box = {0};
                button_box.x      = canvas_box.x + canvas.width + margin_between_canvas_and_buttons;
                button_box.y      = canvas_box.y + margin_between_canvas_and_buttons;
                button_box.width  = canvas_box.width*0.05;
                button_box.height = button_box.width;

                Color icon_color   = GetColor(BACKGROUND_COLOR);
                Color button_color = WHITE;

                // Toggle Pause Button
                if (button(button_box, camera, button_color)) pause = !pause;
                if (pause) play_icon(button_box, icon_color); else pause_icon(button_box, icon_color);
                box_merge(&ui_box, button_box);

                // Reset Button
                button_box.y += button_box.height + margin_between_canvas_and_buttons;
                if (button(button_box, camera, button_color)) {
                    reload_rom(&rom, rom_path);
                    call_vector(ENTRY_POINT);
                }
                box_merge(&ui_box, button_box);
                reset_icon(button_box, icon_color, WHITE);

                box_pad(&ui_box, 2);
            } EndMode2D();
        } EndDrawing();
    }

    CloseWindow();

    return 0;
}

void box_merge(Rectangle *a, Rectangle b)
{
    if (b.x < a->x) {
        a->x = b.x;
    }
    if (b.y < a->y) {
        a->y = b.y;
    }
    if (a->x + a->width < b.x + b.width) {
        a->width = b.x + b.width;
    }
    if (a->y + a->height < b.y + b.height) {
        a->height = b.y + b.height;
    }
}

void box_pad(Rectangle *a, float pad)
{
    a->x      -= pad;
    a->y      -= pad;
    a->width  += pad*2;
    a->height += pad*2;
}

void play_icon(Rectangle button_box, Color color)
{
    float icon_padding_left   = 0.3*button_box.width;
    float icon_padding_right  = 0.2*button_box.width;
    float icon_padding_top    = 0.2*button_box.width;
    float icon_padding_bottom = 0.2*button_box.width;
    Vector2 v1 = {button_box.x + icon_padding_left, button_box.y + icon_padding_top};
    Vector2 v2 = {button_box.x + icon_padding_left, button_box.y + button_box.height - icon_padding_bottom};
    Vector2 v3 = {button_box.x + button_box.width - icon_padding_right, button_box.y + button_box.height*0.5};
    DrawTriangle(v1, v2, v3, color);
}

void pause_icon(Rectangle button_box, Color color)
{
    float icon_padding = 0.2*button_box.width;
    Rectangle pause_icon_box = {
        .x      = button_box.x + icon_padding,
        .y      = button_box.y + icon_padding,
        .width  = button_box.width - icon_padding - icon_padding,
        .height = button_box.height - icon_padding - icon_padding,
    };
    DrawRectangleRec((Rectangle) {
        .x      = pause_icon_box.x,
        .y      = pause_icon_box.y,
        .width  = pause_icon_box.width/3.,
        .height = pause_icon_box.height,
    }, color);
    DrawRectangleRec((Rectangle) {
        .x      = pause_icon_box.x + pause_icon_box.width*2./3.,
        .y      = pause_icon_box.y,
        .width  = pause_icon_box.width/3.,
        .height = pause_icon_box.height,
    }, color);
}

void reset_icon(Rectangle button_box, Color color, Color background_color)
{
    float radius = 0.35*fminf(button_box.width, button_box.height);

    Vector2 center = {
        .x = button_box.x + button_box.width*0.5,
        .y = button_box.y + button_box.height*0.5,
    };

    float gap_angle = 50;
    DrawCircleSector(center, radius, gap_angle*0.5, 360 - gap_angle*0.5, 20, color);
    DrawCircleV(center, radius*0.5, background_color);

    // Arrow
    {
        Vector2 v = {1, 0};
        v = Vector2Rotate(v, -0.5*gap_angle*DEG2RAD);

        Vector2 c = center;
        c = Vector2Add(c, Vector2Scale(v, radius*0.75));

        float size = 0.65*radius;
        v = Vector2Rotate(v, 85*DEG2RAD);
        Vector2 v1 = Vector2Add(c, Vector2Scale(v, size));
        Vector2 v2 = Vector2Add(c, Vector2Scale(Vector2Rotate(v, -120*DEG2RAD), size));
        Vector2 v3 = Vector2Add(c, Vector2Scale(Vector2Rotate(v, 120*DEG2RAD), size));
        DrawTriangle(v1, v2, v3, color);
    }
}

bool button(Rectangle button_box, Camera2D camera, Color color)
{
    DrawRectangleRec(button_box, color);
    bool hover = CheckCollisionPointRec(GetScreenToWorld2D(GetMousePosition(), camera), button_box);
    bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    return (hover && click);
}

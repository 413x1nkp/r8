#define VECTOR_EXIT   0x6969
#define ENTRY_POINT   0x8000
#define FPS_CONFIG    0xFFFD // Lower 5 bits set the FPS the 6502 emulator is running at. The upper high bits are ignored for now.
#define UPDATE_VECTOR 0xFFFE
#define CANVAS        0x1000
#define CANVAS_WIDTH  64
#define CANVAS_HEIGHT 64
#define KEYBOARD      (CANVAS + CANVAS_WIDTH*CANVAS_HEIGHT)

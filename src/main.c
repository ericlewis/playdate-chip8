#ifdef _WINDLL
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#include <stdint.h>
#include <stdlib.h>

#include "pd_api.h"
#include "inc/chip8.h"
#include "inc/peripherals.h"

// Playdate
PlaydateAPI *pd;
int updateCallback(void* userdata);
int updateWrapper(lua_State* L);
int initialize(char *rom_filename);
int initializeWrapper();

LCDFont* font;
PDSynth* synth;
int initialized = 0;
int selectedIdx = 0;
static char** key_pad[] = {
        "1", "2", "3",
        "4", "5", "6",
        "7", "8", "9",
        "0", "A", "B",
        "C", "D", "E",
        "F"};

DllExport int
eventHandler(PlaydateAPI *playdate, PDSystemEvent event, uint32_t arg)
{
    if (event == kEventInit) {
        pd = playdate;

        pd->display->setRefreshRate(0);
        pd->system->logToConsole("[PENDING] Initializing Playdate Resources...");
        synth = pd->sound->synth->newSynth();
        font = pd->graphics->loadFont("Asheville-Sans-24-Light", NULL);
        pd->graphics->setFont(font);
        pd->system->logToConsole("[OK] Playdate Resources loaded!");
    }
    if (event == kEventInitLua) {
        pd->lua->addFunction(initializeWrapper, "loadRom", NULL);
        pd->lua->addFunction(updateWrapper, "emulate", NULL);
    }
    return 0;
}

int initializeWrapper() {
    char *rom = pd->lua->getArgString(1);
    initialize(rom);
    return 0;
}

int initialize(char *rom_filename) {
    pd->system->logToConsole("[PENDING] Initializing CHIP-8 arch...");
    init_cpu();
    
    pd->system->logToConsole("[OK] Done!");
    pd->system->logToConsole("[PENDING] Loading rom %s...", rom_filename);

    int status = load_rom(rom_filename);

    if (status == -1) {
        pd->system->logToConsole("[FAILED] read: the return value was not equal to the rom file size.");
        return 1;
    } else if (status != 0) {
        pd->system->error("Error while loading rom");
        return 1;
    }

    pd->system->logToConsole("[OK] Rom loaded successfully!");

    pd->graphics->clear(kColorBlack);
    pd->system->logToConsole("[OK] Display setup successfully!");
    initialized = 1;
    
    return 0;
}

int updateWrapper(lua_State* L) {
    updateCallback(NULL);
    return 0;
}

int updateCallback(void* userdata)
{
    PDButtons buttons;
    pd->system->getButtonState(NULL, NULL, &buttons);

    if (sound_flag) {
        pd->sound->synth->playMIDINote(synth, 60, 1, -1, 0);
    } else {
        pd->sound->synth->stop(synth);
    }

    pd->graphics->pushContext(NULL);
    int height = pd->graphics->getFontHeight(font);
    int width = 27;
    for(int i = 0;i<15;i++) {
        int isSelected = selectedIdx == i;
        pd->graphics->fillRect(i * width, 240-height, width, height, isSelected ? kColorWhite : kColorBlack);
        pd->graphics->setDrawMode(isSelected ? kDrawModeFillBlack : kDrawModeFillWhite);
        pd->graphics->drawText(key_pad[i], 1, kUTF8Encoding, (i * width) + 6, (240-height) + 3);
    }
    pd->graphics->popContext();

    if (initialized) {
        if (buttons & kButtonA) {
            for (int keycode = 0; keycode < 16; keycode++) {
                keypad[keycode] = 0;
            }
            keypad[selectedIdx] = 1;
        }
        if (buttons & kButtonB) {
            for (int keycode = 0; keycode < 16; keycode++) {
                keypad[keycode] = 0;
            }
        }
        if (buttons & kButtonLeft) {
            if (selectedIdx - 1 < 0) {
                selectedIdx = 14;
            } else {
                selectedIdx = max(selectedIdx - 1, 0);
            }
        }
        if (buttons & kButtonRight) {
            if (selectedIdx + 1 > 14) {
                selectedIdx = 0;
            } else {
                selectedIdx = min(selectedIdx + 1, 14);
            }
        }

        emulate_cycle();

        if (draw_flag) {
            draw(display);
        }
    }

    return 1;
}
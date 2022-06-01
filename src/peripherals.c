#include "inc/peripherals.h"
#include "pd_api.h"

static const float scale = 6.25;
void draw(unsigned char* display) {
    LCDBitmap *frame = pd->graphics->newBitmap(64, 32, kColorWhite);
    pd->graphics->pushContext(frame);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display[x + (y * 64)]) {
                pd->graphics->fillRect(x, y, 1, 1, kColorWhite);
            } else {
                pd->graphics->fillRect(x, y, 1, 1, kColorBlack);
            }
        }
    }
    pd->graphics->popContext();
    pd->graphics->drawScaledBitmap(frame, 0, 0, scale, scale);
    pd->graphics->freeBitmap(frame);
}
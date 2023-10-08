#include <stdint.h>
#include "llapi.h"
#include "keyboard.h"

#define kbd_MakeEvent(event, value) ((value << 16) | event)

uint32_t kbd_query_event(void) {
    uint8_t key_event = llapi_query_key() & 0xFF;
    uint8_t pressed = ((KEY_EVENT_STATE_BIT & key_event) == 0);
    uint8_t code = key_event & 0x7F;
    if (key_event != 0xFF) {
        return kbd_MakeEvent(pressed + 1, code); 
    }
    return kbd_MakeEvent(KACT_NOP, 0);
}

void kbd_discard(void) {
    uint8_t key_event = llapi_query_key() & 0xFF;
    while (key_event != 0xFF) {
        key_event = llapi_query_key() & 0xFF;
    }
}

#include <stdint.h>
#include <vgp_impl_0002.h>
#include <keyboard.h>
#include "mvgp_impl_0002.h"
#include "llapi.h"

static uint8_t key_status = 0;

int32_t vgp_gamepad_status(void) {
    return (int32_t) key_status;
}

void update_key_status(void) {
    uint8_t key_event = llapi_query_key() & 0xFF;
    uint8_t pressed = ((KEY_EVENT_STATE_BIT & key_event) == 0);
    uint8_t code = key_event & 0x7F;
    if (pressed) {
        switch (code) {
            case KEY_UP:
                key_status = key_status | KEY_MASK_UP;
                break;
            case KEY_DOWN:
                key_status = key_status | KEY_MASK_DOWN;
                break;
            case KEY_LEFT:
                key_status = key_status | KEY_MASK_LEFT;
                break;
            case KEY_RIGHT:
                key_status = key_status | KEY_MASK_RIGHT;
                break;
            case KEY_F2:
            case KEY_F4:
            case KEY_F6:
            case KEY_ENTER:
                key_status = key_status | KEY_MASK_A;
                break;
            case KEY_F1:
            case KEY_F3:
            case KEY_F5:
            case KEY_ON:
                key_status = key_status | KEY_MASK_B;
                break;
            default:
                break;
        }
    } else {
        switch (code) {
            case KEY_UP:
                key_status = key_status & ~(KEY_MASK_UP);
                break;
            case KEY_DOWN:
                key_status = key_status & ~(KEY_MASK_DOWN);
                break;
            case KEY_LEFT:
                key_status = key_status & ~(KEY_MASK_LEFT);
                break;
            case KEY_RIGHT:
                key_status = key_status & ~(KEY_MASK_RIGHT);
                break;
            case KEY_F2:
            case KEY_F4:
            case KEY_F6:
            case KEY_ENTER:
                key_status = key_status & ~(KEY_MASK_A);
                break;
            case KEY_F1:
            case KEY_F3:
            case KEY_F5:
            case KEY_ON:
                key_status = key_status & ~(KEY_MASK_B);
                break;
            default:
                break;
        }
    }
}

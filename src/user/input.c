#include "input.h"
#include <core/events/input_event/input_event.h>
#include <core/input/key.h>
#include <core/input/mouse.h>

static InputEventCallbackHandler g_callbackHandler;

// Maybe be better to use bit array
static boolean g_keysState[KEY_COUNT] = {false};
static boolean g_mouseButtonsState[MOUSE_BUTTON_COUNT] = {false};

static void input_event_callback(const InputEvent* const event) {
    switch (event->type) {
        case INPUT_EVENT_TYPE_KEY:
            g_keysState[event->data.key.key] = event->data.key.is_pressed;
            break;
        case INPUT_EVENT_TYPE_MOUSE_BUTTON:
            g_mouseButtonsState[event->data.mouse_button.button] = event->data.mouse_button.is_pressed;
            break;
    }
}


void input_init(void) {
    g_callbackHandler = input_event_connect(input_event_callback);
}

void input_exit(void) {
    input_event_disconnect(g_callbackHandler);
}

boolean input_key_is_pressed(Key key) {
    return g_keysState[key];
}

boolean input_mouse_button_is_pressed(MouseButton btn) {
    return g_mouseButtonsState[btn];
}

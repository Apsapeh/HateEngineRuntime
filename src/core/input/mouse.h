#ifndef H_INPUT_MOUSE
#define H_INPUT_MOUSE

#include <core/types/types.h>

// SDL3 based values
// https://wiki.libsdl.org/SDL3/SDL_MouseButtonFlags

/*
API ENUM {
        "name": "MouseButton",
        "type": "u8",
        "values": [
                ["Left", 1],
                ["Middle", 2],
                ["Right", 3],
                ["X1", 4],
                ["X2", 5]
        ]
}
*/


// clang-format off
#define MOUSE_BUTTON_LEFT   1
#define MOUSE_BUTTON_MIDDLE  2
#define MOUSE_BUTTON_RIGHT  3
#define MOUSE_BUTTON_X1  4
#define MOUSE_BUTTON_X2  5
#define MOUSE_BUTTON_UNKNOWN 255
#define MOUSE_BUTTON_FIRST MOUSE_BUTTON_LEFT
#define MOUSE_BUTTON_LAST  MOUSE_BUTTON_X2

#define MOUSE_BUTTON_COUNT 6
// clang-format on


/**
 * 1 - Left mouse button
 *
 * 2 - Middle mouse button
 *
 * 3 - Right mouse button
 *
 * 4 - X1 mouse button. I don't give a fuck what is it
 *
 * 5 - X2 mouse button. I don't give a fuck what is it
 *
 * @api
 */
typedef u8 MouseButton;


#endif // !H_INPUT_MOUSE

#ifndef H_INPUT_MOUSE
#define H_INPUT_MOUSE

#include <core/types/types.h>

// SDL3 based values
// https://wiki.libsdl.org/SDL3/SDL_MouseButtonFlags

/*
u8 MouseButton

Left = 1
Middle
Right
X1
X2
Count
Unknown = 255
 */


/**
 * @api
 */
enum {
    MOUSE_BUTTON_LEFT = 1,
    MOUSE_BUTTON_MIDDLE = 2,
    MOUSE_BUTTON_RIGHT = 3,
    MOUSE_BUTTON_X1 = 4,
    MOUSE_BUTTON_X2 = 5,
    MOUSE_BUTTON_COUNT = 6,
    MOUSE_BUTTON_UNKNOWN = 255,
};

/**
 * 1 - Left

 * 2 - Middle

 * 3 - Right

 * 4 - X1

 * 5 - X2

 * 6 - Count

 * 255 - Unknown

 * @api
 */
typedef u8 MouseButton;


#endif // !H_INPUT_MOUSE

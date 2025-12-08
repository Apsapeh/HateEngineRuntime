#ifndef H_INPUT_EVENT_GUARD
#define H_INPUT_EVENT_GUARD

#include <types/types.h>
#include <input/mouse.h>
#include <input/key.h>
#include "math/vec2.h"


/*
API ENUM {
        "name": "InputEventType",
        "type": "u8",
        "values": [
                ["Key", 1],
                ["MouseButton", 2],
                ["MouseMotion", 3]
        ]
}
*/

// clang-format off
#define INPUT_EVENT_TYPE_KEY   0
#define INPUT_EVENT_TYPE_MOUSE_BUTTON  1
#define INPUT_EVENT_TYPE_MOUSE_MOTION  2
#define INPUT_EVENT_TYPE_UNKNOWN 255
#define INPUT_EVENT_TYPE_FIRST INPUT_EVENT_TYPE_KEY
#define INPUT_EVENT_TYPE_LAST  INPUT_EVENT_TYPE_MOUSE_MOTION
// clang-format on


/**
 * 0 - Keyboard key
 *
 * 1 - Mouse button
 *
 * 2 - Mouse motion
 *
 * @api
 */
typedef u8 InputEventType;

/**
 * @api
 */
typedef struct {
    InputEventType type;

    union {
        struct {
            Key key;
            boolean is_pressed;
            boolean is_repeat;
            // Key virtual_key; // I don't give a fuck for what this shit. In 99.999% cases, you need a
            // physical keys, not their layout represtation
        } key;

        struct {
            MouseButton button;
            boolean is_pressed;
            u8 clicks;
            Vec2 coords;
        } mouse_button;

        struct {
            Vec2 offset;
            Vec2 coords;
        } mouse_motion;
    } data;
} InputEvent;


boolean input_event_constructor(InputEvent* self);

boolean input_event_destructor(InputEvent* self);


/**
 * @api
 */
InputEvent* input_event_new(void);

/**
 * @api
 */
boolean input_event_free(InputEvent* event);

/**
 * @api
 */
boolean input_event_set_type(InputEvent* event, InputEventType type);

/**
 * return INPUT_EVENT_TYPE_UNKNOWN (255) on fail
 *
 * @api
 */
InputEventType input_event_get_type(InputEvent* event);

/**
 * @api
 */
boolean input_event_emit(const InputEvent* event);


/* ====================================> key Event <====================================*/

/**
 * @api
 */
boolean input_event_key_set_key(InputEvent* event, Key button);

/**
 * @api
 */
boolean input_event_key_set_is_pressed(InputEvent* event, boolean is_pressed);

/**
 * @api
 */
boolean input_event_key_set_is_repeat(InputEvent* event, boolean is_repeat);

/**
 * @api
 */
boolean input_event_key_get_key(InputEvent* event, Key* button);

/**
 * @api
 */
boolean input_event_key_is_pressed(InputEvent* event, boolean* is_pressed);

/**
 * @api
 */
boolean input_event_key_is_repeat(InputEvent* event, boolean* is_repeat);


/* ====================================> Mouse Button Event <====================================*/

/**
 * @api
 */
boolean input_event_mouse_button_set_button(InputEvent* event, MouseButton button);

/**
 * @api
 */
boolean input_event_mouse_button_set_is_pressed(InputEvent* event, boolean is_pressed);

/**
 * @api
 */
boolean input_event_mouse_button_set_clicks(InputEvent* event, u8 clicks);

/**
 * @api
 */
boolean input_event_mouse_button_set_coords(InputEvent* event, const Vec2* const coords);


/**
 * @api
 */
boolean input_event_mouse_button_get_button(InputEvent* event, MouseButton* button);

/**
 * @api
 */
boolean input_event_mouse_button_is_pressed(InputEvent* event, boolean* is_pressed);

/**
 * @api
 */
boolean input_event_mouse_button_get_clicks(InputEvent* event, u8* clicks);

/**
 * @api
 */
boolean input_event_mouse_button_get_coords(InputEvent* event, Vec2* coords);


/* ====================================> Mouse Button Event <====================================*/
/**
 * @api
 */
boolean input_event_mouse_motion_set_offset(InputEvent* event, const Vec2* const coords);

/**
 * @api
 */
boolean input_event_mouse_motion_set_coords(InputEvent* event, const Vec2* const coords);


/**
 * @api
 */
boolean input_event_mouse_motion_get_offset(InputEvent* event, Vec2* coords);

/**
 * @api
 */
boolean input_event_mouse_motion_get_coords(InputEvent* event, Vec2* coords);


#endif

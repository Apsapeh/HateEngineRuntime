#ifndef H_INPUT_EVENT_GUARD
#define H_INPUT_EVENT_GUARD

#include <types/types.h>


// MACROS API START

// clang-format off
#define INPUT_EVENT_TYPE_KEY   0
#define INPUT_EVENT_TYPE_MOUSE_BUTTON  1
#define INPUT_EVENT_TYPE_MOUSE_MOTION  2
#define INPUT_EVENT_TYPE_UNKNOWN 255
#define INPUT_EVENT_TYPE_FIRST INPUT_EVENT_TYPE_KEY
#define INPUT_EVENT_TYPE_LAST  INPUT_EVENT_TYPE_MOUSE_MOTION
// clang-format on

// MACROS API END

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
typedef struct InputEvent InputEvent;

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


/* ====================================> Mouse Motion Event <====================================*/

/**
 * @api
 */
boolean input_event_key_set_is_pressed(InputEvent* event, boolean is_pressed);


/**
 * @api
 */
boolean input_event_key_is_pressed(InputEvent* event, boolean* success);


#endif

#ifndef H_INPUT_INPUT_GUARD
#define H_INPUT_INPUT_GUARD

/*
 *
 * Sugar for input
 *
 */

#include <core/input/key.h>
#include <core/input/mouse.h>

void input_init(void);
void input_exit(void);

/**
 * @warning May return false, even if key is pressed
 *
 * @api
 */
boolean input_key_is_pressed(Key key);

/**
 * @warning May return false, even if button is pressed
 *
 * @api
 */
boolean input_mouse_button_is_pressed(MouseButton btn);

#endif // !H_INPUT_INPUT_GUARD

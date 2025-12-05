#ifndef H_INPUT_EVENT_GUARD
#define H_INPUT_EVENT_GUARD

#include <types/types.h>

/* ====================================> Mouse Motion Event <====================================*/
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
boolean input_event_set_type(InputEvent* event);

/**
 * @api
 */
boolean input_event_emit(const InputEvent* event);

#endif

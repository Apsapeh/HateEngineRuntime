#include "input_event.h"
#include <platform/memory.h>
#include <error.h>
#include <helpers.h>

struct InputEvent {
    InputEventType type;

    struct {
        boolean is_pressed;
    } key;
};

#ifdef HE_DEBUG
    #define EVENT_TYPE_CHECK(event, event_type, function_name, end_block)                               \
        do {                                                                                            \
            if (event->type != event_type) {                                                            \
                LOG_ERROR_OR_DEBUG_FATAL(                                                               \
                        function_name ": Wrong InputEvent type. Expected " #event_type                  \
                                      "(%d), found %d",                                                 \
                        event_type, event->type                                                         \
                );                                                                                      \
                end_block                                                                               \
            }                                                                                           \
        } while (0)
#else
    #define EVENT_TYPE_CHECK(_, __, ___, ____)
#endif


InputEvent* input_event_new(void) {
    InputEvent* result = tmalloc(sizeof(InputEvent));
    ERROR_ALLOC_CHECK(result, { return NULL; });
    result->type = INPUT_EVENT_TYPE_UNKNOWN;
    return result;
}

boolean input_event_free(InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    tfree(event);
    return true;
}

boolean input_event_set_type(InputEvent* event, InputEventType type) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    if (type < INPUT_EVENT_TYPE_FIRST || type > INPUT_EVENT_TYPE_LAST) {
        LOG_ERROR_OR_DEBUG_FATAL("input_event_set_type: Unknown 'type' - %d", type);
        set_error(ERROR_INVALID_ARGUMENT);
        return false;
    }
    event->type = type;

    if (type == INPUT_EVENT_TYPE_KEY) {
        event->key.is_pressed = false;
    }

    return true;
}

InputEventType input_event_get_type(InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return INPUT_EVENT_TYPE_UNKNOWN; });
    return event->type;
}

boolean input_event_emit(const InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return false; });

    if (event->type == INPUT_EVENT_TYPE_KEY) {
        LOG_INFO("IsPressed: %b", event->key.is_pressed);
    }

    return true;
}


/* ====================================> Mouse Motion Event <====================================*/
boolean input_event_key_set_is_pressed(InputEvent* event, boolean is_pressed) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_set_is_pressed", { return false; });
    event->key.is_pressed = is_pressed;
    return true;
}


boolean input_event_key_is_pressed(InputEvent* event, boolean* success) {
    ERROR_ARGS_CHECK_1(event, {
        H_SET_SUCCESS_FALSE;
        return false;
    });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_is_pressed", {
        H_SET_SUCCESS_FALSE;
        return false;
    });

    H_SET_SUCCESS_TRUE;
    return event->key.is_pressed;
}

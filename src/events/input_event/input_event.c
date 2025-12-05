#include "input_event.h"
#include <platform/memory.h>
#include <error.h>

/* ====================================> Mouse Motion Event <====================================*/
struct InputEvent {
    int a;
};

InputEvent* input_event_new(void) {
    InputEvent* result = tmalloc(sizeof(InputEvent));
    ERROR_ALLOC_CHECK(result, { return NULL; });
    return result;
}

boolean input_event_free(InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    tfree(event);
    return true;
}

boolean input_event_set_type(InputEvent* event) {
    return true;
}

boolean input_event_emit(const InputEvent* event) {
    return true;
}

#include "input_event.h"
#include <ex_alloc/chunk_allocator.h>
#include <platform/memory.h>
#include <error.h>
#include <helpers.h>
#include "input/key.h"
#include "input/mouse.h"
#include "log.h"
#include "math/vec2.h"
#include "platform/mutex.h"

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


static ChunkMemoryAllocator g_connectedFunctions;
static mutex_handle g_connectedFunctionsMutex;

void input_event_init(void) {
    chunk_memory_allocator_constructor(&g_connectedFunctions, sizeof(InputEventCallbackFunc), 64, 4);
    g_connectedFunctionsMutex = mutex_new();
}

void input_event_exit(void) {
    chunk_memory_allocator_destructor(&g_connectedFunctions);
    mutex_free(g_connectedFunctionsMutex);
}


boolean input_event_constructor(InputEvent* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    self->type = INPUT_EVENT_TYPE_UNKNOWN;
    return true;
}

boolean input_event_destructor(InputEvent* self) {
    ERROR_ARGS_CHECK_1(self, { return false; });
    return true;
}


InputEvent* input_event_new(void) {
    InputEvent* result = tmalloc(sizeof(InputEvent));
    ERROR_ALLOC_CHECK(result, { return NULL; });
    input_event_constructor(result);
    return result;
}

boolean input_event_free(InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    input_event_destructor(event);
    tfree(event);
    return true;
}

boolean input_event_set_type(InputEvent* event, InputEventType type) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    ERROR_RANGE_CHECK(type, INPUT_EVENT_TYPE_FIRST, INPUT_EVENT_TYPE_LAST, { return false; });

    event->type = type;
    switch (type) {
        case INPUT_EVENT_TYPE_KEY:
            event->data.key.key = KEY_UNKNOWN;
            event->data.key.is_pressed = false;
            event->data.key.is_repeat = false;
            break;
        case INPUT_EVENT_TYPE_MOUSE_BUTTON:
            event->data.mouse_button.button = MOUSE_BUTTON_UNKNOWN;
            event->data.mouse_button.is_pressed = false;
            event->data.mouse_button.clicks = 0;
            event->data.mouse_button.coords = VEC2_ZERO_M;
            break;
        case INPUT_EVENT_TYPE_MOUSE_MOTION:
            event->data.mouse_motion.offset = VEC2_ZERO_M;
            event->data.mouse_motion.coords = VEC2_ZERO_M;
            break;
    }

    return true;
}

InputEventType input_event_get_type(InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return INPUT_EVENT_TYPE_UNKNOWN; });
    return event->type;
}

boolean input_event_emit(const InputEvent* event) {
    ERROR_ARGS_CHECK_1(event, { return false; });

    InputEventCallbackFunc f;

    // TODO: change to normal iterator
    mutex_lock(g_connectedFunctionsMutex);
    // for (usize byte_idx = 0; byte_idx < g_connectedFunctions.chunks_bitfield.size; ++byte_idx) {
    //     u8 byte = g_connectedFunctions.chunks_bitfield.data[byte_idx];
    //     if (byte == 0)
    //         continue;
    //
    //     for (usize bit_idx = 0; bit_idx < 8; ++bit_idx) {
    //         u8 mask = 1 << bit_idx;
    //         boolean bit = byte & mask;
    //
    //         if (!bit)
    //             continue;
    //
    //         usize chunk_idx = byte_idx / g_connectedFunctions.chunk_bitfield_size;
    //         u8* chunk = (u8*) g_connectedFunctions.chunks.data[chunk_idx];
    //         usize el_idx = ((byte_idx % g_connectedFunctions.chunk_bitfield_size) >> 3) + bit_idx;
    //         InputEventCallbackFunc* function =
    //                 (InputEventCallbackFunc*) (chunk + g_connectedFunctions.element_size * el_idx);
    //
    //         LOG_INFO("%d, %p, %d, %p", chunk_idx, chunk, el_idx, function);
    //
    //         (*function)(event);
    //     }
    // }
    ChunkMemoryAllocatorIter iter;
    InputEventCallbackFunc* function;
    chunk_memory_allocator_iter_constructor(&iter, &g_connectedFunctions);
    while (chunk_memory_allocator_iter_next(&iter, (void**) &function)) {
        (*function)(event);
    }

    mutex_unlock(g_connectedFunctionsMutex);


    if (event->type == INPUT_EVENT_TYPE_KEY) {
        LOG_INFO(
                "Key: %d, IsPressed: %b, IsRepeat: %d", event->data.key.key, event->data.key.is_pressed,
                event->data.key.is_repeat
        );
    } else if (event->type == INPUT_EVENT_TYPE_MOUSE_BUTTON) {
        LOG_INFO(
                "MouseButton: %d, IsPressed: %d, Clicks: %d, X: %f, Y: %f",
                event->data.mouse_button.button, event->data.mouse_button.is_pressed,
                event->data.mouse_button.clicks, event->data.mouse_button.coords.x,
                event->data.mouse_button.coords.y
        );
    } else if (event->type == INPUT_EVENT_TYPE_MOUSE_MOTION) {
        LOG_INFO(
                "OffsetX: %f, OffsetY: %f, CoordX: %f, CoordY: %f", event->data.mouse_motion.offset.x,
                event->data.mouse_motion.offset.y, event->data.mouse_motion.coords.x,
                event->data.mouse_motion.coords.y
        );
    }

    return true;
}

InputEventCallbackHandler input_event_connect(InputEventCallbackFunc func) {
    ERROR_ARGS_CHECK_1(func, { return 0; });
    mutex_lock(g_connectedFunctionsMutex);
    chunk_allocator_ptr ptr = chunk_memory_allocator_alloc_mem(&g_connectedFunctions);
    mutex_unlock(g_connectedFunctionsMutex);
    if (!ptr) {
        LOG_ERROR_OR_DEBUG_FATAL(
                "input_event_connoct: CallbackFunction can't be added, memory allocation error"
        );
        return ptr;
    }

    // TODO: change
    InputEventCallbackFunc* f = chunk_memory_allocator_get_real_ptr(&g_connectedFunctions, ptr);
    *f = func;
    return ptr;
}


boolean input_event_disconnect(InputEventCallbackHandler ptr) {
    ERROR_ARGS_CHECK_1(ptr, { return false; });
    mutex_lock(g_connectedFunctionsMutex);
    return chunk_memory_allocator_free_mem(&g_connectedFunctions, ptr);
    mutex_unlock(g_connectedFunctionsMutex);
}


/* ====================================> Key Event <====================================*/
boolean input_event_key_set_key(InputEvent* event, Key key) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_set_is_pressed", { return false; });
    // TODO: Add key range check
    event->data.key.key = key;
    return true;
}

boolean input_event_key_set_is_pressed(InputEvent* event, boolean is_pressed) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_set_is_pressed", { return false; });
    event->data.key.is_pressed = is_pressed;
    return true;
}

boolean input_event_key_set_is_repeat(InputEvent* event, boolean is_repeat) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_set_is_repeat", { return false; });
    event->data.key.is_repeat = is_repeat;
    return true;
}

boolean input_event_key_get_key(InputEvent* event, Key* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_is_pressed", { return false; });

    *out = event->data.key.key;
    return true;
}

boolean input_event_key_is_pressed(InputEvent* event, boolean* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_is_pressed", { return false; });

    *out = event->data.key.is_pressed;
    return true;
}

boolean input_event_key_is_repeat(InputEvent* event, boolean* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_KEY, "input_event_key_is_repeat", { return false; });

    *out = event->data.key.is_repeat;
    return true;
}

/* ====================================> Mouse Button Event <====================================*/
boolean input_event_mouse_button_set_button(InputEvent* event, MouseButton button) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_set_button", {
        return false;
    });
    ERROR_RANGE_CHECK(button, MOUSE_BUTTON_FIRST, MOUSE_BUTTON_LAST, { return false; });

    event->data.mouse_button.button = button;
    return true;
}

boolean input_event_mouse_button_set_is_pressed(InputEvent* event, boolean is_pressed) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_set_is_pressed", {
        return false;
    });
    event->data.mouse_button.is_pressed = is_pressed;
    return true;
}

boolean input_event_mouse_button_set_clicks(InputEvent* event, u8 clicks) {
    ERROR_ARGS_CHECK_1(event, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_set_clicks", {
        return false;
    });
    event->data.mouse_button.clicks = clicks;
    return true;
}

boolean input_event_mouse_button_set_coords(InputEvent* event, const Vec2* const coords) {
    ERROR_ARGS_CHECK_2(event, coords, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_set_coords", {
        return false;
    });
    event->data.mouse_button.coords = *coords;
    return true;
}


boolean input_event_mouse_button_get_button(InputEvent* event, MouseButton* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_get_button", {
        return false;
    });
    *out = event->data.mouse_button.button;
    return true;
}

boolean input_event_mouse_button_is_pressed(InputEvent* event, boolean* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_is_pressed", {
        return false;
    });
    *out = event->data.mouse_button.is_pressed;
    return true;
}

boolean input_event_mouse_button_get_clicks(InputEvent* event, u8* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_get_clicks", {
        return false;
    });
    *out = event->data.mouse_button.clicks;
    return true;
}

boolean input_event_mouse_button_get_coords(InputEvent* event, Vec2* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_BUTTON, "input_event_mouse_button_get_coords", {
        return false;
    });
    *out = event->data.mouse_button.coords;
    return true;
}


/* ====================================> Mouse Motion Event <====================================*/
boolean input_event_mouse_motion_set_offset(InputEvent* event, const Vec2* const offset) {
    ERROR_ARGS_CHECK_2(event, offset, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_MOTION, "input_event_mouse_motion_set_coords", {
        return false;
    });
    event->data.mouse_motion.offset = *offset;
    return true;
}

boolean input_event_mouse_motion_set_coords(InputEvent* event, const Vec2* const coords) {
    ERROR_ARGS_CHECK_2(event, coords, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_MOTION, "input_event_mouse_motion_set_coords", {
        return false;
    });
    event->data.mouse_motion.coords = *coords;
    return true;
}

boolean input_event_mouse_motion_get_offset(InputEvent* event, Vec2* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_MOTION, "input_event_mouse_moition_get_coords", {
        return false;
    });
    *out = event->data.mouse_motion.offset;
    return true;
}

boolean input_event_mouse_motion_get_coords(InputEvent* event, Vec2* out) {
    ERROR_ARGS_CHECK_2(event, out, { return false; });
    EVENT_TYPE_CHECK(event, INPUT_EVENT_TYPE_MOUSE_MOTION, "input_event_mouse_moition_get_coords", {
        return false;
    });
    *out = event->data.mouse_motion.coords;
    return true;
}

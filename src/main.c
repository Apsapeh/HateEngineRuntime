#include <stdlib.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_timer.h"
#include "events/input_event/input_event.h"
#include "log.h"
#include "platform/memory.h"

#include "game_loader/game_loader.h"
#include "servers/render_context/render_context.h"
#include "servers/render_server/render_server.h"
#include "types/uid.h"
#include "vfs/vfs.h"
#include "servers/platform_driver/platform_driver.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "args.h"

static void init(void);
static void exit_init(void);

int main(int argc, char* argv[]) {
    if (!parse_cli_args(argc, argv)) {
        return 0;
    }

    init();
    atexit(exit_init);

    GameFunctions game_functions = load_game();
    game_functions._ready();

    InputEvent* input_event = input_event_new();
    u64 prev_time = SDL_GetTicksNS();
    u32 frame_counter = 0;
    u64 time_counter = 0;
    while (1) {
        u64 cur_time = SDL_GetTicksNS();
        float delta = (float) (cur_time - prev_time) / 1000000000;
        ++frame_counter;
        time_counter += cur_time - prev_time;
        prev_time = cur_time;

        if (time_counter >= 1000000000) {
            LOG_INFO("%u", frame_counter);
            frame_counter = 0;
            time_counter = 0;
        }

        if (!PlatformDriver._poll_events()) {
            break;
        }

        game_functions._process(0.166);

        render_server_begin_frame(); // Исполняем очередь команд
        game_functions._render(0.166);
        RenderServer._draw(0.166);

        render_server_end_frame();
        SDL_DelayNS(16660000);
    }

    input_event_free(input_event);

    return 0;
}

// Train train tRain TRain Поезд Поезда


static void init(void) {
    memory_init();
    log_init();
    uid_init();
    vfs_init();
    input_event_init();
    platform_driver_init();
    render_context_init();
    render_server_init();
}

static void exit_init(void) {
    RenderServer._quit();
    RenderContext._quit();
    PlatformDriver._quit();

    render_server_exit();
    render_context_exit();
    platform_driver_exit();
    input_event_exit();
    vfs_exit();
    uid_exit();
    log_exit();
    memory_exit();
}

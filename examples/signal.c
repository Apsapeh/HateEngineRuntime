#define HEAPI_LOAD_IMPL
#define HEAPI_FULL_TRACE
#include <HateEngineAPI.h>

#include <stdio.h>
#include <stdlib.h>

void signal_a_fn1(void* args, void* ctx) {
    printf("Hello from Function 1, Unsafe Signal A\n");
}

void signal_a_fn2(void* args, void* ctx) {
    printf("Hello from Function 2, Unasfe Signal A\n");
}


void signal_b_fn(void* args, void* ctx) {
    int* num = ctx;
    int* arg = args;

    *num += *arg;
    printf("Signal B:\n\tArg: %d\n\tNum: %d\n", *arg, *num);

    *arg = *num;
}


SignalUnsafe* g_signalA;
Signal* g_signalB;
PUBLIC void _ready(void) {
    g_signalA = signal_unsafe_new();
    g_signalB = signal_new();

    signal_unsafe_connect(g_signalA, signal_a_fn1, NULL);
    signal_unsafe_connect(g_signalA, signal_a_fn2, NULL);

    int* ctx = tmalloc(sizeof(int));
    *ctx = 0;
    signal_connect(g_signalB, signal_b_fn, ctx);

    signal_unsafe_emit(g_signalA, NULL);
}

PUBLIC void _process(double delta) {
    int arg = 1;
    signal_emit(g_signalB, &arg);

    if (arg == 10) {
        exit(0);
    }
}

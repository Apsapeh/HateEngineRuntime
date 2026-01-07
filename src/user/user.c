#include <user/user.h>

#include <user/input.h>

void init_user(void) {
    input_init();
}

void exit_user(void) {
    input_exit();
}

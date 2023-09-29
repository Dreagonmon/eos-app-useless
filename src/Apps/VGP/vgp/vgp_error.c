#include <vgp_error.h>
#include <stddef.h>

static const char *error = NULL; 

const char *vgp_get_last_error(void) {
    return error;
}

void __vgp_set_last_error(const char *err) {
    error = err;
}

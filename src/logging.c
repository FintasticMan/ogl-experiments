#include "logging.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

static uint8_t level;
static FILE *fpt;

void tlog_init(uint8_t const loglevel, FILE *const dest) {
    level = loglevel;
    fpt = dest ? dest : stderr;
}

void tlog(uint8_t const loglevel, char const *const restrict fmt, ...) {
    if (loglevel < level) {
        return;
    }
    char time_str[9] = {0};
    time_t cur = time(NULL);
    strftime(time_str, sizeof time_str, "%T", localtime(&cur));
    fprintf(fpt, "[%" PRIu8 " %s] ", loglevel, time_str);
    va_list args;
    va_start(args, fmt);
    vfprintf(fpt, fmt, args);
    va_end(args);
    fprintf(fpt, "\n");
}

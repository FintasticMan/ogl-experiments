#include <inttypes.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#include <logging.h>

static uint8_t ll = 0;
static FILE *fp = NULL;

void tlog_init(uint8_t loglevel, FILE *dest) {
    ll = loglevel;
    fp = dest != NULL ? dest : stderr;
}

void tlog(uint8_t loglevel, const char *restrict fmt, ...) {
    if (loglevel < ll) return;
    char t[64] = {'\0'};
    time_t cur = time(NULL);
    strftime(t, sizeof t, "%T", localtime(&cur));
    fprintf(fp, "[%" PRIu8 " %s] ", loglevel, t);
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}

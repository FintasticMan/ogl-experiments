#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdint.h>

void tlog_init(uint8_t loglevel, FILE *dest);
void tlog(uint8_t loglevel, const char *restrict fmt, ...);

#endif

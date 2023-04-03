#ifndef LOGGING_H
#define LOGGING_H

#include <stdint.h>
#include <stdio.h>

void tlog_init(uint8_t loglevel, FILE *dest);
void tlog(uint8_t loglevel, char const *fmt, ...);

#endif

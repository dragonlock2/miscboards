#include <stdio.h>
#include <project.h>
#include "dbg.h"

void dbg_init() {
    debug_Start();
}

void dbg_printf(char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    char buf[128];
    vsnprintf(buf, sizeof buf, fmt, argp);
    debug_PutString(buf);
    va_end(argp);
}

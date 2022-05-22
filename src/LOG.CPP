#include "OSDBKG.HPP"

#define LOG_BODY(ch) void osd_log_##ch(const char* tag, const char* fmt, ...){\
va_list va;\
va_start(va, fmt);\
osd_log_write(#ch,tag,fmt,va);\
va_end(va); \
}

LOG_BODY(D);
LOG_BODY(E);
LOG_BODY(W);
LOG_BODY(I);
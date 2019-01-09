#pragma once

#include <string.h>

#define PROGMEM

#define strlen_P strlen

inline char pgm_read_byte_near(const char *p) { return *p; }
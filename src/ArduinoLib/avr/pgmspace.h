#pragma once

#include <string.h>

#define PROGMEM

#define strlen_P strlen
#define strncmp_P strncmp
#define strncpy_P strncpy

inline char pgm_read_byte_near(const char *p) { return *p; }
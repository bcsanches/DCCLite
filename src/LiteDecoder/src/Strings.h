#pragma once

#include <avr/pgmspace.h>

extern const char g_fstrDecoders[] PROGMEM;
#define FSTR_DECODERS Console::FlashStr(g_fstrDecoders)

extern const char g_fstrInit[] PROGMEM;
#define FSTR_INIT Console::FlashStr(g_fstrInit)

extern const char g_fstrLump[] PROGMEM;
#define FSTR_LUMP Console::FlashStr(g_fstrLump)

extern const char g_fstrName[] PROGMEM;
#define FSTR_NAME Console::FlashStr(g_fstrName)

extern const char g_fstrNode[] PROGMEM;
#define FSTR_NODE Console::FlashStr(g_fstrNode)

extern const char g_fstrNo[] PROGMEM;
#define FSTR_NO Console::FlashStr(g_fstrInit)

extern const char g_fstrPort[] PROGMEM;
#define FSTR_PORT Console::FlashStr(g_fstrPort)

extern const char g_fstrRom[] PROGMEM;
#define FSTR_ROM Console::FlashStr(g_fstrRom)

extern const char g_fstrSrvport[] PROGMEM;
#define FSTR_SRVPORT Console::FlashStr(g_fstrSrvport)

extern const char g_fstrSession[] PROGMEM;
#define FSTR_SESSION Console::FlashStr(g_fstrSession)

extern const char g_fstrUnknown[] PROGMEM;
#define FSTR_UNKNOWN Console::FlashStr(g_fstrUnknown)
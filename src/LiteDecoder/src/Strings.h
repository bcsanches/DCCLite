#pragma once

#include <avr/pgmspace.h>

extern const char g_fstrArp[] PROGMEM;
#define FSTR_ARP Console::FlashStr(g_fstrArp)

extern const char g_fstrBroadcast[] PROGMEM;
#define FSTR_BROADCAST Console::FlashStr(g_fstrBroadcast)

extern const char g_fstrDisconnect[] PROGMEM;
#define FSTR_DISCONNECT Console::FlashStr(g_fstrDisconnect)

extern const char g_fstrDecoders[] PROGMEM;
#define FSTR_DECODERS Console::FlashStr(g_fstrDecoders)

extern const char g_fstrInit[] PROGMEM;
#define FSTR_INIT Console::FlashStr(g_fstrInit)

extern const char g_fstrInvalid[] PROGMEM ;
#define FSTR_INVALID Console::FlashStr(g_fstrInvalid)

extern const char g_fstrLump[] PROGMEM;
#define FSTR_LUMP Console::FlashStr(g_fstrLump)

extern const char g_fstrName[] PROGMEM;
#define FSTR_NAME Console::FlashStr(g_fstrName)

extern const char g_fstrNode[] PROGMEM;
#define FSTR_NODE Console::FlashStr(g_fstrNode)

extern const char g_fstrNo[] PROGMEM;
#define FSTR_NO Console::FlashStr(g_fstrNo)

extern const char g_fstrNok[] PROGMEM;
#define FSTR_NOK Console::FlashStr(g_fstrNok)

extern const char g_fstrOffline[] PROGMEM;
#define FSTR_OFFLINE Console::FlashStr(g_fstrOffline)

extern const char g_fstrOk[] PROGMEM;
#define FSTR_OK Console::FlashStr(g_fstrOk)

extern const char g_fstrSetup[] PROGMEM;
#define FSTR_SETUP Console::FlashStr(g_fstrSetup)

extern const char g_fstrPort[] PROGMEM;
#define FSTR_PORT Console::FlashStr(g_fstrPort)

extern const char g_fstrRom[] PROGMEM;
#define FSTR_ROM Console::FlashStr(g_fstrRom)

extern const char g_fstrSrvport[] PROGMEM;
#define FSTR_SRVPORT Console::FlashStr(g_fstrSrvport)

extern const char g_fstrSession[] PROGMEM;
#define FSTR_SESSION Console::FlashStr(g_fstrSession)

extern const char g_fstrTimeout[] PROGMEM;
#define FSTR_TIMEOUT Console::FlashStr(g_fstrTimeout)

extern const char g_fstrUnknown[] PROGMEM;
#define FSTR_UNKNOWN Console::FlashStr(g_fstrUnknown)
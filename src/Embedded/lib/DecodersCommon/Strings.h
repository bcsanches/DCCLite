#pragma once

#include <avr/pgmspace.h>

#include <WString.h>

typedef __FlashStringHelper FlashStringHelper_t;

#define FSTR_ARP		F("ARP")
#define FSTR_BROADCAST	F("broadcast")
#define FSTR_DISCONNECT F("disconnect")
#define FSTR_DECODERS	F("decoders")
#define FSTR_EOF		F("EOF")
#define FSTR_INIT		F("init")
#define FSTR_INVALID	F("invalid")
#define FSTR_LUMP		F("lump")
#define FSTR_NAME		F("name")
#define FSTR_NODE		F("node")
#define FSTR_NO			F("no")
#define FSTR_NOK		F("NOT OK")
#define FSTR_OFFLINE	F("offline")
#define FSTR_OK			F("ok")
#define FSTR_SETUP		F("setup")
#define FSTR_PORT		F("port")
#define FSTR_ROM		F("rom")
#define FSTR_SRVPORT	F("srvport")
#define FSTR_SESSION	F("session")
#define FSTR_TIMEOUT	F("timeout")
#define FSTR_UNKNOWN	F("unknown")

#ifdef DCCLITE_ARDUINO_EMULATOR

inline size_t FStrLen(const FlashStringHelper_t *fstr)
{
	return strlen_P(fstr);
}

inline char FStrReadChar(const FlashStringHelper_t *fstr, size_t index)
{
	return reinterpret_cast<const char *>(fstr)[index];
}

inline int FStrNCmp(const char *str1, const __FlashStringHelper *fstr2, size_t maxCount)
{
	return strncmp_P(str1, fstr2, maxCount);
}

inline char *FStrCpy(char *dest, const __FlashStringHelper *fsrc, size_t count)
{
	return strncpy_P(dest, fsrc, count);
}

#else

inline size_t FStrLen(const FlashStringHelper_t *fstr)
{
	return strlen_P((PGM_P)fstr);
}

inline char FStrReadChar(const FlashStringHelper_t *fstr, size_t index)
{
	return static_cast<char>(pgm_read_byte_near((PGM_P)(fstr)+index));
}

inline int FStrNCmp(const char *str1, const __FlashStringHelper *fstr2, size_t maxCount)
{
	return strncmp_P(str1, (PGM_P)fstr2, maxCount);
}

inline char *FStrCpy(char *dest, const __FlashStringHelper *fsrc, size_t count)
{
	return strncpy_P(dest, (PGM_P)fsrc, count);
}

#endif

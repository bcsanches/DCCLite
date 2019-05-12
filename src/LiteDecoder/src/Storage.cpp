// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Storage.h"

#include <stdio.h>
#include <string.h>

#include <EEPROM.h>

#include "Console.h"
#include "DecoderManager.h"
#include "NetUdp.h"
#include "Session.h"
#include "Storage.h"
#include "Strings.h"

static const char StorageMagic[] PROGMEM = {"Bcs0008"};
static const char NetUdpStorageId[] PROGMEM = {"NetU002"};
static const char SessionStorageId[] PROGMEM = {"Sson001"};
static const char DecodersStorageId[] PROGMEM = { "DECS002" };
static const char EndStorageId[] PROGMEM = {"ENDEND1"};

const char StorageModuleName[] PROGMEM = {"Storage"} ;
#define MODULE_NAME Console::FlashStr(StorageModuleName)

//#define MODULE_NAME "Storage"

struct Lump
{
	char		m_archName[8];

	//size in bytes of the lump data
	uint16_t 	m_uLength;
};

void Storage::Dump()
{
	Console::SendLn("");

	char number[16];

	for(unsigned int i = 0;i < EEPROM.length(); ++i)
	{
		unsigned char ch = EEPROM.read(i);

		sprintf(number, "%02x ", ch);

		Console::Send(number);
	}

	Console::SendLn("");
	Console::SendLn("");
}

void Storage::DumpHex()
{
	Console::SendLn("");

	for(unsigned int i = 0;i < EEPROM.length(); ++i)
	{
		unsigned char ch = EEPROM.read(i);
		Serial.write(ch);
	}

	Console::SendLn("");
	Console::SendLn("");
}

bool Storage::LoadConfig()
{
    //Console::SendLog(MODULE_NAME, "init %d", sizeof(STORAGE_MAGIC));		
	Console::SendLogEx(MODULE_NAME, FSTR_INIT, ' ', strlen_P(StorageMagic));

	Lump header;

    memset(&header, 0, sizeof(header));

    EpromStream stream(0);	

    stream.Get(header.m_archName, sizeof(header.m_archName));
    stream.Get(header.m_uLength);

	//Console::SendLog(MODULE_NAME, gData.m_archMagic);

    if(strncmp_P(header.m_archName, StorageMagic, strlen_P(StorageMagic)))
    {
        Console::SendLogEx(MODULE_NAME, FSTR_NO, ' ', FSTR_ROM);

        return false;
    }
    else
    {
		Lump lump;		

		for (;;)
		{
			stream.Get(lump.m_archName, sizeof(lump.m_archName));
			stream.Get(lump.m_uLength);

			if (strncmp_P(lump.m_archName, NetUdpStorageId, strlen_P(NetUdpStorageId)) == 0)
			{
				Console::SendLogEx(MODULE_NAME, "net", "udp", ' ', "cfg");
				NetUdp::LoadConfig(stream);
			}
			else if (strncmp_P(lump.m_archName, SessionStorageId, strlen_P(SessionStorageId)) == 0)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_SESSION, ' ',  "cfg");

				Session::LoadConfig(stream);
			}
#if 1
			else if (strncmp_P(lump.m_archName, DecodersStorageId, strlen_P(DecodersStorageId)) == 0)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_DECODERS, ' ', "cfg");

				DecoderManager::LoadConfig(stream);
			}
#endif
			else if (strncmp_P(lump.m_archName, EndStorageId, strlen_P(EndStorageId)) == 0)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_ROM, ' ', "end");

				break;
			}
			else
			{
				Console::SendLogEx(MODULE_NAME, FSTR_UNKNOWN, ' ', FSTR_LUMP, ' ', lump.m_archName);

				stream.Skip(lump.m_uLength);
				continue;
			}

		}
    }

    Console::SendLogEx(MODULE_NAME, "ok");

    return true;
}

void Storage::SaveConfig()
{   
	EpromStream stream(0);

	//clear eprom first bytes, so we make sure only after writing everyting, we put a magic number
	for(unsigned i = 0;i < sizeof(Lump); ++i)
		stream.Put(static_cast<uint8_t>(0));

	stream.Seek(0);		

	LumpWriter lump(stream, StorageMagic, false);
	#if 1
	{
		LumpWriter netLump(stream, NetUdpStorageId, false);

		NetUdp::SaveConfig(stream);	
	}

	{
		LumpWriter sessionLump(stream, SessionStorageId, false);

		Session::SaveConfig(stream);
	}
	{
		LumpWriter decodersLump(stream, DecodersStorageId, false);

		DecoderManager::SaveConfig(stream);
	}
	#endif
	{
		LumpWriter endLump(stream, EndStorageId, false);
	}

    Console::SendLogEx(MODULE_NAME, "sv", ' ', "ok");
}

void Storage::UpdateField(unsigned int index, unsigned char byte)
{
	EEPROM.put(index, byte);
}


EpromStream::EpromStream(unsigned int index):
    m_uIndex(index)
{
    //empty
}


void EpromStream::Get(char &ch)
{
	ch = EEPROM.read(m_uIndex);
	m_uIndex += sizeof(ch);
}

void EpromStream::Get(unsigned char &byte)
{
	byte = EEPROM.read(m_uIndex);

	++m_uIndex;
}

#ifndef WIN32
void EpromStream::Get(unsigned short &number)
{

#ifdef SKIP_BYTE
	if(m_uIndex & 1)
		++m_uIndex;
#endif

	EEPROM.get(m_uIndex, number);

	m_uIndex += sizeof(number);
}
#endif

void EpromStream::Get(uint16_t &number)
{
	EEPROM.get(m_uIndex, number);

	m_uIndex += sizeof(number);
}

unsigned int EpromStream::Get(char *name, unsigned int nameSize)
{
	if(!nameSize)
		return 0;

	for(unsigned int i = 0;i < nameSize; ++i)
	{
		char ch = EEPROM.read(m_uIndex);
		++m_uIndex;

		name[i] = ch;

		if(ch == 0)
		{
      		//NetClient::sendLog(MODULE_NAME, "BLAi %s", name);

			return i;
		}
	}

	//reached the end of the buffer
	name[nameSize-1] = 0;

  	//Console::SendLog(MODULE_NAME, "BLA %s", name);

	return nameSize-1;
}

void EpromStream::Put(char ch)
{
	EEPROM.put(m_uIndex, ch);
	m_uIndex += sizeof(ch);
}

void EpromStream::Put(unsigned char byte)
{
	//NetClient::sendLog(MODULE_NAME, "w 1 byte %c at %u", (char)byte, m_uIndex);

    EEPROM.put(m_uIndex, byte);
    m_uIndex += sizeof(byte);
}

#ifndef WIN32
void EpromStream::Put(unsigned short number)
{
	//NetClient::sendLog(MODULE_NAME, "w %u bytes (short) %u at %u", sizeof(number), number,  m_uIndex);

#ifdef SKIP_BYTE
	if(m_uIndex & 1)
	{
		EEPROM.put(m_uIndex, (char)0);

		++m_uIndex;
		NetClient::sendLog(MODULE_NAME, "SKIP");
	}
#endif

    EEPROM.put(m_uIndex, number);
    m_uIndex += sizeof(number);
}
#endif

void EpromStream::Put(uint16_t number)
{	
    EEPROM.put(m_uIndex, number);
    m_uIndex += sizeof(number);
}

template <typename T>
unsigned int EpromStream::PutData(const T &data)
{
	auto index = m_uIndex;

	EEPROM.put(m_uIndex, data);

	m_uIndex += sizeof(T);

	return index;
}

void EpromStream::Seek(uint32_t pos)
{
	m_uIndex = pos;
}

void EpromStream::Skip(uint32_t bytes)
{
	m_uIndex += bytes;
}

#if 0
void EpromStream::Put(const char *str)
{
	//NetClient::sendLog(MODULE_NAME, "w str %s at %u", str,  m_uIndex);
    while(*str)
    {
        this->Put(static_cast<unsigned char>(*str));
        ++str;
    }

    //make sure 0 goes thought
    this->Put(static_cast<unsigned char>(*str));
}
#endif

LumpWriter::LumpWriter(EpromStream &stream, const char *lumpName, bool nameFromRam):
	m_pszName(lumpName),
	m_rStream(stream),
	m_uStartIndex(stream.m_uIndex),
	m_fNameFromRam(nameFromRam)
{		
	m_rStream.Skip(sizeof(Lump));

#if 0
	char name[32];
	strncpy_P(name, m_pszName, sizeof(name));
	Console::SendLogEx(MODULE_NAME, "cctor name: ",name);
#endif
}

LumpWriter::~LumpWriter()
{
#if 0
	char name[8];
	strncpy_P(name, m_pszName, sizeof(name));
	Console::SendLogEx(MODULE_NAME, "dtor name: ",name);
#endif	

	auto currentPos = m_rStream.m_uIndex;

	m_rStream.Seek(m_uStartIndex);

	Lump header;	

	if(m_fNameFromRam)
		strncpy(header.m_archName, m_pszName, sizeof(header.m_archName));
	else
		strncpy_P(header.m_archName, m_pszName, sizeof(header.m_archName));

	//strncpy(header.m_archName, name, sizeof(header.m_archName));
		
	header.m_uLength = currentPos - m_uStartIndex - sizeof(Lump);

#if 0
	Console::SendLogEx(MODULE_NAME, "m_fNameFromRam: ", m_fNameFromRam);
	Console::SendLogEx(MODULE_NAME, "sizeof(header): ", sizeof(header));
	Console::SendLogEx(MODULE_NAME, "lump: ", header.m_archName, "pos: ", (int) m_rStream.m_uIndex, " lump size: ", (int)header.m_uLength);
#endif

	m_rStream.PutData(header);

	m_rStream.Seek(currentPos);
}




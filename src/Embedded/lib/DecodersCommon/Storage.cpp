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
#include "Storage.h"
#include "Strings.h"

static const char StorageMagic[] PROGMEM = {"Bcs0008"};
static const char EndStorageId[] PROGMEM = {"ENDEND1"};

const char StorageModuleName[] PROGMEM = {"Storage"} ;
#define MODULE_NAME Console::FlashStr(StorageModuleName)

//#define MODULE_NAME "Storage"

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
	Console::SendLogEx(MODULE_NAME, 0, FSTR_INIT, ' ', static_cast<unsigned>(strlen_P(StorageMagic)));

	Lump header;

    memset(&header, 0, sizeof(header));

    EpromStream stream(0);	

    stream.GetString(header.m_archName, sizeof(header.m_archName));
    stream.Get(header.m_uLength);	

    if(strncmp_P(header.m_archName, StorageMagic, strlen_P(StorageMagic)))
    {
		//copy lump header and make sure we put a 0 at the end
		char name[LUMP_NAME_SIZE+1] = {0};
		memcpy(name, header.m_archName, LUMP_NAME_SIZE);

		//dump rom first 8 bytes to check what is in
        Console::SendLogEx(MODULE_NAME, FSTR_NO, ' ', FSTR_ROM, ' ', name);

        return false;
    }
    else
    {
		Lump lump;				

		for (;;)
		{
			stream.GetString(lump.m_archName, sizeof(lump.m_archName));
			stream.Get(lump.m_uLength);			

			if (Storage::Custom_LoadModules(lump, stream))
				continue;

			if (strncmp_P(lump.m_archName, EndStorageId, strlen_P(EndStorageId)) == 0)
			{
				Console::SendLogEx(MODULE_NAME, FSTR_ROM, ' ', "end");

				break;
			}			
			
			Console::SendLogEx(MODULE_NAME, FSTR_UNKNOWN, ' ', FSTR_LUMP, ' ', lump.m_archName);

			stream.Skip(lump.m_uLength);

			if (stream.GetIndex() >= EEPROM.length())
				break;

			continue;			
		}
    }

    Console::SendLogEx(MODULE_NAME, FSTR_OK);

    return true;
}

void Storage::SaveConfig()
{   	
	//clear eprom first bytes, so we make sure only after writing everyting, we put a magic number
	{
		EpromStream stream(0);

		LumpWriter endLump(stream, EndStorageId, false);
	}

	EpromStream stream(0);

	LumpWriter lump(stream, StorageMagic, false);
	
	Storage::Custom_SaveModules(stream);
		
	{
		LumpWriter endLump(stream, EndStorageId, false);
	}

    Console::SendLogEx(MODULE_NAME, "sv", ' ', FSTR_OK);
}

void Storage::UpdateField(unsigned int index, unsigned char byte)
{
	EEPROM.put(index, byte);
}


namespace Storage
{


	EpromStream::EpromStream(unsigned int index) :
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
		if (m_uIndex & 1)
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

	void EpromStream::GetRaw(uint8_t *data,uint16_t size)
	{
		for(uint16_t i = 0; i < size; ++i)
		{
			data[i] = EEPROM.read(m_uIndex++);			
		}		
	}

	unsigned int EpromStream::GetString(char *name, unsigned int nameSize)
	{
		if (!nameSize)
			return 0;

		for (unsigned int i = 0; i < nameSize; ++i)
		{
			char ch = EEPROM.read(m_uIndex);
			++m_uIndex;

			name[i] = ch;

			if (ch == 0)
			{
				//NetClient::sendLog(MODULE_NAME, "BLAi %s", name);

				return i;
			}
		}

		//reached the end of the buffer
		name[nameSize - 1] = 0;

		//Console::SendLog(MODULE_NAME, "BLA %s", name);

		return nameSize - 1;
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
		if (m_uIndex & 1)
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

	unsigned int EpromStream::PutRawData(const uint8_t *data, uint16_t size)
	{
		auto index = m_uIndex;

		for (uint16_t i = 0; i < size; ++i)
			EEPROM.put(m_uIndex++, data[i]);		

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
		while (*str)
		{
			this->Put(static_cast<unsigned char>(*str));
			++str;
		}

		//make sure 0 goes thought
		this->Put(static_cast<unsigned char>(*str));
	}
#endif

	LumpWriter::LumpWriter(EpromStream &stream, const char *lumpName, bool nameFromRam) :
		m_pszName(lumpName),
		m_rStream(stream),
		m_uStartIndex(stream.m_uIndex),
		m_fNameFromRam(nameFromRam)
	{
		m_rStream.Skip(sizeof(Storage::Lump));

#if 0
		char name[32];
		strncpy_P(name, m_pszName, sizeof(name));
		Console::SendLogEx(MODULE_NAME, "cctor name: ", name);
#endif
	}

	LumpWriter::~LumpWriter()
	{
#if 0
		char name[8];
		strncpy_P(name, m_pszName, sizeof(name));
		Console::SendLogEx(MODULE_NAME, "dtor name: ", name);
#endif	

		auto currentPos = m_rStream.m_uIndex;

		m_rStream.Seek(m_uStartIndex);

		Storage::Lump header;

		if (m_fNameFromRam)
			strncpy(header.m_archName, m_pszName, sizeof(header.m_archName));
		else
			strncpy_P(header.m_archName, m_pszName, sizeof(header.m_archName));

		//strncpy(header.m_archName, name, sizeof(header.m_archName));

		header.m_uLength = currentPos - m_uStartIndex - static_cast<uint16_t>(sizeof(Storage::Lump));

#if 0
		Console::SendLogEx(MODULE_NAME, "m_fNameFromRam: ", m_fNameFromRam);
		Console::SendLogEx(MODULE_NAME, "sizeof(header): ", sizeof(header));
		Console::SendLogEx(MODULE_NAME, "lump: ", header.m_archName, "pos: ", (int)m_rStream.m_uIndex, " lump size: ", (int)header.m_uLength);
#endif

		m_rStream.PutData(header);

		m_rStream.Seek(currentPos);
	}
}



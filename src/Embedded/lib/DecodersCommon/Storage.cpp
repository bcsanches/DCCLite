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

#define STORAGE_MAGIC F("Bcs0008")
#define END_STORAGE_ID F("ENDEND1")

#define MODULE_NAME F("Storage")

//#define MODULE_NAME "Storage"

void Storage::Dump()
{
	Console::OutputStream stream;

	stream << DCCLITE_ENDL;	

	for(unsigned int i = 0;i < EEPROM.length(); ++i)
	{		
		stream << EEPROM.read(i) << ' ';
	}

	stream << DCCLITE_ENDL << DCCLITE_ENDL;	
}

void Storage::DumpHex()
{
	DCCLITE_LOG << DCCLITE_ENDL;

	for(unsigned int i = 0;i < EEPROM.length(); ++i)
	{
		unsigned char ch = EEPROM.read(i);
		Serial.write(ch);
	}

	DCCLITE_LOG << DCCLITE_ENDL << DCCLITE_ENDL;		
}

void Storage::Clear()
{
	EpromStream stream(0);

	//just invalidate the header...
	for(int i = 0;i < 8; ++i)
		stream.Put(' ');	
}

extern size_t Storage::Length() noexcept
{
	return EEPROM.length();
}

bool Storage::LoadConfig()
{	    
	DCCLITE_LOG_MODULE_LN(FSTR_INIT << ' ' << static_cast<unsigned>(FStrLen(STORAGE_MAGIC)));

	Lump header;

    memset(&header, 0, sizeof(header));

    EpromStream stream(0);	

    stream.GetString(header.m_archName, sizeof(header.m_archName));
    stream.Get(header.m_uLength);	

    if(FStrNCmp(header.m_archName, STORAGE_MAGIC, FStrLen(STORAGE_MAGIC)))
    {
		//copy lump header and make sure we put a 0 at the end
		char name[LUMP_NAME_SIZE+1] = {0};
		memcpy(name, header.m_archName, LUMP_NAME_SIZE);

		//dump rom first 8 bytes to check what is in        
		DCCLITE_LOG_MODULE_LN(FSTR_NO << ' ' << FSTR_ROM << ' ' << name);

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

			if (FStrNCmp(lump.m_archName, END_STORAGE_ID, FStrLen(END_STORAGE_ID)) == 0)
			{
				//Console::SendLogEx(MODULE_NAME, FSTR_ROM, ' ', "end");
				//DCCLITE_LOG << MODULE_NAME << FSTR_ROM << ' ' << F("end") << DCCLITE_ENDL;

				break;
			}			
						
			DCCLITE_LOG << MODULE_NAME << ' ' << FSTR_UNKNOWN << ' ' << FSTR_LUMP << ' ' << lump.m_archName << DCCLITE_ENDL;

			stream.Skip(lump.m_uLength);

			if (stream.GetIndex() >= EEPROM.length())
				break;		
		}
    }
    
	DCCLITE_LOG_MODULE_LN(FSTR_OK);

    return true;
}

void Storage::SaveConfig()
{   	
	//clear eprom first bytes, so we make sure only after writing everyting, we put a magic number
	{
		EpromStream stream(0);

		LumpWriter endLump(stream, END_STORAGE_ID);
	}

	EpromStream stream(0);

	LumpWriter lump(stream, STORAGE_MAGIC);
	
	Storage::Custom_SaveModules(stream);
		
	{
		LumpWriter endLump(stream, END_STORAGE_ID);
	}

    //Console::SendLogEx(MODULE_NAME, "sv", ' ', FSTR_OK);
	DCCLITE_LOG_MODULE_LN(F("sv") << ' ' << FSTR_OK);	
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

#ifndef BCS_ARDUINO_EMULATOR
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

#ifndef BCS_ARDUINO_EMULATOR
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

	LumpWriter::LumpWriter(EpromStream &stream, const FlashStringHelper_t *lumpName) :
		m_pfszName(lumpName),
		m_rStream(stream),
		m_uStartIndex(stream.m_uIndex)		
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

#if 0
		if (m_fNameFromRam)
			strncpy(header.m_archName, m_pfszName, sizeof(header.m_archName));
		else
#endif
			FStrCpy(header.m_archName, m_pfszName, sizeof(header.m_archName));
		

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



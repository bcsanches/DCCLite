// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef _STORAGE_H
#define _STORAGE_H

#include <Arduino.h>



#define LUMP_NAME_SIZE 8

namespace Storage
{
	class EpromStream
	{
		private:
			friend class LumpWriter;

			uint32_t m_uIndex;

			//for lump writer		
			void Seek(uint32_t pos);

		public:
			explicit EpromStream(unsigned int index);

			void Get(char &ch);
			void Get(unsigned char &byte);

			uint32_t GetIndex() const
			{
				return m_uIndex;
			}

			void Skip(uint32_t bytes);

	#ifndef WIN32
			void Get(unsigned short &number);
	#endif
			void Get(uint16_t &number);

			unsigned int Get(char *name, unsigned int nameSize);

			void Put(char ch);
			void Put(unsigned char byte);

	#ifndef WIN32
			void Put(unsigned short number);
	#endif

			void Put(uint16_t number);

			template <typename T>
			unsigned int PutData(const T &data);

	#if 0 //CHECK THIS
			void Put(const char *str);
	#endif

	};

	struct Lump
	{
		char		m_archName[LUMP_NAME_SIZE];

		//size in bytes of the lump data
		uint16_t 	m_uLength;
	};


	class LumpWriter
	{
		private:
			const char *m_pszName;
			EpromStream &m_rStream;

			uint16_t	m_uStartIndex;

			bool 		m_fNameFromRam;

		public:
			LumpWriter(EpromStream &stream, const char *lumpName, bool nameFromRam = true);

			~LumpWriter();
	};

	extern bool LoadConfig();	

	extern void SaveConfig();

	extern void Dump();
	extern void DumpHex();

	extern void UpdateField(unsigned int index, unsigned char byte);

	extern bool Custom_LoadModules(const Storage::Lump &lump, EpromStream &stream);
	extern void Custom_SaveModules(EpromStream &stream);
};

#endif

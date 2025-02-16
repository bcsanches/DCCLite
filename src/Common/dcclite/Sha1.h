// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <cstring>

#include "FileSystem.h"

namespace dcclite
{
	constexpr auto SHA1_LENGTH = 20;

	namespace detail
	{
		///////////////////////////////////////////////////////////////////////////////
		//
		// SHA1.cpp : Console App to hash files using SHA-1.
		//
		//            Version 1.0 -- 2011, October 29th
		//
		// Copyright 2011, by Giovanni Dicanio <giovanni.dicanio@gmail.com>
		//
		///////////////////////////////////////////////////////////////////////////////

		//-----------------------------------------------------------------------------
		// Class defining private copy constructor and operator=, to ban copies.
		//-----------------------------------------------------------------------------
		class NonCopyable
		{
			protected:
				NonCopyable() {}
				~NonCopyable() {}
			private:
				NonCopyable(const NonCopyable&);
				const NonCopyable& operator=(const NonCopyable&);
		};

		//-----------------------------------------------------------------------------
		// Wrapper around C FILE *, for reading binary data from file.
		//-----------------------------------------------------------------------------
		class FileReader : NonCopyable
		{
			public:

				// Opens the specified file.
				explicit FileReader(const char* filename)
				{
					m_file = fopen(filename, "rb");
					if (!m_file)
					{
						throw std::runtime_error("Can't open file for reading.");
					}
				}


				// Closes the file.
				~FileReader()
				{
					if (m_file != nullptr)
						fclose(m_file);
				}


				// End Of File reached?
				bool EoF() const
				{
					return feof(m_file) ? true : false;
				}


				// Reads bytes from file to a memory buffer.
				// Returns the number of bytes actually read.
				size_t Read(void* buffer, size_t bufferSize)
				{
					return fread(buffer, 1, bufferSize, m_file);
				}


				//
				// IMPLEMENTATION
				//
			private:
				// Raw C file handle
				FILE* m_file;
		};

	}

	struct Sha1
	{		
		unsigned char mData[SHA1_LENGTH];

		Sha1()
		{
			memset(mData, 0, sizeof(mData));
		}			

		std::string ToString() const;

		void ComputeForFile(const fs::path &fileName);
		bool TryLoadFromString(std::string_view str);

		inline bool operator!=(const Sha1 &rhs) const
		{
			return memcmp(mData, rhs.mData, sizeof(mData)) != 0;
		}

		private:

			template<typename T>
			void ComputeForFile(const fs::path& fileName)
			{
				T hasher{};

				// Object to read data from file
				detail::FileReader file(fileName.string().c_str());

				// Read buffer
				std::vector<std::uint8_t> buffer(4 * 1024);   // 4 KB buffer

				// Reading loop
				while (!file.EoF())
				{
					// Read a chunk of data from file to memory buffer
					auto readBytes = file.Read(buffer.data(), buffer.size());

					// Hash this chunk of data
					hasher.Compute(buffer.data(), readBytes);
				}

				// Finalize hashing
				hasher.Finalize(mData);
			}
	};		
} //end of namespace dcclite
	



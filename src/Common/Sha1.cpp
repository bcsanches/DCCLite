///////////////////////////////////////////////////////////////////////////////
//
// SHA1.cpp : Console App to hash files using SHA-1.
//
//            Version 1.0 -- 2011, October 29th
//
// Copyright 2011, by Giovanni Dicanio <giovanni.dicanio@gmail.com>
//
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS

#include "Sha1.h"

#include <stdio.h>      // C file management

#include <exception>    // std::exception
#include <ios>          // std::hex
#include <iostream>     // console output
#include <sstream>      // std::ostringstream
#include <stdexcept>    // std::runtime_error
#include <string>       // std::wstring
#include <vector>       // std::vector

#include <Windows.h>    // Win32 Platform SDK
#include <bcrypt.h>     // Cryptography API

#include "Misc.h"
#include "Util.h"

#ifndef WIN32
#error "Please implement"
#endif


//#pragma comment(lib, "bcrypt.lib")


//-----------------------------------------------------------------------------
// Checks if input NTSTATUS corresponds to a success code.
//-----------------------------------------------------------------------------
inline bool NtSuccess(NTSTATUS status)
{
	return (status >= 0);
}



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
// Error occurred during cryptographic processing.
//-----------------------------------------------------------------------------
class CryptException :
	public std::runtime_error
{
	public:

		// Constructs the exception with an error message and an error code.
		explicit CryptException(const std::string & message, NTSTATUS errorCode)
			: std::runtime_error(FormatErrorMessage(message, errorCode)),
			m_errorCode(errorCode)
		{}


		// Returns the error code.
		NTSTATUS ErrorCode() const
		{
			return m_errorCode;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Error code from Cryptography API
		NTSTATUS m_errorCode;

		// Helper method to format an error message including the error code.
		static std::string FormatErrorMessage(const std::string & message, NTSTATUS errorCode)
		{
			std::ostringstream os;
			os << message << " (NTSTATUS=0x" << std::hex << errorCode << ")";
			return os.str();
		}
};



//-----------------------------------------------------------------------------
// RAII wrapper to crypt algorithm provider
//-----------------------------------------------------------------------------
class CryptAlgorithmProvider : NonCopyable
{
	public:

		// Creates a crypt algorithm provider object.
		// This can be used to create one ore more hash objects to hash some data.
		CryptAlgorithmProvider()
		{
			NTSTATUS result = ::BCryptOpenAlgorithmProvider(
				&m_alg,                     // algorithm handle
				BCRYPT_SHA1_ALGORITHM,      // hashing algorithm ID
				nullptr,                    // use default provider
				0                           // default flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't load default cryptographic algorithm provider.", result);
		}


		// Releases resources
		~CryptAlgorithmProvider()
		{
			::BCryptCloseAlgorithmProvider(m_alg, 0);
		}


		// Gets raw handle
		BCRYPT_ALG_HANDLE Handle() const
		{
			return m_alg;
		}


		// Gets the value of a DWORD named property
		DWORD GetDWordProperty(const std::wstring & propertyName) const
		{
			DWORD propertyValue;
			DWORD resultSize;

			//
			// Get the value of the input named property
			//

			NTSTATUS result = ::BCryptGetProperty(
				Handle(),
				propertyName.c_str(),
				reinterpret_cast<BYTE *>(&propertyValue),
				sizeof(propertyValue),
				&resultSize,
				0);
			if (!NtSuccess(result))
				throw CryptException("Can't get crypt property value.", result);

			return propertyValue;
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Handle to crypt algorithm provider
		BCRYPT_ALG_HANDLE m_alg;
};



//-----------------------------------------------------------------------------
// Crypt Hash object, used to hash data.
//-----------------------------------------------------------------------------
class CryptHashObject : NonCopyable
{
	public:

		// Creates a crypt hash object.
		explicit CryptHashObject(const CryptAlgorithmProvider & provider)
			: m_hashObj(provider.GetDWordProperty(BCRYPT_OBJECT_LENGTH))
		{
			// Create the hash object
			NTSTATUS result = ::BCryptCreateHash(
				provider.Handle(),						// handle to parent
				&m_hash,								// hash object handle
				m_hashObj.data(),						// hash object buffer pointer
				static_cast<DWORD>(m_hashObj.size()),   // hash object buffer length
				nullptr,								// no secret
				0,										// no secret
				0										// no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't create crypt hash object.", result);
		}


		// Releases resources
		~CryptHashObject()
		{
			::BCryptDestroyHash(m_hash);
		}


		// Hashes the data in the input buffer.
		// Can be called one or more times.
		// When finished with input data, call FinishHash().
		// This method can't be called after FinisHash() is called.
		void HashData(const void * data, ULONG length) const
		{
			// Hash this chunk of data
			NTSTATUS result = ::BCryptHashData(
				m_hash, // hash object handle
				static_cast<UCHAR *>(const_cast<void *>(data)),    // safely remove const from buffer pointer
				length, // input buffer length in bytes
				0       // no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't hash data.", result);
		}


		// Finalizes hash calculation.
		// After this method is called, hash value can be got using HashValue() method.
		// After this method is called, the HashData() method can't be called anymore.
		void FinishHash(unsigned char hashValue[20])
		{
			//
			// Retrieve the hash of the accumulated data
			//

			//BYTE hashValue[20]; // SHA-1: 20 bytes = 160 bits

			NTSTATUS result = ::BCryptFinishHash(
				m_hash,             // handle to hash object
				hashValue,          // output buffer for hash value
				20,					// size of this buffer
				0                   // no flags
			);
			if (!NtSuccess(result))
				throw CryptException("Can't finalize hashing.", result);


			
		}		

		//
		// IMPLEMENTATION
		//
	private:

		// Handle to hash object
		BCRYPT_HASH_HANDLE m_hash;

		// Buffer to store hash object
		std::vector<BYTE> m_hashObj;		
};

//-----------------------------------------------------------------------------
// Wrapper around C FILE *, for reading binary data from file.
//-----------------------------------------------------------------------------
class FileReader : NonCopyable
{
	public:

		// Opens the specified file.
		explicit FileReader(const char *filename)
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
		size_t Read(void * buffer, size_t bufferSize)
		{
			return fread(buffer, 1, bufferSize, m_file);
		}


		//
		// IMPLEMENTATION
		//
	private:
		// Raw C file handle
		FILE * m_file;
};

dcclite::Sha1::Sha1()
{
	memset(mData, 0, sizeof(mData));
}

std::string dcclite::Sha1::ToString() const
{
	//
	// Get the hash digest string from hash value buffer.
	//

	// Each byte --> 2 hex chars
	std::string hashDigest;
	hashDigest.resize(sizeof(mData) * 2);

	// Upper-case hex digits
	static const char hexDigits[] = "0123456789ABCDEF";

	// Index to current character in destination string
	size_t currChar = 0;

	// For each byte in the hash value buffer
	for (size_t i = 0; i < sizeof(mData); ++i)
	{
		// high nibble
		hashDigest[currChar] = hexDigits[(mData[i] & 0xF0) >> 4];
		++currChar;

		// low nibble
		hashDigest[currChar] = hexDigits[(mData[i] & 0x0F)];
		++currChar;
	}

	return hashDigest;
}

void dcclite::Sha1::ComputeForFile(const std::filesystem::path &fileName)
{
	// Create the algorithm provider for SHA-1 hashing
	CryptAlgorithmProvider sha1;

	// Create the hash object for the particular hashing
	CryptHashObject hasher(sha1);

	// Object to read data from file
	FileReader file(fileName.string().c_str());

	// Read buffer
	std::vector<BYTE> buffer(4 * 1024);   // 4 KB buffer

	// Reading loop
	while (!file.EoF())
	{
		// Read a chunk of data from file to memory buffer
		auto readBytes = file.Read(buffer.data(), buffer.size());

		// Hash this chunk of data
		hasher.HashData(buffer.data(), static_cast<ULONG>(readBytes));
	}

	// Finalize hashing
	hasher.FinishHash(mData);	
}



bool dcclite::Sha1::TryLoadFromString(std::string_view str)
{	
	return TryHexStrToBinary(mData, sizeof(mData), str);	
}



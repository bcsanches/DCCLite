///////////////////////////////////////////////////////////////////////////////
//
// SHA1.cpp : Console App to hash files using SHA-1.
//
//            Version 1.0 -- 2011, October 29th
//
// Copyright 2011, by Giovanni Dicanio <giovanni.dicanio@gmail.com>
//
///////////////////////////////////////////////////////////////////////////////

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
class CryptAlgorithmProvider : dcclite::detail::NonCopyable
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
class CryptHashObject : dcclite::detail::NonCopyable
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

class HasherWrapper: public dcclite::detail::NonCopyable
{
	public:
		HasherWrapper():
			mHasher{mProvider}
		{
			//empty
		}

		void Compute(const void* data, size_t length)
		{
			mHasher.HashData(data, static_cast<ULONG>(length));
		}

		void Finalize(unsigned char hash[dcclite::SHA1_LENGTH])
		{
			mHasher.FinishHash(hash);
		}

	private:
		CryptAlgorithmProvider	mProvider;
		CryptHashObject			mHasher;
};

void dcclite::Sha1::ComputeForFile(const std::filesystem::path &fileName)
{
	ComputeForFile<HasherWrapper>(fileName);
}





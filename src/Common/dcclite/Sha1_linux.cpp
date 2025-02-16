// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <openssl/evp.h>
#include <cassert>

#include "Sha1.h"

class Hasher : public dcclite::detail::NonCopyable
{
	public:
		Hasher()
		{			
			m_pContext = EVP_MD_CTX_new();
			if(m_pContext == nullptr)
			{
				throw std::runtime_error("[Hasher] Cannot create SSL context");
			}

			if(!EVP_DigestInit(m_pContext, EVP_sha1()))
			{
				EVP_MD_CTX_free(m_pContext);

				throw std::runtime_error("[Hasher] Cannot init SSL digest");
			}						
		}

		~Hasher()
		{			
			EVP_MD_CTX_free(m_pContext);
		}

		void Compute(const void* data, size_t length)
		{
			EVP_DigestUpdate(m_pContext, data, length);
		}

		void Finalize(unsigned char hash[dcclite::SHA1_LENGTH])
		{
			unsigned int s = dcclite::SHA1_LENGTH;

			EVP_DigestFinal_ex(m_pContext, hash, &s);
		}

	private:		
		EVP_MD_CTX *m_pContext;
};

void dcclite::Sha1::ComputeForFile(const dcclite::fs::path &fileName)
{
	ComputeForFile<Hasher>(fileName);
}

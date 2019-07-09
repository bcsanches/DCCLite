// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include <openssl/sha.h>
#include <cassert>

#include "Sha1.h"


class Hasher : public dcclite::detail::NonCopyable
{
	public:
		Hasher()
		{
			assert(dcclite::SHA1_LENGTH == SHA_DIGEST_LENGTH);

			SHA1_Init(&m_Context);
		}

		~Hasher()
		{
			if (!m_fFinalized)
			{
				unsigned char hash[dcclite::SHA1_LENGTH];

				SHA1_Final(hash, &m_Context);
			}
		}

		void Compute(const void* data, size_t length)
		{
			assert(!m_fFinalized);

			SHA1_Update(&m_Context, data, length);
		}

		void Finalize(unsigned char hash[dcclite::SHA1_LENGTH])
		{
			assert(!m_fFinalized);

			SHA1_Final(hash, &m_Context);
			m_fFinalized = true;
		}

	private:
		bool m_fFinalized = false;

		SHA_CTX m_Context;
};

void dcclite::Sha1::ComputeForFile(const dcclite::fs::path &fileName)
{
	ComputeForFile<Hasher>(fileName);
}

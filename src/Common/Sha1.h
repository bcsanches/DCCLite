#pragma once

#include <filesystem>

namespace dcclite
{
	struct Sha1
	{		
		unsigned char mData[20];

		Sha1();

		std::string ToString() const;

		void ComputeForFile(const std::filesystem::path &fileName);
		bool TryLoadFromString(std::string_view str);

		inline bool operator!=(const Sha1 &rhs) const
		{
			return memcmp(mData, rhs.mData, sizeof(mData)) != 0;
		}
	};		
}
	
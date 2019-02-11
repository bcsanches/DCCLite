#pragma once

#include <filesystem>

namespace dcclite
{
	struct Sha1
	{		
		unsigned char mData[20];

		Sha1();

		std::string ToString() const;
	};

	void ComputeSha1ForFile(Sha1 &dest, const std::filesystem::path &fileName);
}
	
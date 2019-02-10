#pragma once

#include <filesystem>

class Project
{
	public:
		Project(std::filesystem::path path) :
			m_pthRoot(std::move(path))
		{
			//empty
		}

		std::filesystem::path GetFilePath(const std::string_view fileName) const
		{
			std::filesystem::path path(m_pthRoot);

			path.append(fileName);

			return path.string();
		}

	private:
		const std::filesystem::path m_pthRoot;
};

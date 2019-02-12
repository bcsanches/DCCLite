#pragma once

#include <filesystem>

#include "Guid.h"
#include "Sha1.h"

class Project;



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

		std::filesystem::path GetAppFilePath(const std::string_view fileName) const;

		dcclite::Guid GetFileToken(const std::string_view fileName) const;

		inline void SetName(std::string_view name)
		{
			m_strName = name;
		}

	private:
		const std::filesystem::path m_pthRoot;
		std::string m_strName;
};

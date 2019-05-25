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

#include <filesystem>

#include "Guid.h"
#include "Sha1.h"

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

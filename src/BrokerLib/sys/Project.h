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

#include <FileSystem.h>

#include "Guid.h"
#include "Sha1.h"

namespace dcclite::broker
{

	class Project
	{
		public:
			explicit Project(dcclite::fs::path path)  :
				m_pthRoot(std::move(path))
			{
				//empty
			}

			dcclite::fs::path GetFilePath(const std::string_view fileName) const
			{
				dcclite::fs::path path(m_pthRoot);

				path.append(fileName);

				return path.string();
			}

			dcclite::fs::path GetAppFilePath(const std::string_view fileName) const;			

			inline void SetName(std::string_view name)
			{
				m_strName = name;
			}

			inline const std::string &GetName() const noexcept
			{
				return m_strName;
			}

			/// <summary>
			/// 
			/// </summary>
			/// <returns>Returns the path to the root used for the current project</returns>
			inline const dcclite::fs::path &GetRoot() const noexcept
			{
				return m_pthRoot;
			}

		private:
			const dcclite::fs::path m_pthRoot;
			std::string m_strName;
	};
}
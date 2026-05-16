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

#include <dcclite/FileSystem.h>
#include <dcclite/Sha1.h>

namespace dcclite::broker::sys
{
	namespace Project
	{
		/**
		* The project working directory. This will be used to search for project related files
		* 
		* 						
		*/
		void SetWorkingDir(dcclite::fs::path path);


		/**
		*
		* Gets a file path relative to the project working directory. This is used to search for project config files
		* 
		* 
		* 
		*/
		dcclite::fs::path GetFilePath(const std::string_view fileName);

		/**
		*	Gets a path on AppData folder (win32) or ~/.local/share (linux) for the given file name. 
		* 
		*	This is used to store project related files that should not be shared between different projects, such as state files.
		* 
		*	This is for auto generated files and should not be used for user created files
		*
		*/
		dcclite::fs::path GetAppFilePath(const std::string_view fileName);

		/**
		*	Sets an app name for using with the App folder. 
		* 		
		*	If you call SetName("MyProject"), 
		*	then GetAppFilePath("state.json") will return something like "C:\Users\Username\AppData\Local\dcclite\MyProject\state.jsdccliteon" on Windows 
		*	or "/home/username/.local/share/dcclite/MyProject/state.json" on Linux.
		* 
		*/
		void SetName(std::string_view name);			

		const std::string &GetName() noexcept;			

		/// <summary>
		/// 
		/// </summary>
		/// <returns>Returns the path to the root used for the current project</returns>
		const dcclite::fs::path &GetRoot() noexcept;
	}
}

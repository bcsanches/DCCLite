// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Settings.h"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "FileSystem.h"
#include "Log.h"
#include "PathUtils.h"

namespace dcclite::panel_editor::Settings
{	
	static auto constexpr SETTINGS_FILE_NAME = "LitePanelEditorSDL.settings.json";
	static auto constexpr APP_PATH = "LitePanelEditorSDL";

	struct SettingsData
	{
		void Clear()
		{
			m_vecRecentFiles.clear();

			m_fLoaded = false;
		}

		std::vector<fs::path>	m_vecRecentFiles;


		bool					m_fLoaded;
	};

	static SettingsData g_stSettings;

	static fs::path GetAppPath()
	{
		auto cacheFilePath = dcclite::PathUtils::GetAppFolder();

		cacheFilePath.append(APP_PATH);

		return cacheFilePath;
	}

	static fs::path GetSettingFilePath()
	{
		auto path = GetAppPath();

		path.append(SETTINGS_FILE_NAME);

		return path;
	}

	static bool TryLoadSettings()
	{
		auto settingsFilePath = GetSettingFilePath();

		if (!fs::exists(settingsFilePath))
			return false;

		std::ifstream settingsFile(settingsFilePath);

		if (!settingsFile)
		{
			dcclite::Log::Warn("[Settings::TryLoadSettings] Settings file not found.");

			return false;
		}		

		g_stSettings.Clear();

		rapidjson::IStreamWrapper isw(settingsFile);
		rapidjson::Document data;
		data.ParseStream(isw);
		
		const auto recentFilesData = data.FindMember("recentFiles");
		if ((recentFilesData != data.MemberEnd()) && (!recentFilesData->value.IsArray()))
		{			
			dcclite::Log::Error("[Settings::TryLoadSettings] Settings recentFiles on settings data not found or invalid, expected array.");

			return false;
		}

		auto recentFilesDataArray = recentFilesData->value.GetArray();
		for (auto &it : recentFilesDataArray)
		{
			g_stSettings.m_vecRecentFiles.push_back(it.GetString());
		}

		g_stSettings.m_fLoaded = true;

		return true;
	}

	std::optional<dcclite::fs::path> GetLastProjectPath()
	{
		if ((!g_stSettings.m_fLoaded) && !TryLoadSettings() && g_stSettings.m_vecRecentFiles.empty())
			return {};

		
		return g_stSettings.m_vecRecentFiles.back();
	}
}

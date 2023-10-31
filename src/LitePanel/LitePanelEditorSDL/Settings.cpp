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
		//already loaded, ignore it...
		if (g_stSettings.m_fLoaded)
			return true;

		//try once to load... it it fails... keep a blank settings...
		g_stSettings.m_fLoaded = true;

		auto settingsFilePath = GetSettingFilePath();

		if (!fs::exists(settingsFilePath))
		{
			dcclite::Log::Warn("[Settings::TryLoadSettings] Settings path not found.");

			return false;
		}			

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

		return true;
	}

	std::optional<dcclite::fs::path> GetLastProjectPath()
	{
		if (!TryLoadSettings() && g_stSettings.m_vecRecentFiles.empty())
			return {};

		
		return g_stSettings.m_vecRecentFiles.back();
	}

	void AddRecentProject(dcclite::fs::path path)
	{
		TryLoadSettings();

		auto it = std::find(g_stSettings.m_vecRecentFiles.begin(), g_stSettings.m_vecRecentFiles.end(), path);

		if (it != g_stSettings.m_vecRecentFiles.end())
		{			
			//if already in the last post, just ignore it, already in the correct spot
			if (it == g_stSettings.m_vecRecentFiles.end() - 1)
			{
				return;
			}

			//not fast, but this is not expected to be called often... so code in the easy way
			g_stSettings.m_vecRecentFiles.erase(it);
		}
		
		//add it 
		g_stSettings.m_vecRecentFiles.push_back(path);
	}

	const std::vector<dcclite::fs::path> &GetRecentFiles()
	{
		TryLoadSettings();

		return g_stSettings.m_vecRecentFiles;
	}
}

#include "Project.h"

#include "PathUtils.h"

#include "Log.h"

FileState::FileState(const Project &owner, std::string_view fileName)
{
	auto filePath = owner.GetFilePath(fileName);	

	std::string tmp(fileName);
	tmp.append(".state");

	auto cacheFilePath = owner.GetAppFilePath(fileName);

	dcclite::Sha1 hash;
	dcclite::ComputeSha1ForFile(hash, filePath);

	dcclite::Log::Trace("hash {} -> {}", filePath.string(), hash.ToString());
}


std::filesystem::path Project::GetAppFilePath(const std::string_view fileName) const
{
	auto cacheFilePath = dcclite::PathUtils::GetAppFolder();

	cacheFilePath.append(m_strName);
	cacheFilePath.append(fileName);

	return cacheFilePath;
}
#pragma once

#include <vector>

class Project;

namespace detail
{
	template <typename T, typename Y>
	std::vector<const T *> FillVector(const Y &map)
	{
		std::vector<const T *> vec;

		vec.reserve(map.size());

		for (auto it = map.cbegin(), end = map.cend(); it != end; ++it)
		{
			vec.push_back(it->second.get());
		}

		return vec;
	}
}

class ProjectItem
{
	protected:
		ProjectItem() = delete;
		ProjectItem(const ProjectItem &) = delete;
		ProjectItem(const ProjectItem &&) = delete;

		ProjectItem(Project& project) :
			m_rclProject(project)
		{
			//empty
		}

		Project &m_rclProject;

		virtual ~ProjectItem() = default;
};

class NamedProjectItem: public ProjectItem
{
	protected:
		NamedProjectItem(Project& project, std::string name):
			ProjectItem(project),			
			m_strName(std::move(name))
		{
			//empty
		}

		bool IsEqual(const NamedProjectItem& rhs) const
		{
			return this == &rhs;
		}

	public:		
		const std::string& GetName() const
		{
			return m_strName;
		}

	private:	
		std::string		m_strName;
};

class NetworkType: public NamedProjectItem
{
	public:
		NetworkType(Project& project, std::string name):
			NamedProjectItem(project, std::move(name))
		{
			//empty
		}

		bool operator ==(const NetworkType& rhs) const
		{
			return this->IsEqual(rhs);
		}

		bool operator !=(const NetworkType& rhs) const
		{
			return !this->IsEqual(rhs);
		}
};

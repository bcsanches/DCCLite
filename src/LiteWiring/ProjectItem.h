#pragma once

class Project;

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
		NamedProjectItem(Project& project, const IntId_t id, std::string name):
			ProjectItem(project),
			m_Id(id),
			m_strName(std::move(name))
		{
			//empty
		}

		bool IsEqual(const NamedProjectItem& rhs) const
		{
			return this == &rhs;
		}

	public:
		const IntId_t GetId() const
		{
			return m_Id;
		}

		const std::string& GetName() const
		{
			return m_strName;
		}

	private:
		const IntId_t	m_Id;
		std::string		m_strName;
};

class NetworkType: public NamedProjectItem
{
	public:
		NetworkType(Project& project, const IntId_t id, std::string name):
			NamedProjectItem(project, id, std::move(name))
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

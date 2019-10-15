#pragma once

#include <wx/splitter.h>
#include <wx/treectrl.h>

class Project;

class ProjectView: wxSplitterWindow
{
	public:
		ProjectView(wxWindow *parent);

		void SetProject(Project *project);

	private:
		void OnSelChanged(wxTreeEvent &event);

	private:
		//
		// GUI Stuff
		//
		wxTreeCtrl *m_pclTreeView = nullptr;

		wxTreeItemId		m_clTreeRootId;

		wxTreeItemId		m_clDevicesNodeId;
		wxTreeItemId		m_clCablesNodeId;
		wxTreeItemId		m_clConfigNodeId;
		wxTreeItemId		m_clNetworksNodeId;
		wxTreeItemId		m_clDeviceTypesNodeId;

		//
		//
		// Project Data
		//
		//

		Project				*m_pclProject = nullptr;
};

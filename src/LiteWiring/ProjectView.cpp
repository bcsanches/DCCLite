#include "ProjectView.h"

#include <wx/panel.h>

#include "Project.h"

ProjectView::ProjectView(wxWindow *parent):
	wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE)
{
	this->SetSize(parent->GetClientSize());

	m_pclTreeView = new wxTreeCtrl(this);

	m_clTreeRootId = m_pclTreeView->AddRoot("Project");	

	m_clDevicesNodeId = m_pclTreeView->AppendItem(m_clTreeRootId, "Devices");
	m_clCablesNodeId = m_pclTreeView->AppendItem(m_clTreeRootId, "Cables");
	m_clConfigNodeId = m_pclTreeView->AppendItem(m_clTreeRootId, "Config");

	m_clNetworksNodeId = m_pclTreeView->AppendItem(m_clConfigNodeId, "Networks");
	m_clDeviceTypesNodeId = m_pclTreeView->AppendItem(m_clConfigNodeId, "Device Types");

	auto *panel = new wxPanel(this);
	panel->SetBackgroundColour(*wxRED);

	this->SplitVertically(m_pclTreeView, panel, 0);

	m_pclTreeView->ExpandAll();

	Bind(wxEVT_TREE_SEL_CHANGED, &ProjectView::OnSelChanged, this, m_pclTreeView->GetId());
}

void ProjectView::SetProject(Project *project)
{
	if (m_pclProject != nullptr)
	{
		m_pclTreeView->DeleteChildren(m_clNetworksNodeId);
		m_pclTreeView->DeleteChildren(m_clDeviceTypesNodeId);
	}

	m_pclProject = project;

	if (m_pclProject)
	{
		auto networkTypes = m_pclProject->GetNetworkTypes();

		for (auto network : networkTypes)
		{
			m_pclTreeView->AppendItem(m_clNetworksNodeId, network->GetName());
		}

		auto deviceTypes = m_pclProject->GetDeviceTypes();

		for (auto deviceType : deviceTypes)
		{
			auto nodeId = m_pclTreeView->AppendItem(m_clDeviceTypesNodeId, deviceType->GetName());

			auto models = deviceType->GetDeviceModels();
			for (auto model : models)
			{
				m_pclTreeView->AppendItem(nodeId, model->GetName());
			}
		}
	}
}

void ProjectView::OnSelChanged(wxTreeEvent &event)
{
	//todo
}

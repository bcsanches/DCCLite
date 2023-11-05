// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "MessageBox.h"

#include <stdexcept>

#include "imgui.h"

namespace dcclite::panel_editor
{
	static void DisplayButton(const char *label, MessageBoxResult &resultOutput, MessageBoxResult buttonResult, ImGuiKey key)
	{
		if ((ImGui::Button(label)) ||
			((ImGui::IsKeyDown(ImGuiKey_LeftAlt) || ImGui::IsKeyDown(ImGuiKey_RightAlt)) && ImGui::IsKeyPressed(key)))
		{
			resultOutput = buttonResult;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
	}

	MessageBox::MessageBox(const char *msg, const char *caption, MessageBoxButtons buttons) :
		m_pszMsg{ msg },
		m_pszCaption{ caption },
		m_kButtons{ buttons }
	{
		if (!m_pszMsg)
			throw std::invalid_argument("[MessageBox] msg cannot be null");

		if (!m_pszCaption)
			throw std::invalid_argument("[MessageBox] caption cannot be null");
	}

	MessageBoxResult MessageBox::Display()
	{
		ImGui::OpenPopup(m_pszCaption);

		if (!ImGui::BeginPopupModal(m_pszCaption, &m_fDisplay, ImGuiWindowFlags_NoResize))
		{
			this->SetDefaultResult();

			return m_kResult;
		}

		ImGui::Text(m_pszMsg);

		switch (m_kButtons)
		{
			case MessageBoxButtons::OK:
				this->DisplayOkButton();
				break;

			case MessageBoxButtons::OK_CANCEL:
				this->DisplayOkButton();
				this->DisplayCancelButton();
				break;

			case MessageBoxButtons::YES_NO:
				this->DisplayYesButton();
				this->DisplayNoButton();
				break;

			case MessageBoxButtons::YES_NO_CANCEL:
				this->DisplayYesButton();
				this->DisplayNoButton();
				this->DisplayCancelButton();
				break;
		}

		if (ImGui::IsKeyDown(ImGuiKey_Escape))
		{
			this->SetDefaultResult();

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

		return m_kResult;
	}

	void MessageBox::DisplayOkButton()
	{
		DisplayButton("OK", m_kResult, MessageBoxResult::OK, ImGuiKey_O);
	}

	void MessageBox::DisplayCancelButton()
	{
		DisplayButton("Cancel", m_kResult, MessageBoxResult::CANCEL, ImGuiKey_C);
	}

	void MessageBox::DisplayYesButton()
	{
		DisplayButton("Yes", m_kResult, MessageBoxResult::YES, ImGuiKey_Y);
	}

	void MessageBox::DisplayNoButton()
	{
		DisplayButton("No", m_kResult, MessageBoxResult::NO, ImGuiKey_N);
	}

	void MessageBox::SetDefaultResult()
	{
		switch (m_kButtons)
		{
			case MessageBoxButtons::OK:
				m_kResult = MessageBoxResult::OK;
				break;

			case MessageBoxButtons::OK_CANCEL:
				m_kResult = MessageBoxResult::CANCEL;
				break;

			case MessageBoxButtons::YES_NO:
				m_kResult = MessageBoxResult::NO;
				break;

			case MessageBoxButtons::YES_NO_CANCEL:
				m_kResult = MessageBoxResult::CANCEL;
				break;
		}
	}
};
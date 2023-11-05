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

namespace dcclite::panel_editor
{
	enum class MessageBoxButtons
	{
		OK,
		OK_CANCEL,
		YES_NO,
		YES_NO_CANCEL
	};

	enum class MessageBoxResult
	{
		NONE,
		OK,
		CANCEL,
		YES,
		NO
	};

	class MessageBox
	{
		public:
			MessageBox(const char *msg, const char *caption, MessageBoxButtons buttons);

			MessageBoxResult Display();

		private:		
			void SetDefaultResult();

			void DisplayOkButton();
			void DisplayCancelButton();
			void DisplayYesButton();
			void DisplayNoButton();
	
		private:
			const char *m_pszMsg;
			const char *m_pszCaption;

			bool				m_fDisplay = true;

			MessageBoxButtons	m_kButtons;

			MessageBoxResult	m_kResult = MessageBoxResult::NONE;
	};
}
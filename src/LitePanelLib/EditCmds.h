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

#include <array>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "LitePanelLibDefs.h"

namespace LitePanel
{	
	class EditCmd
	{
		public:
			virtual ~EditCmd() {}

			virtual std::unique_ptr<EditCmd> Run(Panel &panel) noexcept = 0;

	};

	class InsertRailCmd: public EditCmd
	{
		public:
			InsertRailCmd(std::unique_ptr<RailObject> rail);

			std::unique_ptr<EditCmd> Run(Panel &panel) noexcept override;

		private:
			std::unique_ptr<RailObject> m_spRailObject;
	};

	class RemoveRailCmd: public EditCmd
	{
		public:
			RemoveRailCmd(const TileCoord_t &coord);

			std::unique_ptr<EditCmd> Run(Panel &panel) noexcept override;

		private:
			const TileCoord_t m_Position;
	};	
}

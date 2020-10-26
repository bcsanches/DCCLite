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

#include "MapObject.h"


namespace LitePanel
{
	namespace Gui
	{
		struct ViewInfo;
	}	

	class TempObject: public MapObject
	{
		public:
			TempObject(const TileCoord_t& position):
				MapObject(position)
			{
				//empty
			}

			virtual void Draw(const Gui::ViewInfo &viewInfo) const noexcept = 0;
	};	

	class QuadObject : public TempObject
	{
		public:
			QuadObject(const TileCoord_t& position, float r, float g, float b):
				TempObject(position)
			{
				m_fpColor[0] = r;
				m_fpColor[1] = g;
				m_fpColor[2] = b;
			}

			void Draw(const Gui::ViewInfo &viewInfo) const noexcept override;

		private:	
			float m_fpColor[3];

	};
}



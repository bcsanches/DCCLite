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
	class TempObject: public MapObject
	{
		public:
			using MapObject::MapObject;

			bool IsPersistent() const noexcept override { return false; }
	};

	class QuadObject : public TempObject
	{
		public:
			QuadObject(const TileCoord_t& position, float r, float g, float b):
				TempObject(position), m_fpColor{r, g, b} {}

			void Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const override;

		private:
			float m_fpColor[3];
	};
}

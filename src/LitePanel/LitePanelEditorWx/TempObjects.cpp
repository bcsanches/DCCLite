// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TempObjects.h"

#include <algorithm>
#include <cstdint>

#include "render/IRenderer.h"
#include "render/TileMapRenderer.h"

namespace LitePanel
{
	void QuadObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		const auto channel = [](float value)
		{
			return static_cast<uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
		};

		const auto color = Render::LP_COL32(channel(m_fpColor[0]), channel(m_fpColor[1]), channel(m_fpColor[2]), 255);
		const auto halfSize = static_cast<float>(viewInfo.m_uHalfTileSize);
		const FloatPoint_t offset{halfSize, halfSize};
		const FloatPoint_t halfExtent{halfSize * 0.8f, halfSize * 0.8f};
		renderer.DrawFilledRect(tileOrigin + offset - halfExtent, tileOrigin + offset + halfExtent, color);
	}
}

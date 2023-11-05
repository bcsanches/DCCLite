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

#include "Util.h"

#include <array>
#include <vector>

#include <SDL3/SDL.h>

namespace dcclite::panel_editor
{    
	class ConsoleWidget;

	enum KeyModifiers
	{
		KEY_MODIFIER_CTRL = 0x01,
		KEY_MODIFIER_ALT = 0x02,
		KEY_MODIFIER_SHIFT = 0x04,

		KEY_MODIFIER_ALL = KEY_MODIFIER_CTRL | KEY_MODIFIER_ALT | KEY_MODIFIER_SHIFT,

		KEY_MODIFIER_COUNT = KEY_MODIFIER_ALL + 1
	};

	typedef uint16_t KeyModifier_t;

	class KeyBindingManager
	{
		public:
			KeyBindingManager();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(KeyBindingManager);

			void Bind(const char *cmd, SDL_Scancode key, KeyModifier_t keyMod = 0);

			void HandleEvent(const SDL_KeyboardEvent &key, ConsoleWidget &console);

		private:			
			//overkill? A bit bigger... but fast and size is insignificant compared to the rest...
			std::array<std::array<std::string, KEY_MODIFIER_COUNT>, SDL_NUM_SCANCODES> m_arBindings;
	};
}

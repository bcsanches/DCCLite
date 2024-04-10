// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "KeyBindingManager.h"

#include <stdexcept>

#include <fmt/format.h>
#include <magic_enum.hpp>

#include "ConsoleWidget.h"

namespace dcclite::PanelEditor
{
    KeyBindingManager::KeyBindingManager()
    {
        //empty
    }

    void KeyBindingManager::Bind(const char *cmd, SDL_Scancode key, KeyModifier_t keyMod)
    {
        keyMod &= KEY_MODIFIER_ALL;

        auto &vec = m_arBindings[key];

        if(!vec[keyMod].empty())
        {
            throw std::invalid_argument(fmt::format("[KeyBindingManager::Bind] Binding for key {} already exists with mod {}", magic_enum::enum_name(key), keyMod));
        }        

        vec[keyMod] = cmd;
    }

    void KeyBindingManager::HandleEvent(const SDL_KeyboardEvent &key, ConsoleWidget &console)
    {
        auto &vec = m_arBindings[key.keysym.scancode];

        KeyModifier_t activeModifier = (key.keysym.mod & SDL_KMOD_CTRL) ? KEY_MODIFIER_CTRL : 0;
        activeModifier |= (key.keysym.mod & SDL_KMOD_SHIFT) ? KEY_MODIFIER_SHIFT : 0;
        activeModifier |= (key.keysym.mod & SDL_KMOD_ALT) ? KEY_MODIFIER_ALT : 0;
        
        if(!vec[activeModifier].empty())
            console.ExecuteCommand(vec[activeModifier].c_str());
    }
}

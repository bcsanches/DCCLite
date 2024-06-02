// Copyright (C) 2022 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Threading.Tasks;

namespace SharpCommon
{
    [Flags]
    public enum ServoTurnoutFlags : byte
    {
        SRVT_INVERTED_OPERATION = 0x04,
        SRVT_IGNORE_SAVED_STATE = 0x08,
        SRVT_ACTIVATE_ON_POWER_UP = 0x10,
        SRVT_INVERTED_FROG = 0x20,
        SRVT_INVERTED_POWER = 0x40,

        SRVT_POWER_ON = 0x80
    }

    public interface IServoTurnout
    {
        public string Name { get; }

        public ServoTurnoutFlags Flags { get; }

        public uint StartPos { get; }
        public uint EndPos { get; }

        public uint MsOperationTime { get; }

        public Task ActivateAsync();
        public Task DeactivateAsync();
        public Task FlipAsync();
    }
}

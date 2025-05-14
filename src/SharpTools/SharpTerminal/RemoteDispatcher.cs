// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System.Json;
using System.Runtime.Versioning;

namespace SharpTerminal.Dispatcher
{
	[SupportedOSPlatform("windows")]
	public class RemoteDispatcher : RemoteObject
    {             
        public RemoteDispatcher(string name, string className, string path, ulong internalId, RemoteFolder parent) :
            base(name, className, path, internalId, parent)
        {            
            //empty
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);
        }
    }

    public enum SectionStates
    {
        CLEAR = 0,
        UP_START = 1,
        UP = 2,
        DOWN_START = 3,
        DOWN = 4
    }

	[SupportedOSPlatform("windows")]
	public class RemoteSection : RemoteObject
    {
        protected static IRemoteObjectAction g_ResetAction = new RemoteObjectCmdAction("Reset-Item", "Reset", "Reset the section");

        public SectionStates m_kState;                

        public RemoteSection(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, parent)
        {
            this.ParseStateData(objectDef);           
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            this.ParseStateData(objectDef);
        }

        private void ParseStateData(JsonValue objectDef)
        {
            m_kState = (SectionStates)(int)objectDef["state"];            
        }

        public override string TryGetIconName()
        {
            switch(m_kState)
            {
                case SectionStates.CLEAR:
                    return DefaultIcons.EMPTY_CIRCLE;

                case SectionStates.UP:
                    return DefaultIcons.UP_ARROW_ICON;

                case SectionStates.UP_START:
                    return DefaultIcons.UP_EMPTY_ARROW_ICON;

                case SectionStates.DOWN:
                    return DefaultIcons.DOWN_ARROW_ICON;

                case SectionStates.DOWN_START:
                    return DefaultIcons.DOWN_EMPTY_ARROW_ICON;

                default:
                    return null;
            }            
        }
      
        public override IRemoteObjectAction[] GetActions()
        {
            return [g_ResetAction];
        }
    }  
}

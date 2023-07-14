using System;
using System.Json;
using System.ComponentModel;

namespace SharpTerminal.Dispatcher
{    
    public class RemoteDispatcher : RemoteObject
    {             
        public RemoteDispatcher(string name, string className, string path, ulong internalId, ulong parentInternalId) :
            base(name, className, path, internalId, parentInternalId)
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

    public class RemoteSection : RemoteObject
    {
        protected static IRemoteObjectAction g_ResetAction = new RemoteDecoderCmdAction("Reset", "Reset", "Reset the section");

        public SectionStates m_kState;                

        public RemoteSection(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId)
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
            return new IRemoteObjectAction[1] { g_ResetAction };
        }
    }  
}

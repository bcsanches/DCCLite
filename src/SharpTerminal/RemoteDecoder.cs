using System;
using System.Json;
using System.ComponentModel;


namespace SharpTerminal
{
    public abstract class RemoteDecoderCmdBaseAction: IRemoteObjectAction
    {
        private string mDescription;
        private string mLabel;

        public RemoteDecoderCmdBaseAction(string label, string description)
        {
            mDescription = description;
            mLabel = label ?? throw new System.ArgumentNullException(nameof(label));            
        }

        public abstract void Execute(IConsole console, RemoteObject target);

        public string GetDescription()
        {
            return mDescription;
        }

        public string GetLabel()
        {
            return mLabel;
        }
    }

    public class RemoteDecoderCmdAction : RemoteDecoderCmdBaseAction
    {        
        private string mCmd;

        public RemoteDecoderCmdAction(string cmd, string label, string description):
            base(label, description)
        {            
            mCmd = cmd ?? throw new System.ArgumentNullException(nameof(cmd));
        }

        public override void Execute(IConsole console, RemoteObject target)
        {
            var decoder = (RemoteDecoder)target;

            string[] args = new string[3] { mCmd, decoder.SystemName, decoder.Name };
            console.ProcessCmd(args);
        }        
    }


    public class RemoteDecoder : RemoteObject
    {
        protected static IRemoteObjectAction g_FlipAction = new RemoteDecoderCmdAction("Flip-Item", "Flip", "Activate / deactivate the item");

        public int Address { get; }
        public string LocationHint { get; }

        public string SystemName { get; }

        private bool mBroken;
        
        private bool mRequestedState;
        private bool mRemoteState;

        public bool Broken
        {
            get { return mBroken; }
            set
            {
                this.UpdateProperty(ref mBroken, value);
            }
        }

        [Category("RemoteFlags")]
        public bool RequestedState
        {
            get { return mRequestedState; }

            set
            {
                this.UpdateProperty(ref mRequestedState, value);
            }
        }

        [Category("RemoteFlags")]
        public bool RemoteState 
        { 
            get { return mRemoteState; }

            set
            {
                this.UpdateProperty(ref mRemoteState, value);
            }
        }

        public RemoteDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId)
        {
            DeviceName = objectDef["deviceName"];

            Address = objectDef["address"];

            if (objectDef.ContainsKey("locationHint"))
                LocationHint = objectDef["locationHint"];

            SystemName = objectDef["systemName"];

            this.ParseState(objectDef);
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            this.ParseState(objectDef);
        }

        private void ParseState(JsonValue objectDef)
        {                                    
            //Only remote decoders supports broken
            if (objectDef.ContainsKey("broken"))
                Broken = objectDef["broken"];

            if (objectDef.ContainsKey("remoteActive"))
                RemoteState = objectDef["remoteActive"];

            if (objectDef.ContainsKey("requestedState"))
                RequestedState = objectDef["requestedState"];
        }

        public string DeviceName { get; }
    }

    public class RemoteSignalDecoder : RemoteDecoder
    {
        public RemoteSignalDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {

        }

        public override string TryGetIconName()
        {
            return DefaultIcons.SIGNAL_ICON;
        }
    }

    public class RemoteSensorDecoder: RemoteDecoder
    {
        public RemoteSensorDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {

        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.SENSOR_ON_ICON : DefaultIcons.SENSOR_OFF_ICON;
        }
    }

    public class RemoteOutputDecoder : RemoteDecoder
    {        
        public RemoteOutputDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {

        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.LAMP_ON_ICON : DefaultIcons.LAMP_OFF_ICON;
        }

        public override IRemoteObjectAction[] GetActions()
        {
            return new IRemoteObjectAction[1]{ g_FlipAction };
        }
    }

    [Flags]
    public enum ServoTurnoutFlags: byte
    {
        SRVT_INVERTED_OPERATION = 0x04,
        SRVT_IGNORE_SAVED_STATE = 0x08,
        SRVT_ACTIVATE_ON_POWER_UP = 0x10,
        SRVT_INVERTED_FROG = 0x20,
        SRVT_INVERTED_POWER = 0x40,

        SRVT_POWER_ON = 0x80
    }

    public class RemoteTurnoutDecoder: RemoteDecoder
    {
        protected static IRemoteObjectAction gProgrammerAction = new ServoProgrammerAction("Program", "Program the turnout servo");

        public bool m_fInvertedOperation;
        public bool m_fIgnoreSaveState;
        public bool m_fActivateOnPowerUp;
        public bool m_fInvertedFrog;
        public bool m_fInvertedPower;

        public uint m_iStartPos;
        public uint m_iEndPos;
        public uint m_msOperationTime;

        public ServoTurnoutFlags m_fFlags;

        [Category("Flags")]
        public ServoTurnoutFlags Flags 
        { 
            get { return m_fFlags; }
            private set
            {
                this.UpdateProperty(ref m_fFlags, value);

                InvertedOperation = (m_fFlags & ServoTurnoutFlags.SRVT_INVERTED_OPERATION) != 0;
                IgnoreSaveState = (m_fFlags & ServoTurnoutFlags.SRVT_IGNORE_SAVED_STATE) != 0;
                ActivateOnPowerUp = (m_fFlags & ServoTurnoutFlags.SRVT_ACTIVATE_ON_POWER_UP) != 0;
                InvertedFrog = (m_fFlags & ServoTurnoutFlags.SRVT_INVERTED_FROG) != 0;
                InvertedPower = (m_fFlags & ServoTurnoutFlags.SRVT_INVERTED_POWER) != 0;
            }
        }

        [Category("Flags")]
        public bool InvertedOperation 
        { 
            get { return m_fInvertedOperation; }

            private set
            {
                this.UpdateProperty(ref m_fInvertedOperation, value);
            }
        }

        [Category("Flags")]
        public bool IgnoreSaveState 
        { 
            get { return m_fIgnoreSaveState; }

            private set
            {
                this.UpdateProperty(ref m_fIgnoreSaveState, value);
            }
        }

        [Category("Flags")]
        public bool ActivateOnPowerUp 
        { 
            get { return m_fActivateOnPowerUp; }

            private set
            {
                this.UpdateProperty(ref m_fActivateOnPowerUp, value);
            }
        }

        [Category("Flags")]
        public bool InvertedFrog 
        { 
            get { return m_fInvertedFrog; }

            private set
            {
                this.UpdateProperty(ref m_fInvertedFrog, value);
            }
        }

        [Category("Flags")]
        public bool InvertedPower 
        { 
            get { return m_fInvertedPower; }

            private set
            {
                this.UpdateProperty(ref m_fInvertedPower, value);
            }

        }

        [Category("Servo")]
        public uint StartPos 
        { 
            get { return m_iStartPos; } 

            private set
            {
                this.UpdateProperty(ref m_iStartPos, value);
            }
        }

        [Category("Servo")]
        public uint EndPos 
        { 
            get { return m_iEndPos; } 
        
            private set
            {
                this.UpdateProperty(ref m_iEndPos, value);
            }
        }

        [Category("Servo")]
        public uint MsOperationTime 
        { 
            get { return m_msOperationTime; } 
        
            private set { this.UpdateProperty(ref m_msOperationTime, value); }
        }


        public RemoteTurnoutDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {
            this.ParseState(objectDef);
        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.TURNOUT_ON_ICON : DefaultIcons.TURNOUT_OFF_ICON;
        }

        public override IRemoteObjectAction[] GetActions()
        {
            return new IRemoteObjectAction[2] { g_FlipAction, gProgrammerAction };
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            this.ParseState(objectDef);
        }

        private void ParseState(JsonValue objectDef)
        {
            Flags = (ServoTurnoutFlags)(int)objectDef["flags"];                        

            StartPos = objectDef["startPos"];
            EndPos = objectDef["endPos"];
            MsOperationTime = objectDef["msOperationTime"];
        }
    }
}

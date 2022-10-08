using System;
using System.Json;
using System.ComponentModel;
using System.Windows.Forms;

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
        private readonly string mCmd;
        private readonly string[] mExtraParams;

        public RemoteDecoderCmdAction(string cmd, string label, string description, params string[] extraParams):
            base(label, description)
        {            
            mCmd = cmd ?? throw new System.ArgumentNullException(nameof(cmd));

            mExtraParams = extraParams;
        }

        public override void Execute(IConsole console, RemoteObject target)
        {
            var decoder = (RemoteDecoder)target;
            
            string[] args = new string[3 + mExtraParams.Length];
            args[0] = mCmd;
            args[1] = decoder.SystemName;
            args[2] = decoder.Name;

            for (var i = 0; i < mExtraParams.Length; i++)
                args[3 + i] = mExtraParams[i];            

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
        public String []m_Aspects;
        public string m_strCurrentAspect;
        public string m_strRequestedAspect;

        IRemoteObjectAction []m_arActions;

        public RemoteSignalDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {
            this.ParseStateData(objectDef);

            var aspectsData = objectDef["aspects"];

            m_Aspects = new string[aspectsData.Count];
            for (var i = 0; i < m_Aspects.Length; ++i)
                m_Aspects[i] = aspectsData[i];
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            this.ParseStateData(objectDef);
        }

        private void ParseStateData(JsonValue objectDef)
        {
            CurrentAspect = (String)objectDef["currentAspectName"];
            RequestedAspect = (String)objectDef["requestedAspectName"];
        }

        public override string TryGetIconName()
        {
            return DefaultIcons.SIGNAL_ICON;
        }

        [Category("Aspect")]
        public string CurrentAspect
        {
            get { return m_strCurrentAspect; }

            set
            {
                this.UpdateProperty(ref m_strCurrentAspect, value);
            }
        }

        [Category("Aspect")]
        public string RequestedAspect
        {
            get { return m_strRequestedAspect; }

            set
            {
                this.UpdateProperty(ref m_strRequestedAspect, value);
            }
        }

        public override IRemoteObjectAction[] GetActions()
        {
            if(m_arActions == null)
            {
                m_arActions = new IRemoteObjectAction[m_Aspects.Length];

                for(int i = 0; i < m_Aspects.Length; i++)
                {
                    var aspect = m_Aspects[i];
                    m_arActions[i] = new RemoteDecoderCmdAction("Set-Aspect", aspect, "Set " + aspect + " aspect", aspect);
                }
            }

            return m_arActions;
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

        public SharpCommon.ServoTurnoutFlags m_fFlags;

        [Category("Flags")]
        public SharpCommon.ServoTurnoutFlags Flags 
        { 
            get { return m_fFlags; }
            private set
            {
                this.UpdateProperty(ref m_fFlags, value);

                InvertedOperation = (m_fFlags & SharpCommon.ServoTurnoutFlags.SRVT_INVERTED_OPERATION) != 0;
                IgnoreSaveState = (m_fFlags & SharpCommon.ServoTurnoutFlags.SRVT_IGNORE_SAVED_STATE) != 0;
                ActivateOnPowerUp = (m_fFlags & SharpCommon.ServoTurnoutFlags.SRVT_ACTIVATE_ON_POWER_UP) != 0;
                InvertedFrog = (m_fFlags & SharpCommon.ServoTurnoutFlags.SRVT_INVERTED_FROG) != 0;
                InvertedPower = (m_fFlags & SharpCommon.ServoTurnoutFlags.SRVT_INVERTED_POWER) != 0;
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
            Flags = (SharpCommon.ServoTurnoutFlags)(int)objectDef["flags"];                        

            StartPos = objectDef["startPos"];
            EndPos = objectDef["endPos"];
            MsOperationTime = objectDef["msOperationTime"];
        }
    }
}

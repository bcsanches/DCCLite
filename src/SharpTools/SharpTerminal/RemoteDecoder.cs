// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Json;
using System.ComponentModel;
using System.Runtime.Versioning;

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

	[SupportedOSPlatform("windows")]
	public class RemoteServiceObjectCmdAction : RemoteDecoderCmdBaseAction
    {        
        private readonly string mCmd;
        private readonly string[] mExtraParams;

        public RemoteServiceObjectCmdAction(string cmd, string label, string description, params string[] extraParams):
            base(label, description)
        {            
            mCmd = cmd ?? throw new System.ArgumentNullException(nameof(cmd));

            mExtraParams = extraParams;
        }

        public override void Execute(IConsole console, RemoteObject target)
        {
            var decoder = (RemoteServiceObject)target;
            
            string[] args = new string[3 + mExtraParams.Length];
            args[0] = mCmd;
            args[1] = decoder.SystemName;
            args[2] = decoder.Name;

            for (var i = 0; i < mExtraParams.Length; i++)
                args[3 + i] = mExtraParams[i];            

            console.ProcessCmd(args);
        }        
    }

	[SupportedOSPlatform("windows")]
	public class RemoteServiceObject: RemoteObject
    {
        public string SystemName { get; }

        public RemoteServiceObject(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
          base(name, className, path, internalId, parent)
        {
            SystemName = objectDef["systemName"];
        }
    }

	[SupportedOSPlatform("windows")]
	public class RemoteDecoder : RemoteServiceObject
    {
        protected static IRemoteObjectAction g_FlipAction = new RemoteServiceObjectCmdAction("Flip-Item", "Flip", "Activate / deactivate the item");

        public int Address { get; }
        public string LocationHint { get; }

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

        public RemoteDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
        {
            DeviceName = objectDef["deviceName"];

            Address = objectDef["address"];

            if (objectDef.ContainsKey("locationHint"))
                LocationHint = objectDef["locationHint"];            

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

            if (objectDef.ContainsKey("active"))
                RemoteState = objectDef["active"];

            if (objectDef.ContainsKey("requestedState"))
                RequestedState = objectDef["requestedState"];
        }

        public string DeviceName { get; }
    }	

	[SupportedOSPlatform("windows")]
	public class VirtualSensorDecoder : RemoteDecoder
    {       
        public VirtualSensorDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
        {
            //empty
        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.SENSOR_ON_ICON : DefaultIcons.SENSOR_OFF_ICON;
        }
    }

	[SupportedOSPlatform("windows")]
	public class RemoteSensorDecoder: RemoteDecoder
    {
        private bool mfPullUp;
        private bool mfInverted;

        private uint mActivateDelay;
        private uint mDeactivateDelay;

        [Category("Sensor")]
        public uint ActivateDelay
        {
            get { return mActivateDelay; }            
        }

        [Category("Sensor")]
        public uint DeactivateDelay
        {
            get { return mDeactivateDelay; }
        }

        [Category("Sensor")]
        public bool PullUp
        {
            get { return mfPullUp; }
        }

        [Category("Sensor")]
        public bool Inverted
        {
            get { return mfInverted; }
        }
		
		public RemoteSensorDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
        {
            this.ParseStateData(objectDef);
        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.SENSOR_ON_ICON : DefaultIcons.SENSOR_OFF_ICON;
        }

        private void ParseStateData(JsonValue objectDef)
        {
            mActivateDelay = (uint)objectDef["activateDelay"];
            mDeactivateDelay = (uint)objectDef["deactivateDelay"];

            mfPullUp = (bool)objectDef["pullUp"];
            mfInverted = (bool)objectDef["inverted"];
        }
    }

	[SupportedOSPlatform("windows")]
	public class RemoteOutputDecoder : RemoteDecoder
    {        
        public RemoteOutputDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
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

	[SupportedOSPlatform("windows")]
	public class RemoteTurnoutDecoder: RemoteDecoder
    {                
        public RemoteTurnoutDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
        {
            //empty
        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.TURNOUT_ON_ICON : DefaultIcons.TURNOUT_OFF_ICON;
        }

        public override IRemoteObjectAction[] GetActions()
        {
            return new IRemoteObjectAction[1] { g_FlipAction };
        }
    }

	[SupportedOSPlatform("windows")]
	public class RemoteServoTurnoutDecoder: RemoteTurnoutDecoder
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


        public RemoteServoTurnoutDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
            base(name, className, path, internalId, objectDef, parent)
        {
            this.ParseState(objectDef);
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

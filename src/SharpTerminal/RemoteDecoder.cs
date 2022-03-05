using System.Json;


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

        public bool RequestedState
        {
            get { return mRequestedState; }

            set
            {
                this.UpdateProperty(ref mRequestedState, value);
            }
        }

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

    public class RemoteTurnoutDecoder: RemoteDecoder
    {
        protected static IRemoteObjectAction gProgrammerAction = new ServoProgrammerAction("Program", "Program the turnout servo");

        public RemoteTurnoutDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {

        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.TURNOUT_ON_ICON : DefaultIcons.TURNOUT_OFF_ICON;
        }

        public override IRemoteObjectAction[] GetActions()
        {
            return new IRemoteObjectAction[2] { g_FlipAction, gProgrammerAction };
        }
    }
}

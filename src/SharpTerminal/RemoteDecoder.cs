using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public class RemoteDecoder : RemoteObject
    {
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
    }

    public class RemoteTurnoutDecoder: RemoteDecoder
    {
        public RemoteTurnoutDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef) :
            base(name, className, path, internalId, parentInternalId, objectDef)
        {

        }

        public override string TryGetIconName()
        {
            return RemoteState ? DefaultIcons.TURNOUT_ON_ICON : DefaultIcons.TURNOUT_OFF_ICON;
        }
    }
}

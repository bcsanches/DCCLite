using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemotePin
    {
        public String Decoder { get; set; }
        public int? DecoderAddress { get; set; }
        public bool? DecoderBroken { get; set; }
        public String Usage { get; set; }
        public String SpecialName { get; set; }                      
        

        public RemotePin(string decoder, int? decoderAddress, bool? decoderBroken, string usage, string specialName)
        {
            Decoder = decoder;
            DecoderAddress = decoderAddress;
            DecoderBroken = decoderBroken;
            Usage = usage;
            SpecialName = specialName;
        }
    }

    public class RemoteDevice: RemoteFolder
    {           
        public enum Status
        {
            OFFLINE,
            CONNECTING,
            ONLINE
        }

        public Status ConnectionStatus { get; set; }

        private int mFreeRam;
        public int FreeRam
        {
            get { return mFreeRam; }

            set
            {
                this.UpdateProperty(ref mFreeRam, value);
            }
        }

        public RemotePin[] Pins;

        public RemoteDevice(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            this.UpdateState(objectDef);
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            ConnectionStatus = (Status)(int)objectDef["connectionStatus"];
            FreeRam = (int)objectDef["freeRam"];

            if (!objectDef.ContainsKey("pins"))
                return;

            var pinsData = objectDef["pins"];            

            if(Pins == null)
            {
                Pins = new RemotePin[pinsData.Count];
            }
            else if(Pins.Length < pinsData.Count)
            {                
                Pins = new RemotePin[pinsData.Count];
            }

            for(int i = 0;i < pinsData.Count; ++i)
            {
                var pinDef = pinsData[i];

                string decoder = null;
                int? decoderAddress = null;
                bool? decoderBroken = null;
                string usage = null;

                if(pinDef.ContainsKey("decoder"))
                {
                    decoder = pinDef["decoder"];
                    decoderAddress = pinDef["decoderAddress"];
                    usage = pinDef["usage"];
                    decoderBroken = pinDef["decoderBroken"];
                }
                
                string specialName = pinDef.ContainsKey("specialName") ? pinDef["specialName"] : null;

                Pins[i] = new RemotePin(decoder, decoderAddress, decoderBroken, usage, specialName);
            }
        }

        public override string TryGetIconName()
        {
            if (ConnectionStatus == Status.ONLINE)
                return DefaultIcons.CONNECTED_DRIVE_ICON;
            else if (ConnectionStatus == Status.CONNECTING)
                return DefaultIcons.CONNECTING_DRIVE_ICON;
            else
                return DefaultIcons.DISCONNECTED_DRIVE_ICON;            
        }

        public override Control CreateControl(IConsole console)
        {
            return new RemoteDeviceUserControl(this, Pins);
        }

        public override string GetNameSuffix()
        {
            return " [" + mFreeRam.ToString() + "]";
        }
    }
}

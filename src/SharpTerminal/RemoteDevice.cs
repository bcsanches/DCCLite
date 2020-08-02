using System;
using System.Collections.Generic;
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
        public String Usage { get; set; }
        public String SpecialName { get; set; }

        

        public RemotePin(string decoder, int? decoderAddress, string usage, string specialName)
        {
            Decoder = decoder;
            DecoderAddress = decoderAddress;
            Usage = usage;
            SpecialName = specialName;
        }
    }

    public class RemoteDevice: RemoteFolder
    {           
        public bool Connected { get; set; }

        public RemotePin[] Pins;

        public RemoteDevice(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            this.UpdateState(objectDef);
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            Connected = objectDef["connected"];

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
                string usage = null;

                if(pinDef.ContainsKey("decoder"))
                {
                    decoder = pinDef["decoder"];
                    decoderAddress = pinDef["decoderAddress"];
                    usage = pinDef["usage"];
                }
                
                string specialName = pinDef.ContainsKey("specialName") ? pinDef["specialName"] : null;

                Pins[i] = new RemotePin(decoder, decoderAddress, usage, specialName);
            }
        }

        public override string TryGetIconName()
        {
            return Connected ? DefaultIcons.CONNECTED_DRIVE_ICON : DefaultIcons.DISCONNECTED_DRIVE_ICON;
        }

        public override Control CreateControl()
        {
            return new RemoteDeviceUserControl(this, Pins);
        }
    }
}

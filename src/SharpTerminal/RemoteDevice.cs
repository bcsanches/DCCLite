using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteDevice: RemoteFolder
    {           
        public bool Connected { get; set; }

        public RemoteDevice(string name, string className, string path, int internalId, JsonValue objectDef):
            base(name, className, path, internalId)
        {
            this.UpdateState(objectDef);
        }

        public override void UpdateState(JsonValue objectDef)
        {
            Connected = objectDef["connected"];
        }

        public override string TryGetIconName()
        {
            return DefaultIcons.DISCONNECTED_DRIVE_ICON;
        }
    }
}

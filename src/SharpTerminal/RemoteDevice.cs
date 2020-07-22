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

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            Connected = objectDef["connected"];
        }

        public override string TryGetIconName()
        {
            return Connected ? DefaultIcons.CONNECTED_DRIVE_ICON : DefaultIcons.DISCONNECTED_DRIVE_ICON;
        }
    }
}

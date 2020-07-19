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
        public RemoteDevice(string name, string className, string path, int internalId):
            base(name, className, path, internalId)
        {
            //empty
        }

        public override void UpdateState(JsonValue def)
        {
            //nothing to do
        }             
    }
}

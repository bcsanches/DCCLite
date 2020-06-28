using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteLocationManagerService: RemoteObject
    {           
        public RemoteLocationManagerService(string name, string className, string path, int internalId, Flags flags):
            base(name, className, path, internalId, flags)
        {
            //empty
        }

        public override void UpdateState(JsonValue def)
        {
            //nothing to do
        }             
    }
}

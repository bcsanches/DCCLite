using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteDecoder: RemoteObject
    {           
        public RemoteDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId):
            base(name, className, path, internalId, parentInternalId)
        {
            //empty
        }           
    }
}

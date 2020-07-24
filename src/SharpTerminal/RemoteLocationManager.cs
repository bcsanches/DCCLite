using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteLocationManager: RemoteObject
    {           
        public RemoteLocationManager(string name, string className, string path, ulong internalId, ulong parentInternalId):
            base(name, className, path, internalId, parentInternalId)
        {
            //empty
        }           
    }

    public class RemoteLocation: RemoteObject
    {
        int mBeginAddress;
        int mEndAddress;

        public RemoteLocation(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue data) :
            base(name, className, path, internalId, parentInternalId)
        {
            mBeginAddress = data["begin"];
            mEndAddress = data["end"];
        }

        public override Control CreateControl()
        {
            return new RemoteLocationUserControl(mBeginAddress, mEndAddress);
        }
    }
}

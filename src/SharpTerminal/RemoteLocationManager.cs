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

        RemoteObject[] mRemoteObjects;

        public RemoteLocation(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue data) :
            base(name, className, path, internalId, parentInternalId)
        {
            mBeginAddress = data["begin"];
            mEndAddress = data["end"];

            if (!data.ContainsKey("decoders"))
                return;

            var decoders = data["decoders"];
            var count = decoders.Count;

            mRemoteObjects = new RemoteObject[count];

            for (int i = 0 ;i < count; ++i)
            {
                var decInfo = decoders[i];
                if (decInfo == null)
                    continue;

                var remoteDecoder = RemoteObjectManager.LoadObject(decInfo);

                mRemoteObjects[i] = remoteDecoder;
            }
        }

        public override Control CreateControl()
        {
            return new RemoteLocationUserControl(mBeginAddress, mEndAddress);
        }
    }
}

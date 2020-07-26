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
        readonly int mBeginAddress;
        readonly int mEndAddress;
        readonly RemoteDecoder[] mRemoteDecoders;

        public RemoteLocation(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue data) :
            base(name, className, path, internalId, parentInternalId)
        {
            mBeginAddress = data["begin"];
            mEndAddress = data["end"];

            if (!data.ContainsKey("decoders"))
                return;

            var decoders = data["decoders"];
            var count = decoders.Count;

            mRemoteDecoders = new RemoteDecoder[count];

            for (int i = 0 ;i < count; ++i)
            {
                var decInfo = decoders[i];
                if (decInfo == null)
                    continue;

                var remoteObject = RemoteObjectManager.LoadObject(decInfo);                

                mRemoteDecoders[i] = remoteObject as RemoteDecoder;
            }
        }

        public override Control CreateControl()
        {
            return new RemoteLocationUserControl(this.Name, mBeginAddress, mEndAddress, mRemoteDecoders);
        }
    }
}

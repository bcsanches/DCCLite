using System;
using System.Collections.Generic;
using System.Json;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public struct LocationMismatch
    {
        public RemoteDecoder Decoder { get; }
        public String Reason { get; }
        public String MappedLocation { get; }

        public LocationMismatch(RemoteDecoder decoder, String reason, String mappedLocation)
        {
            if (string.IsNullOrWhiteSpace(reason))
                throw new ArgumentNullException(nameof(reason));

            Decoder = decoder ?? throw new ArgumentNullException(nameof(decoder));
            Reason = reason;
            MappedLocation = mappedLocation;
        }
    }

    public class RemoteLocationManager: RemoteFolder
    {
        readonly LocationMismatch[] mMismatches;

        public RemoteLocationManager(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            if (!objectDef.ContainsKey("mismatches"))
                return;

            var mismatches = objectDef["mismatches"];

            mMismatches = new LocationMismatch[mismatches.Count];

            int index = 0;
            foreach (JsonValue item in mismatches)
            {
                var remoteDecoder = (RemoteDecoder)RemoteObjectManager.LoadObject(item["decoder"]);
                var reason = item["reason"];

                String mappedLocation = item.ContainsKey("location") ? item["location"] : null;                

                var mismatch = new LocationMismatch(remoteDecoder, reason, mappedLocation);
                mMismatches[index++] = mismatch;
            }
        }

        public override Control CreateControl()
        {
            return new RemoteLocationManagerUserControl(mMismatches);
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

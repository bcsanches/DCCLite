// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Json;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
    public readonly struct LocationMismatch
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

	[SupportedOSPlatform("windows")]
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

        public override Control CreateControl(IConsole console)
        {
            return new RemoteLocationManagerUserControl(mMismatches);
        }
    }

	[SupportedOSPlatform("windows")]
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

        public override Control CreateControl(IConsole console)
        {
            return new RemoteLocationUserControl(this.Name, mBeginAddress, mEndAddress, mRemoteDecoders);
        }
    }
}

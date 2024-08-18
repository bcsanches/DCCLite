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
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public class LocationMismatch
    {
        private RemoteDecoder mDecoder;
        public String Reason { get; }
        public String MappedLocation { get; }

        private readonly String mRemoteDecoderPath;

        public LocationMismatch(String remoteDecoderPath, String reason, String mappedLocation)
        {
            if (string.IsNullOrWhiteSpace(reason))
                throw new ArgumentNullException(nameof(reason));

            //Decoder = decoder ?? throw new ArgumentNullException(nameof(decoder));
            mRemoteDecoderPath = remoteDecoderPath;
			Reason = reason;
            MappedLocation = mappedLocation;
        }

		[SupportedOSPlatform("windows")]
		public async Task<RemoteDecoder> LoadDecoderAsync()
        {
			mDecoder ??= (RemoteDecoder) await RemoteObjectManager.GetRemoteObjectAsync(mRemoteDecoderPath);

            return mDecoder;
		}
    }

	[SupportedOSPlatform("windows")]
	public class RemoteLocationManager: RemoteFolder
    {
        readonly LocationMismatch[] mMismatches;

        public RemoteLocationManager(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent):
            base(name, className, path, internalId, parent)
        {
            if (!objectDef.ContainsKey("mismatches"))
                return;

            var mismatches = objectDef["mismatches"];

            mMismatches = new LocationMismatch[mismatches.Count];

            int index = 0;
            foreach (JsonValue item in mismatches)
            {                
                var reason = item["reason"];

                String mappedLocation = item.ContainsKey("location") ? item["location"] : null;                

                var mismatch = new LocationMismatch(item["decoderPath"], reason, mappedLocation);
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
        readonly private string[] mDecodersPath;

        public RemoteLocation(string name, string className, string path, ulong internalId, JsonValue data, RemoteFolder parent) :
            base(name, className, path, internalId, parent)
        {
            mBeginAddress = data["begin"];
            mEndAddress = data["end"];

            if (!data.ContainsKey("decodersPath"))
                return;

            var decodersPath = data["decodersPath"];
			var count = decodersPath.Count;

			if (count == 0)
                return;            

			mDecodersPath = new string[mEndAddress - mBeginAddress];

            for (int i = 0 ;i < count; ++i)
            {
                var decInfo = decodersPath[i];

                mDecodersPath[decInfo["index"]] = decInfo["path"];
            }
        }

        public override Control CreateControl(IConsole console)
        {
            return new RemoteLocationUserControl(this.Name, mBeginAddress, mEndAddress, mDecodersPath);
        }
    }
}

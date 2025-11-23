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
using System.Collections.Generic;
using System.Json;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemotePin
    {
        public String Decoder { get; set; }
        public int? DecoderAddress { get; set; }
        public bool? DecoderBroken { get; set; }
        public String Usage { get; set; }
        public String SpecialName { get; set; }                      
        

        public RemotePin(string decoder, int? decoderAddress, bool? decoderBroken, string usage, string specialName)
        {
            Decoder = decoder;
            DecoderAddress = decoderAddress;
            DecoderBroken = decoderBroken;
            Usage = usage;
            SpecialName = specialName;
        }
    }

    public class EventLog
    {
        public string EventType { get; set; }
        public DateTime DateTime { get; set; }

        public string Message { get; set; }
	}

	[SupportedOSPlatform("windows")]
	public class RemoteNetworkDevice: RemoteFolder
    {           
        public enum Status
        {
            OFFLINE,
            CONNECTING,
            ONLINE
        }

        public Status ConnectionStatus { get; set; }

        private int mFreeRam;
        public int FreeRam
        {
            get { return mFreeRam; }

            set
            {
                this.UpdateProperty(ref mFreeRam, value);
            }
        }

        public uint ProtocolVersion { get; set; }

        public bool Registered { get; set; }

        public RemotePin[] Pins;

        public List<EventLog> EventsLog = [];

        public RemoteNetworkDevice(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent):
            base(name, className, path, internalId, parent)
        {
            this.LoadState(objectDef);
        }

        private void LoadPinsData(JsonValue pinsData)
        {			
			if (Pins == null)
			{
				Pins = new RemotePin[pinsData.Count];
			}
			else if (Pins.Length < pinsData.Count)
			{
				Pins = new RemotePin[pinsData.Count];
			}

			for (int i = 0; i < pinsData.Count; ++i)
			{
				var pinDef = pinsData[i];

				string decoder = null;
				int? decoderAddress = null;
				bool? decoderBroken = null;
				string usage = null;

				if (pinDef.ContainsKey("decoder"))
				{
					decoder = pinDef["decoder"];
					decoderAddress = pinDef["decoderAddress"];
					usage = pinDef["usage"];
					decoderBroken = pinDef["decoderBroken"];
				}

				string specialName = pinDef.ContainsKey("specialName") ? pinDef["specialName"] : null;

				Pins[i] = new RemotePin(decoder, decoderAddress, decoderBroken, usage, specialName);
			}
		}

        private void LoadEventsLog(JsonValue eventsLogData)
        {
            for (int i = 0; i < eventsLogData.Count; ++i)
            {
                var eventDef = eventsLogData[i];
                string eventType = eventDef["type"];
                DateTime dateTime = DateTime.Parse(eventDef["time"]);

                //stupid chrono on CPP side uses UTC.... fix it
                dateTime = DateTime.SpecifyKind(dateTime, DateTimeKind.Utc).ToLocalTime();

				string message = eventDef["info"];

                EventsLog.Add(new EventLog()
                {
                    EventType = eventType,
                    DateTime = dateTime,
                    Message = message
                });
            }

            if(eventsLogData.Count > 0)
            {
				OnPropertyUpdated(nameof(EventsLog));
			}
		}

        private void LoadState(JsonValue objectDef)
        {
            //those fields are not expected to change...
			Registered = (bool)objectDef["registered"];
			ProtocolVersion = (uint)objectDef["protocolVersion"];

            this.OnUpdateState(objectDef);
		}

		protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            if (objectDef.ContainsKey("connectionStatus"))
				ConnectionStatus = (Status)(int)objectDef["connectionStatus"];

            if(objectDef.ContainsKey("freeRam"))
				FreeRam = (int)objectDef["freeRam"];            

            if (objectDef.ContainsKey("pins"))
            {
                this.LoadPinsData(objectDef["pins"]);
			}

            if(objectDef.ContainsKey("eventsLog"))
            {
                this.LoadEventsLog(objectDef["eventsLog"]);
			}
		}

        public override string TryGetIconName()
        {
            if (ConnectionStatus == Status.ONLINE)
                return DefaultIcons.CONNECTED_DRIVE_ICON;
            else if (ConnectionStatus == Status.CONNECTING)
                return DefaultIcons.CONNECTING_DRIVE_ICON;
            else
                return DefaultIcons.DISCONNECTED_DRIVE_ICON;            
        }

        public override Control CreateControl(IConsole console)
        {
            return new RemoteDeviceUserControl(console, this, Pins);
        }

        public override string GetNameSuffix()
        {
            return " [" + mFreeRam.ToString() + "]";
        }
    }
}

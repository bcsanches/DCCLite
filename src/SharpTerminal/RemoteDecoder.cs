﻿using System;
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
        public int Address { get; }
        public string LocationHint { get; }

        public RemoteDecoder(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            DeviceName = objectDef["deviceName"];

            Address = objectDef["address"];

            if(objectDef.ContainsKey("locationHint"))
                LocationHint = objectDef["locationHint"];
        }           

        public string DeviceName { get; }
    }
}
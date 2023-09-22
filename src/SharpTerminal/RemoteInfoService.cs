using System;
using System.Json;
using System.ComponentModel;

namespace SharpTerminal
{    
    public class RemoveInfoService : RemoteObject
    {             
        public RemoveInfoService(string name, string className, string path, ulong internalId, ulong parentInternalId) :
            base(name, className, path, internalId, parentInternalId)
        {            
            //empty
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);
        }       
    }  
}

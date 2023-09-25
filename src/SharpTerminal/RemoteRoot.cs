
using System.Json;
using System.Windows.Forms;

namespace SharpTerminal
{    
    public class RemoteRoot : RemoteFolder
    {             
        public RemoteRoot(string name, string className, string path, ulong internalId, ulong parentInternalId) :
            base(name, className, path, internalId, parentInternalId)
        {            
            //empty
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);
        }

        public override Control CreateControl(IConsole console)
        {
            return new DashboardUserControl(console, this);
        }
    }  
}

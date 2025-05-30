using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Versioning;
using System.Text;
using System.Threading.Tasks;

namespace SharpTerminal
{
    [SupportedOSPlatform("windows")]
    internal class RemoteDccLiteService(string name, string className, string path, ulong internalId, RemoteFolder parent) : 
        RemoteFolder(name, className, path, internalId, parent)
    {
        protected static IRemoteObjectAction g_ClearAction = new RemoteObjectCmdAction("Clear-DccLiteBlockList", "Clear Block List", "Clear the block list");

        public override IRemoteObjectAction[] GetActions()
        {
            return [g_ClearAction];
        }
    }
}

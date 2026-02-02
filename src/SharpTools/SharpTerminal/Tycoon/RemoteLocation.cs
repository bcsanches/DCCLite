using System.Json;
using System.Runtime.Versioning;

namespace SharpTerminal.Tycoon
{
	[SupportedOSPlatform("windows")]
	public class RemoteLocation: RemoteFolder
	{
		public RemoteLocation(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
			base(name, className, path, internalId, parent)
		{
			UpdateState(objectDef);
		}
	}
}

using System.ComponentModel;
using System.Json;
using System.Runtime.Versioning;

namespace SharpTerminal.Tycoon
{
	[SupportedOSPlatform("windows")]
	public class RemoteIndustry: RemoteObject
	{
		[Category("Cargo Holder")]
		public string CargoName { get; private set; }

		[Category("Cargo Holder")]
		public float DailyRate { get; private set; }

		[Category("Cargo Holder")]
		public int MaximumQuantity { get; private set; }

		[Category("Cargo Holder")]
		public int CurrentQuantity { get; private set; }

		[Category("Cargo Holder")]
		public bool Producing { get; private set; }

		[Category("Cargo Holder")]
		public string NextProductionAt { get; private set; }

		public RemoteIndustry(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
			base(name, className, path, internalId, parent)
		{
			CargoName = (string)objectDef["cargo"];
			DailyRate = (float)objectDef["dailyRate"];
			MaximumQuantity = (int)objectDef["maximumQuantity"];
			CurrentQuantity = (int)objectDef["currentQuantity"];
			Producing = (bool)objectDef["producing"];
			NextProductionAt = (string)objectDef["nextProductionAt"];
		}

		protected override void OnUpdateState(JsonValue def)
		{
			base.OnUpdateState(def);
		}
	}
}

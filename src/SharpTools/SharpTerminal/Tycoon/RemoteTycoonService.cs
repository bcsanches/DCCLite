using SharpTerminal.Forms;
using System.Collections.Generic;
using System.Json;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Tycoon
{
	[SupportedOSPlatform("windows")]
	public class RemoteTycoonService: RemoteFolder
	{
		internal class CarType
		{
			public string Name { get; }
			public string Description { get; }

			List<Cargo> m_lstCargos = [];

			public CarType(JsonValue objectDef, Dictionary<string, Cargo> registeredCargos)
			{
				Name = (string) objectDef["name"];
				Description = (string) objectDef["description"];

				if(objectDef.ContainsKey("cargos"))
				{
					var cargosData = (JsonArray)objectDef["cargos"];

					foreach(var c in cargosData)
					{
						var cargo = registeredCargos[(string) c];
						m_lstCargos.Add(cargo);
					}
						
				}
			}
		}

		internal class Cargo
		{
			public string Name { get; }

			public Cargo(string name)
			{
				Name = name ?? throw new System.ArgumentNullException(nameof(name));

				if(string.IsNullOrWhiteSpace(Name))
					throw new System.ArgumentNullException(nameof(name));
			}
		}


		Dictionary<string, Cargo> m_mapCargos = [];
		List<CarType> m_lstCarTypes = [];

		string m_strFastClockTime = "00:00";

		public string FastClockTime
		{
			get => m_strFastClockTime;
			private set
			{
				base.UpdateProperty(ref m_strFastClockTime, value);
			}
		}

		public RemoteTycoonService(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
			base(name, className, path, internalId, parent)
		{
			UpdateState(objectDef);

			if(objectDef.ContainsKey("cargos"))
			{
				var cargosData = (JsonArray)objectDef["cargos"];

				foreach (JsonValue c in cargosData)
				{
					var cargoName = (string)c;

					m_mapCargos.Add(cargoName, new Cargo(cargoName));
				}					
			}

			if(objectDef.ContainsKey("carTypes"))
			{
				var carTypes = (JsonArray)objectDef["carTypes"];

				foreach (var c in carTypes)
					m_lstCarTypes.Add(new CarType(c, m_mapCargos));
			}
		}

		protected override void OnUpdateState(JsonValue def)
		{
			base.OnUpdateState(def);

			if (def.ContainsKey("fast_clock_time"))
			{
				FastClockTime = (string)def["fast_clock_time"];
			}
		}

		public override Control CreateControl(IConsole console)
		{
			return new RemoteTycoonUserControl(console, this);
		}

		public override string GetNameSuffix()
		{
			return " [" + m_strFastClockTime + "]";
		}
	}
}

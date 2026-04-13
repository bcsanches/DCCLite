using SharpTerminal.Forms;
using System.Collections.Generic;
using System.ComponentModel;
using System.Json;
using System.Linq;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Tycoon
{
	public enum SpotStates
	{
		FREE,
		RESERVED,
		LOADING,
		UNLOADING,
		CAR_PARKED
	}

	public class Spot(string name, SpotStates state = SpotStates.FREE): NotifyPropertyBase
	{
		public readonly string Name = name ?? throw new System.ArgumentNullException(nameof(name));
		private SpotStates m_kState = state;
		private string m_strInformation = string.Empty;

		public SpotStates State
		{
			get => m_kState;
			private set
			{
				UpdateProperty(ref m_kState, value);				
			}
		}

		public string Information
		{
			get => m_strInformation;
			set
			{
				UpdateProperty(ref m_strInformation, value);
			}
		}

		public void UpdateRemoteState(string stateName, string info)
		{
			State = (SpotStates)System.Enum.Parse(typeof(SpotStates), stateName);
			Information = info;
		}

		public override string ToString()
		{
			return Name;
		}
	}

	[SupportedOSPlatform("windows")]
	public class RemoteIndustry: RemoteObject
	{
		[Category("Cargo Holder")]
		public string CargoName { get; private set; }

		[Category("Cargo Holder")]
		public float DailyRate { get; private set; }

		[Category("Cargo Holder")]
		public int MaximumQuantity { get; private set; }

		private int m_iCurrentQuantity;

		[Category("Cargo Holder")]
		public int CurrentQuantity 
		{ 
			get => m_iCurrentQuantity; 
			private set
			{
				UpdateProperty(ref m_iCurrentQuantity, value);
			}
		}


		private bool m_fProducing;
		[Category("Cargo Holder")]
		public bool Producing
		{
			get => m_fProducing;
			private set
			{
				UpdateProperty(ref m_fProducing, value);
			}
		}

		private string m_strNextProductionAt;
		[Category("Cargo Holder")]
		public string NextProductionAt 
		{ 
			get => m_strNextProductionAt; 
			private set => UpdateProperty(ref m_strNextProductionAt, value);
		}

		private string m_strNextProductionAtLocalTime;
		[Category("Cargo Holder")]
		public string NextProductionAtLocalTime
		{
			get => m_strNextProductionAtLocalTime;
			private set => UpdateProperty(ref m_strNextProductionAtLocalTime, value);
		}

		private SortedDictionary<string, Spot> m_setSpots = new();

		public Spot[] GetSpots() => m_setSpots.Values.ToArray();

		public RemoteIndustry(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
			base(name, className, path, internalId, parent)
		{
			CargoName = (string)objectDef["cargo"];
			DailyRate = (float)objectDef["dailyRate"];
			MaximumQuantity = (int)objectDef["maximumQuantity"];
			CurrentQuantity = (int)objectDef["currentQuantity"];
			Producing = (bool)objectDef["producing"];
			NextProductionAt = (string)objectDef["nextProductionAt"];
			NextProductionAtLocalTime = (string)objectDef["nextProductionAtLocalTime"];

			this.LoadSpots((JsonArray)objectDef["spots"]);			
		}

		private void LoadSpots(JsonArray spotsData)
		{
			foreach (JsonValue s in spotsData)
			{
				var spotName = (string)s["name"];
				var state = (SpotStates)System.Enum.Parse(typeof(SpotStates), (string)s["state"]);

				var spot = new Spot(spotName, state);

				m_setSpots.Add(spot.Name, spot);
			}
		}

		protected override void OnUpdateState(JsonValue def)
		{
			base.OnUpdateState(def);			

			if(def.ContainsKey("currentQuantity"))
				CurrentQuantity = (int)def["currentQuantity"];

			if(def.ContainsKey("producing"))
				Producing = (bool)def["producing"];

			if(def.ContainsKey("nextProductionAt"))
				NextProductionAt = (string)def["nextProductionAt"];

			if (def.ContainsKey("spots"))
			{
				var spotsData = (JsonArray)def["spots"];
				foreach (JsonValue s in spotsData)
				{
					var spotName = (string)s["name"];

					m_setSpots[spotName].UpdateRemoteState((string)s["state"], (string)s["info"]);
				}
			}
		}

		public override Control CreateControl(IConsole console)
		{
			return new RemoteIndustryUserControl(console, this);
		}
	}
}

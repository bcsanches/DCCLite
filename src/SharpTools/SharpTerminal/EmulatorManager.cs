using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
	public class Emulator
	{
		readonly string m_strDeviceName;

		public string DeviceName
		{
			get { return m_strDeviceName; }
		}

		public Emulator(string deviceName)
		{
			m_strDeviceName = deviceName;

			if (string.IsNullOrWhiteSpace(m_strDeviceName))
			{
				throw new ArgumentNullException(nameof(deviceName));
			}
		}
	}

	internal static class EmulatorManager
	{
		static readonly Dictionary<string, Emulator> g_mapEmulators = [];

		public static void StartEmulator(string deviceName)
		{
			if (string.IsNullOrWhiteSpace(deviceName))
			{
				throw new ArgumentException(nameof(deviceName));
			}

			if (g_mapEmulators.ContainsKey(deviceName))
			{
				//nothing to do
				return;
			}

			var emulator = new Emulator(deviceName);
			g_mapEmulators[deviceName] = emulator;
		}

		internal static IEnumerable<Emulator> GetEmulators()
		{
			return g_mapEmulators.Select(x => x.Value);
		}
	}	

	internal class EmulatorManagerProxy: IControlProvider
	{
		[SupportedOSPlatform("windows")]
		Control IControlProvider.CreateControl(IConsole console)
		{
			return new Forms.EmulatorDashboardUserControl();
		}
	}
}

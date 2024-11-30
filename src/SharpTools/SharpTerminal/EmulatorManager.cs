using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Forms;

namespace SharpTerminal
{
	class Emulator
	{

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

			var emulator = new Emulator();
			g_mapEmulators[deviceName] = emulator;
		}
	}

	internal class EmulatorManagerProxy: IControlProvider
	{
		Control IControlProvider.CreateControl(IConsole console)
		{
			return new Forms.EmulatorDashboardUserControl();
		}
	}
}

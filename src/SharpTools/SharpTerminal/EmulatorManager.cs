using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Versioning;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
	public class Emulator
	{
		readonly string m_strDeviceName;
		readonly Process m_Process;

		private StringBuilder m_sbOutput = new ();
		private int m_iLineCount = 0;

		public delegate void AppendOutputDelegate(object sender, string strLine, int lineCount);

		public AppendOutputDelegate AppendOutput;

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

			var psi = new ProcessStartInfo
			{
				FileName = FindEmulatorExecutable(),
				Arguments = "-d " + deviceName,
				UseShellExecute = false,           // required for redirection
				RedirectStandardOutput = true,     // capture stdout
				RedirectStandardError = true,      // capture stderr (optional)
				CreateNoWindow = true              // hide console window				
			};

			m_Process = new();
			m_Process.StartInfo = psi;
			m_Process.EnableRaisingEvents = true;
			m_Process.OutputDataReceived += (sender, args) =>
			{
				if (args.Data != null)
				{
					m_sbOutput.AppendLine(args.Data);
					++m_iLineCount;

					AppendOutput?.Invoke(this, args.Data, m_iLineCount);
				}
			};

			m_Process.Start();
			m_Process.BeginOutputReadLine(); // start async read of stdout
		}

		private static string FindEmulatorExecutable()
		{			
#if DEBUG
			if (File.Exists("..\\..\\..\\..\\..\\..\\build\\bin\\Debug\\Emulator.exe"))
				return "..\\..\\..\\..\\..\\..\\build\\bin\\Debug\\Emulator.exe";
#else
			if (File.Exists("..\\..\\..\\..\\..\\..\\build\\bin\\Release\\Emulator.exe"))
				return "..\\..\\..\\..\\..\\..\\build\\bin\\Release\\Emulator.exe";
#endif

			return "Emulator.exe";
		}

		public bool HasExited
		{
			get 
			{
				return m_Process.HasExited;
			}	
		}

		public void Kill()
		{
			m_Process.Kill();
		}

		public void Restart()
		{
			if(!m_Process.HasExited)
			{
				throw new InvalidOperationException("Process is running.");
			}

			m_Process.Start();
		}

		private event EventHandler mExitHandler;
		private bool mExitHandlerAdded = false;

		public void AddExitHandler(EventHandler handler)
		{
			mExitHandler += handler;

			if(!mExitHandlerAdded)
			{
				mExitHandlerAdded = true;
				m_Process.Exited += Process_OnExited;
			}			
		}

		public void RemoveExitHandler(EventHandler handler)
		{
			mExitHandler -= handler;
		}

		private void Process_OnExited(object sender, EventArgs e)
		{
			mExitHandler?.Invoke(this, EventArgs.Empty);
		}

		public string GetOutput()
		{
			return m_sbOutput.ToString();
		}
	}

	internal static class EmulatorManager
	{
		static readonly Dictionary<string, Emulator> g_mapEmulators = [];

		public static void StartEmulator(string deviceName)
		{
			if (string.IsNullOrWhiteSpace(deviceName))
			{
				throw new ArgumentException(null, nameof(deviceName));
			}

			if(g_mapEmulators.TryGetValue(deviceName, out var existingEmulator))			
			{
				if (!existingEmulator.HasExited)
					return;

				//It is dead... start a new one
				g_mapEmulators.Remove(deviceName);
			}

			var emulator = new Emulator(deviceName);
			g_mapEmulators[deviceName] = emulator;
		}

		public static void KillAll()
		{
			foreach(var emulator in g_mapEmulators.Values)
			{
				if (!emulator.HasExited)
				{
					emulator.Kill();
				}
			}
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

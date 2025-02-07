using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
	/// <summary>
	/// All the useful stuff that does not fit anywhere else and does not deserve their own file....
	/// </summary>
	internal static class Util
	{
		public static string FindExecutable(string executableName)
		{
			string executableFile = executableName + ".exe";
			string path = executableFile;
			if (System.IO.File.Exists(executableFile))
				return executableFile;
			
			path = System.Reflection.Assembly.GetEntryAssembly().Location;
			path = path.Replace("SharpTerminal.dll", executableFile);
			path = path.Replace("SharpTerminal", executableName);

			return System.IO.File.Exists(path) ? path : null;
		}
	}
}

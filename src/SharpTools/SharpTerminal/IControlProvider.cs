using System.Windows.Forms;

namespace SharpTerminal
{
	internal interface IControlProvider
	{
		public Control CreateControl(IConsole console);
	}
}

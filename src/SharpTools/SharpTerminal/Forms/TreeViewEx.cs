using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal.Forms
{
	public class TreeViewEx: TreeView
	{
		[SupportedOSPlatform("windows")]
		public TreeViewEx()
		{
			this.ForceDoubleBuffering();
		}

		[SupportedOSPlatform("windows")]
		public void ForceDoubleBuffering()
		{
			this.SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint, true);
			this.UpdateStyles();
		}
	}
}

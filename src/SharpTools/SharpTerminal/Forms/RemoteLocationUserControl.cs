// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System;
using System.Drawing;
using System.Linq;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteLocationUserControl : UserControl
	{
		private int mBeginAddress;
		private int mEndAddress;
		private string[] mDecodersPath;

		public RemoteLocationUserControl()
		{
			InitializeComponent();
		}

		public RemoteLocationUserControl(String name, int beginAddress, int endAddress, string[] decodersPath) :
			this()
		{
			mBeginAddress = beginAddress;
			mEndAddress = endAddress;
			mDecodersPath = decodersPath;

			m_lbTitle.Text += " " + name;			
		}

		protected async override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);		

			int num = (mEndAddress - mBeginAddress) - 1;
			if (num <= 0)
				return;			

			this.DoubleBuffered = true;			
			m_gridMain.SuspendLayout();
			m_gridMain.Rows.Add(num);

			for (int i = mBeginAddress, pos = 0; i < mEndAddress; ++i, ++pos)
			{
				var row = m_gridMain.Rows[pos];

				row.Cells[0].Value = i;

				if ((mDecodersPath == null) || (mDecodersPath[pos] == null))
					continue;

				row.Tag = RemoteObjectManager.GetRemoteObjectAsync(mDecodersPath[pos]);
			}

			for (int i = mBeginAddress, pos = 0; i < mEndAddress; ++i, ++pos)
			{
				var row = m_gridMain.Rows[pos];

				var task = (System.Threading.Tasks.Task<RemoteObject>)row.Tag;
				if (task == null)
					continue;

				var decoder = (RemoteDecoder) await task;

				row.Cells[1].Value = decoder.ClassName;
				row.Cells[2].Value = decoder.Name;
				row.Cells[3].Value = decoder.DeviceName;

				if (decoder.Broken)
					row.DefaultCellStyle.BackColor = Color.Red;
			}
			
			m_gridMain.ResumeLayout();			
		}
	}
}

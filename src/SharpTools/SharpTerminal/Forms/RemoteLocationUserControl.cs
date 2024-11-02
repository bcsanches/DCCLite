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
			m_lvItems.SuspendLayout();			

			for (int i = mBeginAddress, pos = 0; i < mEndAddress; ++i, ++pos)
			{
				var item = new ListViewItem(i.ToString());
				m_lvItems.Items.Add(item);
				
				if ((mDecodersPath == null) || (mDecodersPath[pos] == null))
					continue;

				item.Tag = RemoteObjectManager.GetRemoteObjectAsync(mDecodersPath[pos]);
			}

			for (int i = mBeginAddress, pos = 0; i < mEndAddress; ++i, ++pos)
			{
				var row = m_lvItems.Items[pos];

				var task = (System.Threading.Tasks.Task<RemoteObject>)row.Tag;
				if (task == null)
					continue;

				row.Tag = null;
				var decoder = (RemoteDecoder) await task;

				row.SubItems.Add(decoder.ClassName);
				row.SubItems.Add(decoder.Name);
				row.SubItems.Add(decoder.DeviceName);
				if (decoder.Broken)
					row.BackColor = Color.Red;
			}

			m_lvItems.AutoResizeColumns(ColumnHeaderAutoResizeStyle.HeaderSize);
			m_lvItems.AutoResizeColumn(m_lvItems.Columns.Count - 1, ColumnHeaderAutoResizeStyle.HeaderSize);

			m_lvItems.ResumeLayout();			
		}
	}
}
	
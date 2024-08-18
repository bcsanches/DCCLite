// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System.Drawing;
using System.Windows.Forms;
using System.Runtime.Versioning;
using System;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteLocationManagerUserControl : UserControl
    {
        private LocationMismatch[] mMisMatches;

		public RemoteLocationManagerUserControl()
        {
            InitializeComponent();
        }

        public RemoteLocationManagerUserControl(LocationMismatch[] mismatches) :
            this()
        {            
			mMisMatches = mismatches;			
        }

		protected async override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if (mMisMatches == null)
				return;

			m_gridMain.Rows.Add(mMisMatches.Length);

			for (int i = 0; i < mMisMatches.Length; ++i)
			{
				var row = m_gridMain.Rows[i];
				var mismatch = mMisMatches[i];

				var decoder = await mismatch.LoadDecoderAsync();

				row.Cells[0].Value = decoder.Address;
				row.Cells[1].Value = decoder.ClassName;
				row.Cells[2].Value = decoder.Name;
				row.Cells[3].Value = mismatch.Reason;
				row.Cells[4].Value = decoder.DeviceName;
				row.Cells[5].Value = decoder.LocationHint;

				if (mismatch.MappedLocation != null)
					row.Cells[6].Value = mismatch.MappedLocation;

				if (decoder.Broken)
					row.DefaultCellStyle.BackColor = Color.Red;
			}
		}
	}
}

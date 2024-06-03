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
        public RemoteLocationUserControl()
        {
            InitializeComponent();
        }

        public RemoteLocationUserControl(String name, int beginAddress, int endAddress, RemoteDecoder[] decoders) :
            this()
        {
            int num = endAddress - beginAddress;
            if (num == 0)
                return;

            m_gridMain.Rows.Add(num);

            for(int i = beginAddress, pos = 0;i < endAddress; ++i, ++pos)
            {
                var row = m_gridMain.Rows[pos];

                row.Cells[0].Value = i;

                if (decoders[pos] == null)
                    continue;

                row.Cells[1].Value = decoders[pos].ClassName;
                row.Cells[2].Value = decoders[pos].Name;
                row.Cells[3].Value = decoders[pos].DeviceName;

                if (decoders[pos].Broken)
                    row.DefaultCellStyle.BackColor = Color.Red;
            }

            m_lbTitle.Text += " " + name;
        }
    }
}

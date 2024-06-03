// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class RemoteLoconetServiceUserControl : UserControl
    {
        public RemoteLoconetServiceUserControl()
        {
            InitializeComponent();
        }

        public RemoteLoconetServiceUserControl(RemoteLoconetService loconet, RemoteLoconetSlot []slots) :
            this()
        {            
            if (slots == null)
                return;


            for (int i = 0; i < slots.Length; ++i)
            {
                m_bsDataSource.Add(slots[i]);
            }
                //m_bsDataSource = slots;

            /*
            m_gridMain.Rows.Add(slots.Length);

            for (int i = 0; i < slots.Length; ++i)
            {
                var row = m_gridMain.Rows[i];
                var slot = slots[i];

                row.Cells[0].Value = i;
                row.Cells[1].Value = slot.State;
                row.Cells[2].Value = slot.LocomotiveAddress;
                row.Cells[3].Value = slot.Forward ? "FWD" : "BWD";
                row.Cells[4].Value = slot.Speed.ToString();
            }
            */
        }
    }
}

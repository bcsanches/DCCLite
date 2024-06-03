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
	public partial class RemoteDeviceUserControl : UserControl
    {
        private readonly RemoteDevice mRemoteDevice;

        public RemoteDeviceUserControl()
        {
            InitializeComponent();
        }

        public RemoteDeviceUserControl(RemoteDevice remoteDevice, RemotePin[] pins) :
            this()
        {            
            mRemoteDevice = remoteDevice ?? throw new ArgumentNullException(nameof(remoteDevice));

            this.UpdateLabel();

            if (pins == null)
                return;

            m_gridMain.Rows.Add(pins.Length);

            for(int i = 0; i < pins.Length; ++i)            
            {
                var row = m_gridMain.Rows[i];
                var pin = pins[i];                

                row.Cells[0].Value = i;
                row.Cells[1].Value = pin.SpecialName;
                row.Cells[2].Value = pin.Decoder;
                row.Cells[3].Value = pin.DecoderAddress;
                row.Cells[4].Value = pin.Usage;
                
                if((pin.DecoderBroken != null) && ((bool)pin.DecoderBroken))
                {
                    row.DefaultCellStyle.BackColor = Color.Red;
                }
            }

            remoteDevice.PropertyChanged += RemoteDevice_PropertyChanged;
        }

        private void UpdateLabel()
        {
            m_lbTitle.Text = "RemoteDevice: " + mRemoteDevice.Name + " - mem: " + mRemoteDevice.FreeRam.ToString() + " bytes";
        }

        private void RemoteDevice_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            //We only expect free ram to change...
            UpdateLabel();
        }        
    }
}

﻿// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

using SharpTerminal.Forms;
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
		private readonly IConsole mConsole;

		public RemoteDeviceUserControl()
		{
			InitializeComponent();
		}

		public RemoteDeviceUserControl(IConsole console, RemoteDevice remoteDevice, RemotePin[] pins) :
			this()
		{
			mRemoteDevice = remoteDevice ?? throw new ArgumentNullException(nameof(remoteDevice));
			mConsole = console ?? throw new ArgumentNullException(nameof(console));

			m_btnRename.Enabled = !mRemoteDevice.Registered;

			this.UpdateLabel();

			if (pins == null)
				return;

			m_gridMain.Rows.Add(pins.Length);

			for (int i = 0; i < pins.Length; ++i)
			{
				var row = m_gridMain.Rows[i];
				var pin = pins[i];

				row.Cells[0].Value = i;
				row.Cells[1].Value = pin.SpecialName;
				row.Cells[2].Value = pin.Decoder;
				row.Cells[3].Value = pin.DecoderAddress;
				row.Cells[4].Value = pin.Usage;

				if ((pin.DecoderBroken != null) && ((bool)pin.DecoderBroken))
				{
					row.DefaultCellStyle.BackColor = Color.Red;
				}
			}

			remoteDevice.PropertyChanged += RemoteDevice_PropertyChanged;
		}

		private void UpdateLabel()
		{
			var name = "RemoteDevice: " + mRemoteDevice.Name + " - mem: " + mRemoteDevice.FreeRam.ToString() + " bytes";
			m_lbTitle.Text = mRemoteDevice.Registered ? name : name + " - Unregistered";
		}

		private void RemoteDevice_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			//We only expect free ram to change...
			UpdateLabel();
		}

		private async void m_btnRename_Click(object sender, EventArgs e)
		{			
			using var form = new RemoteDeviceRenameForm();
			form.ShowDialog();

			if (form.DialogResult == DialogResult.Cancel)
				return;

			m_btnRename.Enabled = false;
			try
			{
				await mConsole.RequestAsync(["Rename-Item", mRemoteDevice.Path, "newName"]);
			}
			catch (Exception ex)
			{
				MessageBox.Show("Error: " + ex.Message, "Error during operation", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}

			m_btnRename.Enabled = true;			
		}
	}
}
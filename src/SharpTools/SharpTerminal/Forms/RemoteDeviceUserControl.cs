// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
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
using System.Json;
using System.Linq;
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
    [SupportedOSPlatform("windows")]
    public partial class RemoteDeviceUserControl : UserControl
    {
        private readonly RemoteNetworkDevice mRemoteDevice;
        private readonly IConsole mConsole;

        public RemoteDeviceUserControl()
        {
            InitializeComponent();
        }

        public RemoteDeviceUserControl(IConsole console, RemoteNetworkDevice remoteDevice, RemotePin[] pins) :
            this()
        {
            mRemoteDevice = remoteDevice ?? throw new ArgumentNullException(nameof(remoteDevice));
            mConsole = console ?? throw new ArgumentNullException(nameof(console));

            m_btnRename.Enabled = !mRemoteDevice.Registered;

            this.RefreshButtonsState();

            mRemoteDevice.StateChanged += RemoteDevice_StateChanged;
            remoteDevice.PropertyChanged += RemoteDevice_PropertyChanged;

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
        }

        private void RefreshButtonsState()
        {
            bool onlineDevice = mRemoteDevice.ConnectionStatus == RemoteNetworkDevice.Status.ONLINE;

            m_btnBlock.Enabled = onlineDevice;
            m_btnClear.Enabled = onlineDevice;
            m_btnDisconnect.Enabled = onlineDevice;
            m_btnNetworkTest.Enabled = onlineDevice;
            m_btnReadEEPROM.Enabled = onlineDevice;
            m_btnReboot.Enabled = onlineDevice;

            m_btnEmulate.Enabled = !onlineDevice;
        }

        private void RemoteDevice_StateChanged(RemoteObject sender, EventArgs args)
        {
            this.RefreshButtonsState();
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
            var parent = mRemoteDevice.Parent;

            var possibleNames = parent.GetChildren().Where(x => (x != mRemoteDevice) && ((x is RemoteNetworkDevice device) && (device.ConnectionStatus == RemoteNetworkDevice.Status.OFFLINE) && device.Registered)).Select(x => x.Name);

            using var form = new RemoteDeviceRenameForm(possibleNames);
            form.ShowDialog();

            if (form.DialogResult == DialogResult.Cancel)
                return;

            m_btnRename.Enabled = false;
            try
            {
                await mConsole.RequestAsync(["Rename-Item", mRemoteDevice.Path, form.SelectedName]);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Error during operation", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            m_btnRename.Enabled = true;
        }

        private async void m_btnClear_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Are you sure? This cannot be undone!!", "Are you sure?", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button2) != DialogResult.Yes)
                return;

            try
            {
                await mConsole.RequestAsync(["Clear-EEProm", mRemoteDevice.Path]);

                MessageBox.Show("Manually reset the device for changes to take effect", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void m_btnEmulate_Click(object sender, EventArgs e)
        {
            EmulatorManager.StartEmulator(mRemoteDevice.Name);
        }

        private async void m_btnReboot_Click(object sender, EventArgs e)
        {
            try
            {
                await mConsole.RequestAsync(["Reboot-Device", mRemoteDevice.Path]);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void m_btnNetworkTest_Click(object sender, EventArgs e)
        {
            using var form = new NetworkTestForm(mRemoteDevice, mConsole);
            form.ShowDialog();
        }

        private async void m_btnReadEEPROM_Click(object sender, EventArgs e)
        {
            try
            {
                var result = await mConsole.RequestAsync(["Read-EEProm", mRemoteDevice.Path]);
                mConsole.HandleReadEEPromResult((JsonObject)result);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private async void m_btnDisconnect_Click(object sender, EventArgs e)
        {
            try
            {
                await mConsole.RequestAsync(["Disconnect-Device", mRemoteDevice.Path]);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private async void m_btnBlock_Click(object sender, EventArgs e)
        {
            try
            {
                await mConsole.RequestAsync(["Block-Device", mRemoteDevice.Path]);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Operation failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}

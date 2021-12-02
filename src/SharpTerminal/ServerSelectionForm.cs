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
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class ServerSelectionForm : Form
    {
        private UdpClient mClient = new();

        private List<ServiceInfo> m_lstServices = new();

        const uint      PACKET_MAGIC_NUMBER = 0xABCDDCCA;
	    const byte      PACKET_VERSION = 0x01;

        const int       PACKET_MINIMUM_SIZE = 10;

        const ushort    ZEROCONF_PORT = 9381;

        const string    SERVICE_NAME = "TerminalService";

        bool            mFirstService = true;

        uint            mCountdown = 5;

        ServiceInfo     mSelectedService;

        class ServiceInfo
        {
            public string      mServerName;
            public IPAddress   mAddress;
            public ushort      mPort;
        };

        public ServerSelectionForm()
        {
            InitializeComponent();                       
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            mClient.EnableBroadcast = true;
            mClient.Client.Bind(new IPEndPoint(IPAddress.Any, 0));

            this.SendQuery();
            mTimer.Start();

            mBackgroundWorker.WorkerReportsProgress = true;
            mBackgroundWorker.RunWorkerAsync();
        }        

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);            
        }

        private void SendQuery()
        {
            short packetSize = 0;
            byte[] data;

            var stream = new MemoryStream();
            using (var writer = new BinaryWriter(stream))
            {
                writer.Write(PACKET_MAGIC_NUMBER);
                writer.Write(PACKET_VERSION);
                writer.Write((byte)1);              //flag 0x01 - query

                var str = Encoding.UTF8.GetBytes(SERVICE_NAME);
                writer.Write(str);
                writer.Write((byte)0);
                writer.Write((short)0);

                data = stream.GetBuffer();
                packetSize = (short)stream.Length;
            }

            mClient.Send(data, packetSize, "255.255.255.255", ZEROCONF_PORT);
        }

        private static string ReadString(BinaryReader reader)
        {
            StringBuilder builder = new StringBuilder(64);
            byte[] array = new byte[1];
            for (; ; )
            {
                var ch = reader.ReadByte();
                if (ch == 0)
                    break;

                array[0] = ch;
                builder.Append(Encoding.UTF8.GetChars(array));
            }

            return builder.ToString();
        }

        private void mBackgroundWorker_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
        {          
            for(; ;)
            {                
                IPEndPoint endPoint = null;

                try
                {
                    byte[] responseData = mClient.Receive(ref endPoint);
                    if (responseData.Length < PACKET_MINIMUM_SIZE)
                        continue;

                    using (var reader = new BinaryReader(new MemoryStream(responseData)))
                    {
                        if (reader.ReadUInt32() != PACKET_MAGIC_NUMBER)
                            continue;

                        if (reader.ReadByte() != PACKET_VERSION)
                            continue;

                        //If not zero, it is a query...
                        if (reader.ReadByte() != 0)
                            continue;

                        var serviceName = ReadString(reader);                        

                        if (serviceName != SERVICE_NAME)
                            continue;

                        ServiceInfo info = new()
                        {
                            mAddress = endPoint.Address,
                            mPort = reader.ReadUInt16(),
                            mServerName = ReadString(reader)
                        };

                        mBackgroundWorker.ReportProgress(0, info);
                    }
                }                
                catch(SocketException )
                {                    
                    //dialog closed ??
                    return;
                }

            }
        }

        private void mTimer_Tick(object sender, EventArgs e)
        {
            this.SendQuery();            
        }

        private void UpdateCountdownLabel()
        {
            m_lblCountdown.Visible = true;
            m_lblCountdown.Text = "Auto connect in " + mCountdown.ToString();
        }

        private void StopCountdown()
        {
            m_lblCountdown.Visible = false;
            mCountdownTimer.Stop();
        }

        private void mBackgroundWorker_ProgressChanged(object sender, System.ComponentModel.ProgressChangedEventArgs e)
        {
            var newService = (ServiceInfo)e.UserState;

            foreach(var service in m_lstServices)
            {
                if ((service.mPort == newService.mPort) && (service.mAddress.Equals(newService.mAddress)) && (service.mServerName.Equals(newService.mServerName)))
                    return;
            }

            m_lstServices.Add(newService);

            mServicesGrid.Rows.Add();

            var row = mServicesGrid.Rows[mServicesGrid.Rows.Count - 1];

            row.Cells[0].Value = newService.mServerName;
            row.Cells[1].Value = newService.mAddress.ToString() + ":" + newService.mPort;
            row.Tag = newService;

            if(mFirstService)
            {
                mFirstService = false;

                mCountdownTimer.Start();
                UpdateCountdownLabel();

                UpdateSelectedService();
            }
            else
            {
                this.StopCountdown();
            }
        }

        private void UpdateSelectedService()
        {
            if(mServicesGrid.SelectedRows.Count == 0)
            {
                mSelectedService = null;

                m_btnOK.Enabled = false;                
            }
            else
            {
                mSelectedService = (ServiceInfo)mServicesGrid.SelectedRows[0].Tag;
                m_btnOK.Enabled = true;
            }
            
        }

        private void mCountdownTimer_Tick(object sender, EventArgs e)
        {
            --mCountdown;

            UpdateCountdownLabel();

            if (mCountdown == 0)
            {
                this.StopCountdown();
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
        }

        private void ServerSelectionForm_Click(object sender, EventArgs e)
        {
            this.StopCountdown();
        }        

        private void ServerSelectionForm_KeyDown(object sender, KeyEventArgs e)
        {
            this.StopCountdown();
        }

        private void mServicesGrid_SelectionChanged(object sender, EventArgs e)
        {
            this.UpdateSelectedService();
        }
    }
}

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
using System.Runtime.Versioning;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class ServerSelectionForm : Form
    {
        private UdpClient           mClient = new();

        private List<ServiceInfo>   m_lstServices = new();

        const uint                  PACKET_MAGIC_NUMBER = 0xABCDDCCA;
	    const byte                  PACKET_VERSION = 0x01;

        const int                   PACKET_MINIMUM_SIZE = 10;

        const ushort                ZEROCONF_PORT = 9381;

        const int                   TIMER_INTERVAL = 250;

        const int                   DEFAULT_COUNTDOWN_SECS = 5;

        const int                   DEFAULT_TTL = TIMER_INTERVAL * 4 * (DEFAULT_COUNTDOWN_SECS + 1);        

        const string                SERVICE_NAME = "TerminalService";

        bool                        mFirstService = true;

        uint                        mCountdown = DEFAULT_COUNTDOWN_SECS;

        public ServiceInfo          mSelectedService;

        string mFastService;
        ushort mFastPort;

        public class ServiceInfo
        {
            public string      mServerName;
            public IPAddress   mAddress;
            public ushort      mPort;

            public int          mTTL;
        };

		public ServerSelectionForm()
		{
			InitializeComponent();
		}

		public ServerSelectionForm(string previousHost, ushort previousPort)
        {
            InitializeComponent();

			mFastService = previousHost;
            mFastPort = previousPort;

		}

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            
            mClient.EnableBroadcast = true;
            mClient.Client.Bind(new IPEndPoint(IPAddress.Any, 0));

            this.SendQuery();

            mTimer.Interval = TIMER_INTERVAL;
            mTimer.Start();

            mBackgroundWorker.WorkerReportsProgress = true;
            mBackgroundWorker.RunWorkerAsync();
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
                            mServerName = ReadString(reader),
                            mTTL = DEFAULT_TTL
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

            bool expired = false;
            foreach (var info in m_lstServices)
            {
                info.mTTL -= mTimer.Interval;

                if (info.mTTL <= 0)
                { 
                    foreach(DataGridViewRow row in mServicesGrid.Rows)
                    {
                        if(row.Tag == info)
                        {
                            mServicesGrid.Rows.Remove(row);
                            
                            break;
                        }
                    }
                    
                    expired = true;
                }                    
            }

            if (expired)
            {
                m_lstServices.RemoveAll(x => x.mTTL <= 0);

                this.StopCountdown();
                this.mServicesGrid_SelectionChanged(sender, e);
            }                
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

            //
            //Do we already know about this service? If yes, just update the TTL
            foreach(var service in m_lstServices)
            {
                if ((service.mPort == newService.mPort) && (service.mAddress.Equals(newService.mAddress)) && (service.mServerName.Equals(newService.mServerName)))
                {
                    service.mTTL = newService.mTTL;

                    return;
                }                    
            }

            m_lstServices.Add(newService);

            mServicesGrid.Rows.Add();

            var row = mServicesGrid.Rows[mServicesGrid.Rows.Count - 1];

            row.Cells[0].Value = newService.mServerName;
            row.Cells[1].Value = newService.mAddress.ToString() + ":" + newService.mPort;
            row.Tag = newService;

            //Always update selected service because when the  service expires and it is removed list can get empty
            // and refill again, so we always update it, not only for mFirstService
            UpdateSelectedService();

            //This happens when reconnecting, so if the old server comes up, directly connect to it...
            if((mFastService != null) && (newService.mAddress.ToString() == mFastService) && (newService.mPort == mFastPort))
			{
				mSelectedService = newService;
				this.DialogResult = DialogResult.OK;
				this.Close();

                return;
			}

            if (mFirstService)
            {
                mFirstService = false;

                mCountdownTimer.Start();
                UpdateCountdownLabel();                
            }
            else
            {
                this.StopCountdown();
            }
        }

        private void UpdateSelectedService()
        {
            mSelectedService = mServicesGrid.SelectedRows.Count == 0 ? null : (ServiceInfo)mServicesGrid.SelectedRows[0].Tag;

            //always check the mSelectedService, because when we add a new row to the datagrid, the Tag is not set yet, but 
            //the event is fired, so make sure we have a valid SelectedService for enabling btnOK
            m_btnOK.Enabled = mSelectedService != null;
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
            if((e.KeyCode == Keys.Enter) && (mSelectedService != null)) 
            {
                this.DialogResult = DialogResult.OK;
                this.Close();
            }

            this.StopCountdown();
        }

        private void mServicesGrid_SelectionChanged(object sender, EventArgs e)
        {
            this.UpdateSelectedService();
        }
    }
}

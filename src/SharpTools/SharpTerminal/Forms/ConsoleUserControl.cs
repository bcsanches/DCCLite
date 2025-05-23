﻿// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
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
using System.Data;
using System.Json;
using System.Linq;
using System.Runtime.Versioning;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class ConsoleUserControl : UserControl, IResponseHandler, IConsole
    {
        RequestManager mRequestManager;

        private readonly List<string> mUsedCmds = new();
        private int m_iCurrentCmd = 0;

        private List<string> mKnownCmds = new();    
        
        internal RequestManager RequestManager
        {
            set
            {
                if (mRequestManager == value)
                    return;

                if(mRequestManager != null)
                {
                    mRequestManager.ConnectionStateChanged -= mRequestManager_ConnectionStateChanged;
                }

                mRequestManager = value;

                if(mRequestManager != null)
                {
                    mRequestManager.ConnectionStateChanged += mRequestManager_ConnectionStateChanged;
                }
            }
        }

        public ConsoleUserControl()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            m_tbInput.Select();

            KnownCmds_AddDefaultCmds();
        }

        private void Input_Set(string text)
        {
            m_tbInput.ResetText();
            m_tbInput.AppendText(text);
        }

        private void Input_Clear()
        {
            m_tbInput.ResetText();
        }

        private void Input_SendCmd()
        {
            var text = m_tbInput.Text.Trim();
            Input_Clear();

            if (string.IsNullOrWhiteSpace(text))
                return;

            Console_Println("> " + text);

            mUsedCmds.Add(text);
            m_iCurrentCmd = 0;

            ProcessInput(text);
        }

        private void m_tbInput_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (e.KeyCode == Keys.Tab)
            {
                e.IsInputKey = true;
            }
        }

        private void m_tbInput_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Enter:
                    Input_SendCmd();
                    e.Handled = true;

                    break;

                case Keys.Escape:
                    Input_Clear();
                    e.Handled = true;

                    break;

                case Keys.Up:
                    var count = mUsedCmds.Count;
                    if (m_iCurrentCmd < count)
                    {
                        ++m_iCurrentCmd;

                        Input_Set(mUsedCmds[mUsedCmds.Count - m_iCurrentCmd]);

                        e.Handled = true;
                    }
                    break;

                case Keys.Down:
                    if (m_iCurrentCmd > 1)
                    {
                        --m_iCurrentCmd;

                        Input_Set(mUsedCmds[mUsedCmds.Count - m_iCurrentCmd]);

                        e.Handled = true;
                    }
                    break;

                case Keys.Tab:
                    {
                        e.Handled = true;

                        var input = m_tbInput.Text.Trim();

                        if (string.IsNullOrWhiteSpace(input))
                            return;

                        var query = from str in mKnownCmds where str.StartsWith(input) select str;

                        var result = query.FirstOrDefault();
                        if (!string.IsNullOrWhiteSpace(result))
                        {
                            Input_Set(result);
                        }
                    }
                    break;

            }
        }

        private void ProcessLocalCmd(object[] vargs)
        {
            switch (vargs[0])
            {
                case "/clear":
                    Console_Clear();
                    break;

                case "/disconnect":
                    mRequestManager.Disconnect();
                    break;                

                case "/mac":
                    GenerateMac();
                    break;

                case "/quit":
                    this.ParentForm.Close();
                    break;                

                case "/udpping":
                    UdpPing(vargs);
                    break;

                case "/help":
                    foreach(var cmd in mKnownCmds)
                    {
                        if (cmd.StartsWith("/"))
                            Console_Println(cmd);
                    }                    
                    break;

                default:
                    Console_Println("Unknown local command " + vargs[0]);
                    break;
            }
        }

        private int ProcessRemoteCmd(IResponseHandler handler, object[] vargs)
        {
            if (mRequestManager.State != ConnectionState.OK)
            {
                Console_Println("Cannot send command, client not connected. Try /reconnect");

                return -1;
            }

            return mRequestManager.DispatchRequest(vargs, handler ?? this);
        }

        private void ProcessInput(string input)
        {
            input = input.Trim();
            if (string.IsNullOrWhiteSpace(input))
                return;

            var cmds = input.Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
            
            foreach(var cmd in cmds)
            {
                var vargs = cmd.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

                ProcessCmd(vargs);
                
            }            
        }

        public int ProcessCmd(object[] vargs)
        {
            return this.ProcessCmd(this, vargs);
        }

        public int ProcessCmd(IResponseHandler handler, object[] vargs)
        {
            //local command?
            if (((string)vargs[0])[0] == '/')
            {
                ProcessLocalCmd(vargs);

                return -1;
            }
            else
            {
                return ProcessRemoteCmd(handler, vargs);
            }
        }

        public async Task<JsonValue> RequestAsync(params object[] vargs)
        {
            return await mRequestManager.RequestAsync(vargs);            
        }

        private void m_btnClear_Click(object sender, EventArgs e)
        {
            Console_Clear();
        }

        private void m_btnSend_Click(object sender, EventArgs e)
        {
            Input_SendCmd();
        }

        #region ConsoleText

        private void Console_Println(string text)
        {            
            m_tbConsole.AppendText(text + Environment.NewLine);
        }

        private void Console_Clear()
        {
            m_tbConsole.Text = string.Empty;
        }

        #endregion

        void IResponseHandler.OnError(string msg, int id)
        {         
            Console_Println("Call failed for " + id + ": " + msg);            
        }

        public void HandleReadEEPromResult(JsonObject responseObj)
        {
			var filePath = responseObj["filepath"];
			Console_Println("Stored EEPROM at " + filePath);

			String viewerPath = Util.FindExecutable("SharpEEpromViewer");
			if (viewerPath == null)
			{
				MessageBox.Show("Cannot locate SharpEEPromViewer.exe", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}

			var process = new System.Diagnostics.Process();
			process.StartInfo.FileName = viewerPath;
			process.StartInfo.Arguments = filePath;
			process.StartInfo.UseShellExecute = false;

			process.Start();
		}

        void IResponseHandler.OnResponse(JsonValue response, int id)
        {            
            if (response.JsonType == JsonType.Object)
            {
                var responseObj = (JsonObject)response;

                switch ((string)responseObj["classname"])
                {
                    case "ChildItem":
                        Console_Println("Contents of " + responseObj["location"]);
                        {
                            var items = (JsonArray)responseObj["children"];
                            foreach (var item in items)
                            {
                                Console_Println(item["name"]);
                            }
                        }
                        break;

                    case "CmdList":
                        {
                            var items = (JsonArray)responseObj["cmds"];
                            foreach (var item in items)
                            {
                                Console_Println(item["name"]);
                            }
                        }
                        break;

                    case "Location":
                        Console_Println(responseObj["location"]);
                        break;

                    case "ReadEEPromResult":                        
                        {
                            HandleReadEEPromResult(responseObj);
                        }

                        break;

                    default:
                        Console_Println(response.ToString());
                        break;
                }
            }
            else
            {
                Console_Println(response.ToString());
            }            
        }

        //
        //
        //
        //
        //
        //
        #region KnownCmds

        void KnownCmds_AddDefaultCmds()
        {
            mKnownCmds.Add("/clear");
            mKnownCmds.Add("/disconnect");
            mKnownCmds.Add("/mac");
            mKnownCmds.Add("/quit");
            mKnownCmds.Add("/reconnect");
            mKnownCmds.Add("/udpping");            
        }

        void KnownCmds_RetrieveRemoteCmds()
        {
            Console_Println("Requesting remote cmds");

            mRequestManager.DispatchRequest(new[] { "Get-Command" }, new CmdListRetriever(this));
        }

        void KnownCmds_ClearRemoteCmds()
        {
            mKnownCmds = mKnownCmds.Where(x => x.StartsWith("/")).ToList();
        }

        class CmdListRetriever : IResponseHandler
        {
            ConsoleUserControl mOwner;

            public CmdListRetriever(ConsoleUserControl owner)
            {
                mOwner = owner;
            }

            void IResponseHandler.OnError(string msg, int id)
            {
                mOwner.Console_Println("Failed to retrieve command list: " + msg);
            }

            void IResponseHandler.OnResponse(JsonValue response, int id)
            {
                var responseObj = (JsonObject)response;
                var items = (JsonArray)responseObj["cmds"];
                foreach (var item in items)
                {
                    mOwner.mKnownCmds.Add(item["name"]);
                }

                mOwner.Console_Println("Received " + items.Count + " remote commands");
            }
        }
        #endregion

        private void UdpPing(object[] vargs)
        {
            string ip = vargs.Length > 1 ? vargs[1].ToString() : "127.0.0.1";
            int port = vargs.Length > 2 ? int.Parse(vargs[2].ToString()) : 8989;

            using (var client = new System.Net.Sockets.UdpClient())
            {
                var ep = new System.Net.IPEndPoint(System.Net.IPAddress.Parse(ip), port); // endpoint where server is listening
                client.Connect(ep);

                // send data
                client.Send(new byte[] { 104, 101, 108, 108, 111, 0 }, 6);
            }
        }

        private void GenerateMac()
        {
            //http://www.noah.org/wiki/MAC_address
            //02:00:00:00:00:00, and then logically AND it with fe: ff: ff: ff: ff: ff            

            byte[] mac = new byte[6];

            var guid = Guid.NewGuid().ToByteArray();

            for (int i = 0; i < 6; ++i)
            {
                mac[i] = guid[i];
                mac[i] = (byte)(mac[i] | ((i == 0) ? 0x2 : 0x0));
                mac[i] = (byte)(mac[i] & ((i == 0) ? 0xfe : 0xff));
            }

            Console_Println(String.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
            Console_Println(String.Format("{0}:{1}:{2}:{3}:{4}:{5}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
        }

        private void mRequestManager_ConnectionStateChanged(RequestManager sender, ConnectionStateEventArgs args)
        {
            this.OnConnectionStatusChanged(args.State, args.Exception);
        }

        private void OnConnectionStatusChanged(ConnectionState state, Exception ex)
        {            
            switch (state)
            {
                case ConnectionState.OK:                        
                    KnownCmds_RetrieveRemoteCmds();
                    break;

                case ConnectionState.DISCONNECTED:
                    KnownCmds_ClearRemoteCmds();
                    break;
            }            
        }

        void IConsole.Println(string text)
        {
            Console_Println(text);
        }

        void IConsole.Clear()
        {
            Console_Clear();
        }
    }
}

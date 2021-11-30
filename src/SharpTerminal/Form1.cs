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
using System.Json;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class Console : Form
    {
        readonly IConsole mConsole;

        private readonly RequestManager mRequestManager = new RequestManager();        

        public Console()
        {
            InitializeComponent();

            mConsole = ucConsole;
            ucConsole.RequestManager = mRequestManager;
            ucTreeView.RequestManager = mRequestManager;
            RemoteObjectManager.SetRequestManager(mRequestManager);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            using (var dialog = new ServerSelectionForm())
            {
                dialog.ShowDialog();
            }

            mRequestManager.ConnectionStateChanged += mRequestManager_ConnectionStateChanged;
            mRequestManager.BeginConnect("localhost", 4191);

            SetStatus("Connecting");            
        }        

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);

            //avoid deadlock on invoke, so we disable the listenning
            mRequestManager.ConnectionStateChanged -= mRequestManager_ConnectionStateChanged;
            ucConsole.RequestManager = null;
            ucTreeView.RequestManager = null;

            //close connections            
            //mRequestManager.Stop();
            //mRequestManager.Dispose();            
        }        

        private void mRequestManager_ConnectionStateChanged(RequestManager sender, ConnectionStateEventArgs args)
        {
            this.OnConnectionStatusChanged(args.State, args.Exception);
        }

        private void OnConnectionStatusChanged(ConnectionState state, Exception ex)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.OnConnectionStatusChanged(state, ex); }));
            }
            else
            {
                switch(state)
                {
                    case ConnectionState.OK:
                        var label = "Connected";

                        mConsole.Println(label);                        
                        this.SetStatus(label);
                        break;

                    case ConnectionState.DISCONNECTED:
                        mConsole.Println("Disconnected " + (ex != null ? ex.Message : " by unknown reason"));
                        this.SetStatus("Disconnected");
                        break;

                    default:
                        mConsole.Println("Connection state changed to " + state + " " + (ex != null ? ex.Message : " by unknown reason"));
                        this.SetStatus(state.ToString());
                        break;
                }                
            }
        }

        #region StatusBar
        private void SetStatus(string text)
        {
            m_lbStatus.Text = text;
        }
        #endregion
    }
}

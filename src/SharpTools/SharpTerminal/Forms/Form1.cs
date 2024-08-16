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
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public partial class Console : Form
    {
        readonly IConsole mConsole;

        private RequestManager mRequestManager;

        String  m_strParamServer;
        ushort  m_uParamPort; 

        public Console(String[] args = null)
        {
            InitializeComponent();

            if((args != null) && (args.Length== 2))
            {
                m_strParamServer = args[0];
                m_uParamPort = ushort.Parse(args[1]);
            }

			mConsole = ucConsole;

			this.ConfigureRequestManager();
        }

        private void ConfigureRequestManager()
        {
            if (mRequestManager != null)
            {
                mRequestManager.ConnectionStateChanged -= mRequestManager_ConnectionStateChanged;
            }

			//Creates here, so it has a SyncContext
			mRequestManager = new();
			
			ucConsole.RequestManager = mRequestManager;

			ucTreeView.RequestManager = mRequestManager;
			ucTreeView.Console = mConsole;

			mRequestManager.ConnectionStateChanged += mRequestManager_ConnectionStateChanged;

			RemoteObjectManager.SetRequestManager(mRequestManager);
		}

        private bool DisplayServerSelectionForm()
        {
			using var dialog = new ServerSelectionForm(m_strParamServer, m_uParamPort);

			if (dialog.ShowDialog() == DialogResult.Cancel)
			{
				this.Close();
				return false;
			}

			m_strParamServer = dialog.mSelectedService.mAddress.ToString();
			m_uParamPort = dialog.mSelectedService.mPort;

            return true;
		}

        private void DoAutoReconnect()
        {
            if (!DisplayServerSelectionForm())
                return;

            mRequestManager.Dispose();

            this.ConfigureRequestManager();

			SetStatus("Connecting");
			mRequestManager.BeginConnect(m_strParamServer, m_uParamPort);
		}

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            if((m_strParamServer == null) && !this.DisplayServerSelectionForm())
			{                
                this.Close();

                return;
            }            
            
            mRequestManager.BeginConnect(m_strParamServer, m_uParamPort);

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

						this.DoAutoReconnect();
						break;

                    case ConnectionState.ERROR:
						mConsole.Println("Connection error " + (ex != null ? ex.Message : " by unknown reason"));
						this.SetStatus("Connection error");

                        this.DoAutoReconnect();
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

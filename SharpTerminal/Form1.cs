
using System;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class Form1 : Form, ITerminalClientListener
    {
        private TerminalClient mClient = new TerminalClient();

        public Form1()
        {
            InitializeComponent();

            mClient.Listener = this;
            mClient.BeginConnect("localhost", 4190);            
        }

        public void OnConnected(ConnectionStatus status, object param)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.OnConnected(status, param); }));
            }
            else
            {
                m_tbConsole.Text += "Connected";
            }
        }
    }
}


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

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);

            mClient.Stop();
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
                m_tbInput.Enabled = true;
            }
        }

        private void m_tbInput_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                var text = m_tbInput.Text.Trim();
                m_tbInput.Text = string.Empty;

                if(!string.IsNullOrWhiteSpace(text))
                    mClient.SendMessage(text);
            }
        }
    }
}

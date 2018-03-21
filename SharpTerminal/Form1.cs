
using System;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class Form1 : Form, ITerminalClientListener
    {
        private TerminalClient mClient = new TerminalClient();

        private int m_iRequestCount = 1;

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
                var text = m_tbInput.Text;
                m_tbInput.Text = string.Empty;
                
                ProcessInput(text);
            }
        }

        private void ProcessLocalCmd(string input)
        {
            switch(input)
            {
                case "/quit":
                    this.Close();
                    break;

                default:
                    MessageBox.Show("Unknown cmd " + input);
                    break;
            }
        }

        private void DispatchJsonCmd(string input)
        {
            var rawCmd = input.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            var strBuilder = new StringBuilder(input.Length + 32);

            strBuilder.Append("{\n\t\"jsonrpc\":\"2.0\",\n\t \"method\":\"");
            strBuilder.Append(rawCmd[0]);
            strBuilder.Append("\"");

            if(rawCmd.Length > 1)
            {
                strBuilder.Append(",\n\t\"params\":[\"");

                bool first = true;
                for(int i = 1;i < rawCmd.Length; ++i)
                {                     
                    strBuilder.Append(first ? "" : ",\"");
                    first = false;

                    strBuilder.Append(rawCmd[i]);
                    strBuilder.Append("\"");
                }

                strBuilder.Append("]");
            }

            strBuilder.Append(",\n\t\"id\":");
            strBuilder.Append(m_iRequestCount++);

            strBuilder.Append("}");

            mClient.SendMessage(strBuilder.ToString());
        }

        private void ProcessInput(string input)
        {
            input = input.Trim();
            if (string.IsNullOrWhiteSpace(input))
                return;

            //local command?
            if(input[0] == '/')
            {
                ProcessLocalCmd(input);
            }
            else
            {
                DispatchJsonCmd(input);
            }
        }
    }
}

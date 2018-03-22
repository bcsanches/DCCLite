
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

            SetStatus("Connecting");

            m_tbInput.Select();
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);

            //avoid deadlock on invoke, so we disable the listenning
            mClient.Listener = null;

            //close connections
            mClient.Stop();
        }

        public void OnStatusChanged(ConnectionState state, object param)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.OnStatusChanged(state, param); }));
            }
            else
            {
                switch(state)
                {
                    case ConnectionState.OK:
                        var label = "Connected";

                        this.Console_Println(label);
                        this.SetStatus(label);
                        break;

                    case ConnectionState.DISCONNECTED:
                        var ex = param as Exception;                        

                        this.Console_Println("Disconnected " + (ex != null ? ex.Message : " by unknown reason"));
                        this.SetStatus("Disconnected");
                        break;

                    default:
                        this.Console_Println("Connection state changed to " + state);
                        this.SetStatus(state.ToString());
                        break;

                }                
            }
        }

        private void SendCmd()
        {
            var text = m_tbInput.Text.Trim();
            m_tbInput.Text = string.Empty;

            if (string.IsNullOrWhiteSpace(text))
                return;

            Console_Println("> " + text);

            ProcessInput(text);
        }

        private void m_tbInput_KeyUp(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                SendCmd();
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
                    Console_Println("Unknown local command " + input);
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

        private void m_btnClear_Click(object sender, EventArgs e)
        {
            Console_Clear();
        }

        private void m_btnSend_Click(object sender, EventArgs e)
        {
            SendCmd();
        }

        private void m_btnQuit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        #region StatusBar
        private void SetStatus(string text)
        {
            m_lbStatus.Text = text;
        }
        #endregion

        #region ConsoleText

        private void Console_Println(string text)
        {
            m_tbConsole.Text += text + Environment.NewLine;
        }

        private void Console_Clear()
        {
            m_tbConsole.Text = string.Empty;
        }

        #endregion
    }
}

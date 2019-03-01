
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class Console : Form, ITerminalClientListener
    {
        private TerminalClient mClient = new TerminalClient();

        private List<string> mUsedCmds = new List<string>();
        private int m_iCurrentCmd = 0;

        private List<string> mKnownCmds = new List<string>();

        private int m_iRequestCount = 1;

        public Console()
        {
            InitializeComponent();

            mClient.Listener = this;
            mClient.BeginConnect("localhost", 4190);

            SetStatus("Connecting");

            m_tbInput.Select();

            mKnownCmds.Add("/clear");
            mKnownCmds.Add("/disconnect");
            mKnownCmds.Add("/quit");
            mKnownCmds.Add("/reconnect");
            mKnownCmds.Add("/udpping");
        }

        protected override void OnFormClosed(FormClosedEventArgs e)
        {
            base.OnFormClosed(e);

            //avoid deadlock on invoke, so we disable the listenning
            mClient.Listener = null;

            //close connections
            mClient.Stop();
        }

        public void OnMessageReceived(string msg)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.OnMessageReceived(msg); }));
            }
            else
            {
                Console_Println(msg);
            }
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
                        ex = param as Exception;
                        this.Console_Println("Connection state changed to " + state + " " + (ex != null ? ex.Message : " by unknown reason"));
                        this.SetStatus(state.ToString());
                        break;
                }                
            }
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
            switch(e.KeyCode)
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
                    if(m_iCurrentCmd > 1)
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
                        if(!string.IsNullOrWhiteSpace(result))
                        {
                            Input_Set(result);
                        }
                    }
                    break;

            }            
        }

        private void GenerateMac()
        {
            //http://www.noah.org/wiki/MAC_address
            //02:00:00:00:00:00, and then logically AND it with fe: ff: ff: ff: ff: ff            

            byte []mac = new byte[6];

            var guid = Guid.NewGuid().ToByteArray();

            for(int i = 0;i < 6; ++i)
            {
                mac[i] = guid[i];
                mac[i] = (byte)(mac[i] | ((i == 0) ? 0x2 : 0x0));
                mac[i] = (byte)(mac[i] & ((i == 0) ? 0xfe : 0xff));
            }

            Console_Println(String.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
            Console_Println(String.Format("{0}:{1}:{2}:{3}:{4}:{5}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
        }

        private void ProcessLocalCmd(string[] vargs)
        {
            switch(vargs[0])
            {
                case "/clear":
                    Console_Clear();
                    break;

                case "/disconnect":
                    mClient.Stop();
                    break;

                case "/mac":
                    GenerateMac();
                    break;

                case "/quit":
                    Close();
                    break;                

                case "/reconnect":
                    mClient.Reconnect();
                    break;

                case "/udpping":
                    UdpPing(vargs);
                    break;

                default:
                    Console_Println("Unknown local command " + vargs[0]);
                    break;
            }
        }

        private void DispatchJsonCmd(string[] vargs)
        {            
            var strBuilder = new StringBuilder(256);

            strBuilder.Append("{\n\t\"jsonrpc\":\"2.0\",\n\t \"method\":\"");
            strBuilder.Append(vargs[0]);
            strBuilder.Append("\"");

            if(vargs.Length > 1)
            {
                strBuilder.Append(",\n\t\"params\":[\"");

                bool first = true;
                for(int i = 1;i < vargs.Length; ++i)
                {                     
                    strBuilder.Append(first ? "" : ",\"");
                    first = false;

                    strBuilder.Append(vargs[i]);
                    strBuilder.Append("\"");
                }

                strBuilder.Append("]");
            }

            strBuilder.Append(",\n\t\"id\":");
            strBuilder.Append(m_iRequestCount++);

            strBuilder.Append("}");

            mClient.SendMessage(strBuilder.ToString());
        }

        private void ProcessRemoteCmd(string[] vargs)
        {
            if(mClient.State != ConnectionState.OK)
            {
                Console_Println("Cannot send command, client not connected. Try /reconnect");

                return;
            }

            DispatchJsonCmd(vargs);
        }

        private void ProcessInput(string input)
        {
            input = input.Trim();
            if (string.IsNullOrWhiteSpace(input))
                return;

            var vargs = input.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);

            //local command?
            if (input[0] == '/')
            {
                ProcessLocalCmd(vargs);
            }
            else
            {
                ProcessRemoteCmd(vargs);
            }
        }

        private void m_btnClear_Click(object sender, EventArgs e)
        {
            Console_Clear();
        }

        private void m_btnSend_Click(object sender, EventArgs e)
        {
            Input_SendCmd();
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
            if(this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate { this.Console_Println(text); }));
            }
            else
                m_tbConsole.AppendText(text + Environment.NewLine);
        }

        private void Console_Clear()
        {
            m_tbConsole.Text = string.Empty;
        }

        #endregion

        private void UdpPing(string[] vargs)
        {
            string ip = vargs.Length > 1 ? vargs[1] : "127.0.0.1";
            int port = vargs.Length > 2 ? int.Parse(vargs[2]) : 8989;


            var client = new System.Net.Sockets.UdpClient();
            var ep = new System.Net.IPEndPoint(System.Net.IPAddress.Parse(ip), port); // endpoint where server is listening
            client.Connect(ep);

            // send data
            client.Send(new byte[] { 104, 101, 108, 108, 111, 0 }, 6);
        }
    }
}

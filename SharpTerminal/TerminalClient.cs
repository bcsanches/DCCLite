using System;
using System.Net.Sockets;

namespace SharpTerminal
{
    class TerminalClient
    {
        private TcpClient mClient;

        public TerminalClient()
        {
            mClient = new TcpClient();
            mClient.NoDelay = true;            
        }        

        public IAsyncResult BeginConnect(string host, int port)
        {
            return mClient.BeginConnect(host, port, null, null);
        }
    }
}

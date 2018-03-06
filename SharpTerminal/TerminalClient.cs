using System.Net.Sockets;

namespace SharpTerminal
{
    class TerminalClient
    {
        private TcpClient mClient;

        public TerminalClient(string server, int port)
        {
            mClient = new TcpClient(server, port);
        }
    }
}

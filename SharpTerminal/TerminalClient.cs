using System;
using System.Collections.Concurrent;
using System.Net.Sockets;
using System.Threading;

namespace SharpTerminal
{
    public enum ConnectionStatus
    {
        OK,
        DISCONNECTED,
        WOULD_BLOCK,
        ERROR
    }

    class TerminalClient
    {
        private TcpClient mClient;

        public ITerminalClientListener Listener { get; set; }

        private Thread mReceiverThread;
        private Thread mSenderThread;

        private string mHost;
        private int mHostPort;

        private bool mStop;

        private BlockingCollection<string> mSendQueue = new BlockingCollection<string>();

        public TerminalClient()
        {
            mClient = new TcpClient();
            mClient.NoDelay = true;            
        }        

        public void BeginConnect(string host, int port)
        {
            if(mSenderThread != null)
            {
                throw new InvalidOperationException("Sender thread running? WTF");
            }

            if(string.IsNullOrWhiteSpace(host))
            {
                throw new ArgumentNullException(nameof(host));
            }

            mHost = host;
            mHostPort = port;

            mSenderThread = new Thread(SenderWorker);
            mSenderThread.Start();
        }

        public void SendMessage(string msg)
        {
            mSendQueue.Add(msg);
        }

        private void SenderWorker(Object param)
        {
            try
            {
                mClient.Connect(mHost, mHostPort);
            }            
            catch(Exception ex)
            {
                Listener?.OnConnected(ConnectionStatus.ERROR, ex);

                return;
            }

            Listener?.OnConnected(ConnectionStatus.OK, null);

            var stream = mClient.GetStream();

            while(!mStop)
            {
                var str = mSendQueue.Take();

                //maybe this is just our little hack for stopping the thread
                if (string.IsNullOrWhiteSpace(str))
                    continue;

                var data = System.Text.Encoding.ASCII.GetBytes(str);
                stream.Write(data, 0, data.Length);
            }
        }
    }
}

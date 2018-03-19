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

    class TerminalClient: IDisposable
    {
        private TcpClient mClient;

        public ITerminalClientListener Listener { get; set; }

        private Thread mReceiverThread;
        private Thread mSenderThread;

        private string mHost;
        private int mHostPort;

        private CancellationTokenSource mCancellationTokenSource = new CancellationTokenSource();

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

            mReceiverThread = new Thread(ReceiverWorker);
            mSenderThread = new Thread(SenderWorker);
            mSenderThread.Start();
        }

        public void SendMessage(string msg)
        {
            mSendQueue.Add(msg);
        }

        private void ReceiverWorker(Object param)
        {
            var token = mCancellationTokenSource.Token;

            var stream = mClient.GetStream();
            var bytes = new byte[1];


            while (!token.IsCancellationRequested)
            {
                var data = stream.ReadByte();
                if (data < 0)
                {                   
                    Thread.Sleep(100);

                    throw new InvalidOperationException("fixme");                    
                }

                bytes[0] = (byte)data;
                var ch = System.Text.Encoding.ASCII.GetChars(bytes);
            }
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

            var cancellationToken = mCancellationTokenSource.Token;

            System.Diagnostics.Debug.Assert(mReceiverThread != null);
            mReceiverThread.Start();

            var stream = mClient.GetStream();

            try
            {
                for (; ; )
                {
                    var str = mSendQueue.Take(cancellationToken);

                    //maybe this is just our little hack for stopping the thread
                    if (string.IsNullOrWhiteSpace(str))
                        continue;

                    var data = System.Text.Encoding.ASCII.GetBytes(str);
                    stream.Write(data, 0, data.Length);
                }
            }
            catch(OperationCanceledException)
            {
                //ignore
            }
        }

        public void Stop()
        {
            mCancellationTokenSource.Cancel();            

            if(mSenderThread.ThreadState == ThreadState.Running)
                mSenderThread.Join();

            if (mReceiverThread.ThreadState == ThreadState.Running)
                mReceiverThread.Join();

            mClient.Close();
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    mCancellationTokenSource.Dispose();
                    mClient.Dispose();
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.
                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        // ~TerminalClient() {
        //   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
        //   Dispose(false);
        // }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            // GC.SuppressFinalize(this);
        }
        #endregion
    }
}

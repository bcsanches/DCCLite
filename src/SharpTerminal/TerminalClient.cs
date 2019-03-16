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
using System.Collections.Concurrent;
using System.Net.Sockets;
using System.Threading;

namespace SharpTerminal
{
    public enum ConnectionState
    {
        OK,
        CONNECTING,
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
            if(string.IsNullOrWhiteSpace(host))
            {
                throw new ArgumentNullException(nameof(host));
            }

            mHost = host;
            mHostPort = port;

            Connect();
        }

        public void Reconnect()
        {
            Stop();

            mClient = new TcpClient();
            mClient.NoDelay = true;

            Connect();
        }

        private void Connect()
        {
            if (mSenderThread != null)
            {
                throw new InvalidOperationException("Sender thread running? WTF");
            }

            if (mReceiverThread != null)
            {
                throw new InvalidOperationException("mReceiverThread running? WTF");
            }
            
            mSenderThread = new Thread(SenderWorker);
            mSenderThread.Start();

            SetState(ConnectionState.CONNECTING, null);
        }

        public void SendMessage(string msg)
        {
            mSendQueue.Add(msg);
        }

        private void IOThreadFailure(object param)
        {            
            SetState(ConnectionState.ERROR, param);
        }

        private void ReceiverWorker(Object param)
        {
            var token = mCancellationTokenSource.Token;

            var stream = mClient.GetStream();
            var bytes = new byte[1];
            var stringBuilder = new System.Text.StringBuilder(128);

            bool lastCharWasCarriageReturn = false;

            while (!token.IsCancellationRequested)
            {
                int data = 0;
                try
                {
                    data = stream.ReadByte();
                }
                catch(System.IO.IOException ex)
                {
                    if(!token.IsCancellationRequested)
                    {
                        IOThreadFailure(ex);
                        break;
                    }

                    continue;
                }

                if (data < 0)
                {
                    IOThreadFailure(null);
                    break;
                }

                bytes[0] = (byte)data;
                var ch = System.Text.Encoding.ASCII.GetChars(bytes);
                stringBuilder.Append(ch);

                if (ch[0] == '\r')
                    lastCharWasCarriageReturn = true;
                else if ((ch[0] == '\n') && (lastCharWasCarriageReturn))
                {
                    lastCharWasCarriageReturn = false;

                    var msg = stringBuilder.ToString();
                    stringBuilder.Clear();

                    //empty msg?
                    if (msg == "\r\n")
                        continue;

                    Listener?.OnMessageReceived(msg);                    
                }
                else
                {
                    //always clear 
                    lastCharWasCarriageReturn = false;
                }                    
            }

            mReceiverThread = null;
        }

        private void SenderWorker(Object param)
        {
            try
            {
                mClient.Connect(mHost, mHostPort);
            }            
            catch(Exception ex)
            {
                SetState(ConnectionState.ERROR, ex);
                mSenderThread = null;

                return;
            }

            SetState(ConnectionState.OK, null);            

            var cancellationToken = mCancellationTokenSource.Token;

            mReceiverThread = new Thread(ReceiverWorker);
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

                    if (!str.EndsWith("\r\n"))
                        str += "\r\n";

                    var data = System.Text.Encoding.ASCII.GetBytes(str);
                    stream.Write(data, 0, data.Length);
                }
            }
            catch(System.IO.IOException ex)
            {
                IOThreadFailure(ex);
            }
            catch (OperationCanceledException)
            {
                //ignore
            }

            mSenderThread = null;
        }

        private void JoinWorkerThread(Thread t)
        {
            if ((t != null) && (Thread.CurrentThread != t) && (t.ThreadState == ThreadState.Running))
            {
                t.Join();
            }
        }        

        public void Stop()
        {
            mCancellationTokenSource.Cancel();

            mClient.Close();

            JoinWorkerThread(mSenderThread);
            JoinWorkerThread(mReceiverThread);

            mCancellationTokenSource = new CancellationTokenSource();

            SetState(ConnectionState.DISCONNECTED, null);
        }

        #region ConnectionState

        private ConnectionState mState;

        public ConnectionState State
        {
            get { return mState; }
        }

        private void SetState(ConnectionState newState, object param)
        {
            lock(this)
            {
                if (newState == mState)
                    return;                

                mState = newState;

                Listener?.OnStatusChanged(mState, param);
            }
        }

        #endregion

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

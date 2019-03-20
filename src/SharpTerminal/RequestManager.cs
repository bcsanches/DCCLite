using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SharpTerminal
{
    public class ConnectionStateEventArgs: EventArgs
    {
        public ConnectionStateEventArgs(ConnectionState state, Exception ex)
        {
            State = state;
            Exception = ex;
        }

        public ConnectionState State { get; private set; }
        public Exception Exception { get; private set; }
    }

    public interface IResponseHandler
    {
        void OnResponse(string msg, int id);
    }

    delegate void ConnectionStateChangedEventHandler(RequestManager sender, ConnectionStateEventArgs args);

    class RequestManager: ITerminalClientListener, IDisposable
    {
        TerminalClient mClient = new TerminalClient();

        private int m_iRequestCount = 1;        

        public event ConnectionStateChangedEventHandler ConnectionStateChanged;

        public RequestManager()
        {
            //empty
        }

        public ConnectionState State
        {
            get { return mClient.State; }
        }

        public void BeginConnect(string host, int port)
        {
            mClient.BeginConnect(host, port);
        }

        public void Stop()
        {
            mClient.Stop();
        }

        public void Reconnect()
        {
            mClient.Reconnect();
        }

        public void OnStatusChanged(ConnectionState state, object param)
        {
            if(ConnectionStateChanged != null)
            { 
                var args = new ConnectionStateEventArgs(state, param as Exception);

                ConnectionStateChanged(this, args);
            }
        }

        public void OnMessageReceived(string msg)
        {
            if(mHandler != null)
                mHandler.OnResponse(msg, -1);
        }

        //FIXME hacl
        private IResponseHandler mHandler;

        public int DispatchRequest(string[] vargs, IResponseHandler handler)
        {
            var strBuilder = new StringBuilder(256);

            strBuilder.Append("{\n\t\"jsonrpc\":\"2.0\",\n\t \"method\":\"");
            strBuilder.Append(vargs[0]);
            strBuilder.Append("\"");

            if (vargs.Length > 1)
            {
                strBuilder.Append(",\n\t\"params\":[\"");

                bool first = true;
                for (int i = 1; i < vargs.Length; ++i)
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

            mHandler = handler;
            mClient.SendMessage(strBuilder.ToString());

            return m_iRequestCount - 1;
        }

        public void Dispose()
        {
            ((IDisposable)mClient).Dispose();
        }
    }
}

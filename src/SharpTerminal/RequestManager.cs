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
using System.Collections.Generic;
using System.Linq;
using System.Json;
using System.Text;
using System.Threading;
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

    public class RpcNotificationEventArgs : EventArgs
    {
        public RpcNotificationEventArgs(JsonValue notification)
        {
            Notification = notification;            
        }

        public JsonValue Notification;        
    }

    public interface IResponseHandler
    {
        void OnResponse(JsonValue response, int id);
        void OnError(string msg, int id);
    }

    internal class TaskReponseHandler : IResponseHandler, IDisposable
    {
        EventWaitHandle mWaitHandler = new EventWaitHandle(false, EventResetMode.ManualReset);

        string mErrorMessage;
        JsonValue mResponse;

        public JsonValue DoTask(RequestManager manager, string []args)
        {
            manager.DispatchRequest(args, this);            

            mWaitHandler.WaitOne();            

            if (mErrorMessage != null)
                throw new Exception(mErrorMessage);            

            return mResponse;
        }

        public void OnError(string msg, int id)
        {
            mErrorMessage = msg;

            mWaitHandler.Set();
        }

        public void OnResponse(JsonValue response, int id)
        {
            mResponse = response;

            mWaitHandler.Set();
        }

        public void Dispose()
        {
            ((IDisposable)mWaitHandler).Dispose();
        }
    }

    public delegate void ConnectionStateChangedEventHandler(RequestManager sender, ConnectionStateEventArgs args);
    public delegate void RpcNotificationArrivedEventHandler(RequestManager sender, RpcNotificationEventArgs args);

    public class RequestManager: ITerminalClientListener, IDisposable
    {
        struct RequestInfo
        {
            public IResponseHandler mHandler;

            public DateTime mTime;

            public RequestInfo(IResponseHandler handler)
            {
                mHandler = handler ?? throw new ArgumentNullException(nameof(handler));
                mTime = DateTime.Now + new TimeSpan(0, 0, 3);
            }
        }

        readonly TerminalClient mClient = new TerminalClient();

        private int m_iRequestCount = 1;

        readonly Dictionary<int, RequestInfo> mRequests = new Dictionary<int, RequestInfo>();

        public event ConnectionStateChangedEventHandler ConnectionStateChanged;
        public event RpcNotificationArrivedEventHandler RpcNotificationArrived;

        public RequestManager()
        {
            mClient.Listener = this;
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
            var jsonObject = (JsonObject) JsonObject.Parse(msg);

            if (!jsonObject.TryGetValue("id", out var idValue))
            {
                if(RpcNotificationArrived != null)
                {
                    var args = new RpcNotificationEventArgs(jsonObject);
                    RpcNotificationArrived(this, args);
                }

                return;
            }
            
            int id = int.Parse(idValue.ToString());

            lock(this)
            {                
                if (!mRequests.TryGetValue(id, out var request))
                {
                    throw new Exception("Request response " + id + " not found, data: " + msg);
                }

                if(jsonObject.TryGetValue("error", out JsonValue errorValue))
                {
                    request.mHandler.OnError(((JsonObject)errorValue)["message"].ToString(), id);
                }
                else
                {
                    request.mHandler.OnResponse(jsonObject["result"], id);
                }
                
                mRequests.Remove(id);

                var expiredCalls = mRequests.Where(pair => pair.Value.mTime <= DateTime.Now).Select(pair => pair).ToArray();

                foreach(var pair in expiredCalls)
                {
                    pair.Value.mHandler.OnError("timeout", pair.Key);

                    mRequests.Remove(pair.Key);
                }
            }            
        }        

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

            lock(this)
            {
                int requestId = m_iRequestCount;
                ++m_iRequestCount;

                strBuilder.Append(",\n\t\"id\":");
                strBuilder.Append(requestId);

                strBuilder.Append("}");

                mRequests.Add(requestId, new RequestInfo(handler));
                mClient.SendMessage(strBuilder.ToString());

                return requestId;
            }            
        }

        public async Task<JsonValue> RequestAsync(string[] vargs)
        {
            using (var handler = new TaskReponseHandler())
            {
                var task = new Task<JsonValue>(() => { return handler.DoTask(this, vargs); });

                task.Start();

                return await task;
            }                           
        }

        public void Dispose()
        {
            ((IDisposable)mClient).Dispose();
        }
    }
}



namespace SharpTerminal
{
    interface ITerminalClientListener
    {
        void OnStatusChanged(ConnectionState state, object param);

        void OnMessageReceived(string msg);
    }
}

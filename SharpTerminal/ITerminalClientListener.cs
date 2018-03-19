

namespace SharpTerminal
{
    interface ITerminalClientListener
    {
        void OnConnected(ConnectionStatus status, object param);
    }
}

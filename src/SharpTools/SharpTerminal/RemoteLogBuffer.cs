using System;
using System.Globalization;

namespace SharpTerminal
{
	internal sealed class MsgArrivedEventArgs : EventArgs
	{
		public MsgArrivedEventArgs(LogMsg msg)
		{
			Msg = msg;
		}

		public LogMsg Msg { get; }
	}

	struct LogMsg
	{
		public LogMsg(string msg)
		{
			Msg = msg;

			var end = msg.IndexOf(']');
			var start = msg.IndexOf('[');
			
			if ((start >= 0) && (end > 0))
			{
				var r = DateTime.TryParseExact(
					msg.AsSpan(start + 1, end - start - 1),
					"yyyy-MM-dd HH:mm:ss.fff",
					CultureInfo.InvariantCulture,
					DateTimeStyles.None,
					out DateTime timestamp
				);

				//remove out date / time
				Msg = msg.Substring(end + 1);

				if(r)
					Time = timestamp;
			}
		}

		public string Msg { get; }
		public DateTime Time { get; }
	}

	internal static class RemoteLogBuffer
	{
		const int MAX_LINES = 1024;

		private static RequestManager gRequestManager;
		private static LogMsg[]gBuffer = new LogMsg[MAX_LINES];

		private static uint gNextIndex = 0;

		public static event EventHandler<MsgArrivedEventArgs> MsgArrived;

		public static void SetRequestManager(RequestManager requestManager)
		{
			if(gRequestManager != null)
			{
				gRequestManager.RpcNotificationArrived -= RequestManager_RpcNotificationArrived;
			}

			ArgumentNullException.ThrowIfNull(requestManager);
			gRequestManager = requestManager;

			gRequestManager.RpcNotificationArrived += RequestManager_RpcNotificationArrived;
		}

		private static void RequestManager_RpcNotificationArrived(object sender, RpcNotificationEventArgs e)
		{
			if (e.Notification == null)
				return;

			if (e.Notification["method"] != "On-LogMsg")
				return;

			var parameters = e.Notification["params"];
			var msg = (string)parameters?["msg"];

			var logMsg = new LogMsg(msg);
			gBuffer[gNextIndex % MAX_LINES] = logMsg;
			++gNextIndex;

			MsgArrived?.Invoke(null, new MsgArrivedEventArgs(logMsg));
		}

		public static LogMsg[] GetMsgs()
		{
			var count = Math.Min(gNextIndex, MAX_LINES);
			if(count == 0)
			{
				return [];
			}

			var result = new LogMsg[count];

			for(int i = 0;i < count; ++i)
			{
				result[i] = gBuffer[(gNextIndex - count + i) % MAX_LINES];
			}

			return result;
		}
	}
}

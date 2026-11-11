using System;

namespace SharpTerminal
{
	internal static class RemoteLogBuffer
	{
		const int MAX_LINES = 1024;

		private static RequestManager gRequestManager;
		private static string []gBuffer = new string[MAX_LINES];

		private static uint gNextIndex = 0;

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
			var msg = parameters?["msg"].ToString();

			gBuffer[gNextIndex % MAX_LINES] = msg;
			++gNextIndex;
		}

		public static string[] GetMsgs()
		{
			var count = Math.Min(gNextIndex, MAX_LINES);
			if(count == 0)
			{
				return [];
			}

			var result = new string[count];

			for(int i = 0;i < count; ++i)
			{
				result[i] = gBuffer[(gNextIndex - count + i) % MAX_LINES];
			}

			return result;
		}
	}
}

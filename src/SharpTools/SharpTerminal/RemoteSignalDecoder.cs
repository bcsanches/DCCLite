
using System;
using System.ComponentModel;
using System.Json;
using System.Runtime.Versioning;

namespace SharpTerminal
{
	[SupportedOSPlatform("windows")]
	public class RemoteSignalDecoder : RemoteDecoder
	{
		public String[] m_Aspects;
		public string m_strCurrentAspect;
		public string m_strRequestedAspect;

		public string m_strAspectReason;
		public string m_strAspectRequester;

		IRemoteObjectAction[] m_arActions;

		public RemoteSignalDecoder(string name, string className, string path, ulong internalId, JsonValue objectDef, RemoteFolder parent) :
			base(name, className, path, internalId, objectDef, parent)
		{
			this.ParseStateData(objectDef);

			var aspectsData = objectDef["aspects"];

			m_Aspects = new string[aspectsData.Count];
			for (var i = 0; i < m_Aspects.Length; ++i)
				m_Aspects[i] = aspectsData[i];
		}

		protected override void OnUpdateState(JsonValue objectDef)
		{
			base.OnUpdateState(objectDef);

			this.ParseStateData(objectDef);
		}

		private void ParseStateData(JsonValue objectDef)
		{
			CurrentAspect = (String)objectDef["currentAspectName"];
			RequestedAspect = (String)objectDef["requestedAspectName"];
			AspectRequester = (String)objectDef["aspectRequester"];
			AspectReason = (String)objectDef["aspectReason"];
		}

		public override string TryGetIconName()
		{
			return DefaultIcons.SIGNAL_ICON;
		}

		[Category("Aspect")]
		public string CurrentAspect
		{
			get { return m_strCurrentAspect; }

			set
			{
				this.UpdateProperty(ref m_strCurrentAspect, value);
			}
		}

		[Category("Aspect")]
		public string RequestedAspect
		{
			get { return m_strRequestedAspect; }

			set
			{
				this.UpdateProperty(ref m_strRequestedAspect, value);
			}
		}

		[Category("Aspect")]
		public string AspectRequester
		{
			get { return m_strAspectRequester; }
			set
			{
				this.UpdateProperty(ref m_strAspectRequester, value);
			}
		}

		[Category("Aspect")]
		public string AspectReason
		{
			get { return m_strAspectReason; }
			set
			{
				this.UpdateProperty(ref m_strAspectReason, value);
			}
		}

		public override IRemoteObjectAction[] GetActions()
		{
			if (m_arActions == null)
			{
				m_arActions = new IRemoteObjectAction[m_Aspects.Length];

				for (int i = 0; i < m_Aspects.Length; i++)
				{
					var aspect = m_Aspects[i];
					m_arActions[i] = new RemoteServiceObjectCmdAction("Set-Aspect", aspect, "Set " + aspect + " aspect", aspect);
				}
			}

			return m_arActions;
		}
	}

}

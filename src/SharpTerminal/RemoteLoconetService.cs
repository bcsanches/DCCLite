using System;
using System.Json;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteLoconetSlot: NotifyPropertyBase
    {        
        public int Index
        {
            get { return m_iIndex; }
            set
            {
                this.UpdateProperty(ref m_iIndex, value);
            }
        }

        public int LocomotiveAddress
        {
            get
            {
                return m_iLocomotiveAddress;
            }

            set
            {
                this.UpdateProperty(ref m_iLocomotiveAddress, value);
            }
        }

        public int Speed
        {
            get
            {
                return m_iSpeed;

            }
            set
            {
                this.UpdateProperty(ref m_iSpeed, value);
            }
        }

        public bool Forward
        {
            get
            {
                return m_fForward;
            }
            set
            {
                this.UpdateProperty(ref m_fForward, value);
            }
        }

        public String State
        {
            get
            {
                return m_strState;
            }

            set
            {
                this.UpdateProperty(ref m_strState, value);
            }
        }
        

        public RemoteLoconetSlot(int index, string state, int locomotiveAddress, int speed, bool forward)
        {
            Index = index;
            State = state;
            LocomotiveAddress = locomotiveAddress;
            Speed = speed;
            Forward = forward;
        }

        private int m_iLocomotiveAddress;
        private int m_iSpeed;
        private bool m_fForward;
        private string m_strState;
        private int m_iIndex;
    }

    public class RemoteLoconetService: RemoteFolder
    {                   
        public RemoteLoconetSlot[] Slots;

        public RemoteLoconetService(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            this.UpdateState(objectDef);
        }

        protected override void OnUpdateState(JsonValue objectDef)
        {
            base.OnUpdateState(objectDef);

            if (objectDef.ContainsKey("slot"))
            {
                var slotDeltaData = objectDef["slot"];
                int index = (int)slotDeltaData["index"];

                var slotDef = slotDeltaData["data"];

                var slot = Slots[index];

                slot.State = slotDef["state"];
                slot.LocomotiveAddress = (int)slotDef["locomotiveAddress"];
                slot.Speed = (int)slotDef["speed"];
                slot.Forward = (bool)slotDef["forward"];
            }
            else if (objectDef.ContainsKey("slots"))
            {
                var slotsData = objectDef["slots"];

                if (slotsData.Count == 0)
                {
                    return;
                }

                Slots = new RemoteLoconetSlot[slotsData.Count];

                for (int i = 0; i < slotsData.Count; ++i)
                {
                    var slotDef = slotsData[i];

                    var state = slotDef["state"];
                    var locomotiveAddress = (int)slotDef["locomotiveAddress"];
                    var speed = (int)slotDef["speed"];
                    var forward = (bool)slotDef["forward"];

                    Slots[i] = new RemoteLoconetSlot(i, state, locomotiveAddress, speed, forward);
                }
            }
        }
        
        public override Control CreateControl()
        {
            return new RemoteLoconetServiceUserControl(this, Slots);
        }
    }
}

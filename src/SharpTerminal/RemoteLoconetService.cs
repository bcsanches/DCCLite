using System;
using System.Json;
using System.Text;
using System.Windows.Forms;

namespace SharpTerminal
{    
    public class RemoteLoconetSlot: NotifyPropertyBase    
    {
        public const int MAX_FUNCTIONS = 32;

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

        public string FunctionsLabel
        {
            get
            {
                StringBuilder builder = new StringBuilder();

                string separator = "";
                for(int i = 0; i < m_fFunctions.Length; ++i)
                {
                    if (!m_fFunctions[i])
                        continue;

                    builder.Append(separator);
                    builder.Append("F" + i.ToString());

                    separator = ", ";
                }

                return builder.ToString();
            }
        }

        public bool []Functions
        {
            set
            {
                m_fFunctions = value;

                this.OnPropertyUpdated(nameof(OnPropertyUpdated));
            }
        }
        

        public RemoteLoconetSlot(int index, string state, int locomotiveAddress, int speed, bool forward, bool []functions)
        {
            Index = index;
            State = state;
            LocomotiveAddress = locomotiveAddress;
            Speed = speed;
            Forward = forward;
            m_fFunctions = functions;
        }

        private int m_iLocomotiveAddress;
        private int m_iSpeed;
        private bool m_fForward;
        private string m_strState;
        private int m_iIndex;
        private bool []m_fFunctions;
    }

    public class RemoteLoconetService: RemoteFolder
    {                   
        public RemoteLoconetSlot[] Slots;

        public RemoteLoconetService(string name, string className, string path, ulong internalId, ulong parentInternalId, JsonValue objectDef):
            base(name, className, path, internalId, parentInternalId)
        {
            this.UpdateState(objectDef);
        }

        private bool[] ParseFunctions(JsonValue objectDef)
        {
            var functionsData = (int)objectDef["functions"];
            bool[] functions = new bool[RemoteLoconetSlot.MAX_FUNCTIONS];
            
            for (int k = 0; k < functions.Length; ++k)
            {
                functions[k] = (functionsData & 0x01) == 1;
                functionsData >>= 1;
            }

            return functions;
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
                
                slot.Functions = this.ParseFunctions(slotDef);

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

                    Slots[i] = new RemoteLoconetSlot(i, state, locomotiveAddress, speed, forward, this.ParseFunctions(slotDef));
                }
            }
        }
        
        public override Control CreateControl(IConsole console)
        {
            return new RemoteLoconetServiceUserControl(this, Slots);
        }
    }
}

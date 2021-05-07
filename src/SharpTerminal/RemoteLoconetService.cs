using System;
using System.Json;
using System.Windows.Forms;

namespace SharpTerminal
{   
    public class RemoteLoconetSlot
    {        
        public int LocomotiveAddress { get; set; }

        public int Speed { get; set; }

        public bool Forward;

        public String State;
        

        public RemoteLoconetSlot(string state, int locomotiveAddress, int speed, bool forward)
        {
            State = state;
            LocomotiveAddress = locomotiveAddress;
            Speed = speed;
            Forward = forward;
        }
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

            if (!objectDef.ContainsKey("slots"))
                return;

            var slotsData = objectDef["slots"];

            if(slotsData.Count == 0)
            {
                return;
            }

            Slots = new RemoteLoconetSlot[slotsData.Count];

            for(int i = 0;i < slotsData.Count; ++i)
            {
                var slotDef = slotsData[i];

                var state = slotDef["state"];
                var locomotiveAddress = (int)slotDef["locomotiveAddress"];
                var speed = (int)slotDef["speed"];
                var forward = (bool)slotDef["forward"];

                Slots[i] = new RemoteLoconetSlot(state, locomotiveAddress, speed, forward);
            }
        }
        
        public override Control CreateControl()
        {
            return new RemoteLoconetServiceUserControl(this, Slots);
        }
    }
}

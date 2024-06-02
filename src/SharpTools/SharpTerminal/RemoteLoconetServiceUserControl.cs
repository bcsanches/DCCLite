
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteLoconetServiceUserControl : UserControl
    {
        public RemoteLoconetServiceUserControl()
        {
            InitializeComponent();
        }

        public RemoteLoconetServiceUserControl(RemoteLoconetService loconet, RemoteLoconetSlot []slots) :
            this()
        {            
            if (slots == null)
                return;


            for (int i = 0; i < slots.Length; ++i)
            {
                m_bsDataSource.Add(slots[i]);
            }
                //m_bsDataSource = slots;

            /*
            m_gridMain.Rows.Add(slots.Length);

            for (int i = 0; i < slots.Length; ++i)
            {
                var row = m_gridMain.Rows[i];
                var slot = slots[i];

                row.Cells[0].Value = i;
                row.Cells[1].Value = slot.State;
                row.Cells[2].Value = slot.LocomotiveAddress;
                row.Cells[3].Value = slot.Forward ? "FWD" : "BWD";
                row.Cells[4].Value = slot.Speed.ToString();
            }
            */
        }
    }
}

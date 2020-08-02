using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteDeviceUserControl : UserControl
    {
        public RemoteDeviceUserControl()
        {
            InitializeComponent();
        }

        public RemoteDeviceUserControl(RemoteDevice remoteDevice, RemotePin[] pins) :
            this()
        {
            m_lbTitle.Text += " - " + remoteDevice.Name;

            if (pins == null)
                return;

            m_gridMain.Rows.Add(pins.Length);

            for(int i = 0; i < pins.Length; ++i)            
            {
                var row = m_gridMain.Rows[i];
                var pin = pins[i];                

                row.Cells[0].Value = i;
                row.Cells[1].Value = pin.SpecialName;
                row.Cells[2].Value = pin.Decoder;
                row.Cells[3].Value = pin.DecoderAddress;
                row.Cells[4].Value = pin.Usage;                             
            }            
        }
    }
}

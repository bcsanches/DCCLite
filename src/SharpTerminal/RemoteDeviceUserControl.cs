
using System;
using System.Drawing;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteDeviceUserControl : UserControl
    {
        private readonly RemoteDevice mRemoteDevice;

        public RemoteDeviceUserControl()
        {
            InitializeComponent();
        }

        public RemoteDeviceUserControl(RemoteDevice remoteDevice, RemotePin[] pins) :
            this()
        {            
            mRemoteDevice = remoteDevice ?? throw new ArgumentNullException(nameof(remoteDevice));

            this.UpdateLabel();

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
                
                if((pin.DecoderBroken != null) && ((bool)pin.DecoderBroken))
                {
                    row.DefaultCellStyle.BackColor = Color.Red;
                }
            }

            remoteDevice.PropertyChanged += RemoteDevice_PropertyChanged;
        }

        private void UpdateLabel()
        {
            m_lbTitle.Text = "RemoteDevice: " + mRemoteDevice.Name + " - mem: " + mRemoteDevice.FreeRam.ToString() + " bytes";
        }

        private void RemoteDevice_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            //We only expect free ram to change...
            UpdateLabel();
        }        
    }
}

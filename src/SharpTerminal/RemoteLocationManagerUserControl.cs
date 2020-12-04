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
    public partial class RemoteLocationManagerUserControl : UserControl
    {
        public RemoteLocationManagerUserControl()
        {
            InitializeComponent();
        }

        public RemoteLocationManagerUserControl(LocationMismatch[] mismatches) :
            this()
        {
            if (mismatches == null)
                return;

            m_gridMain.Rows.Add(mismatches.Length);

            for(int i = 0; i < mismatches.Length; ++i)            
            {
                var row = m_gridMain.Rows[i];
                var mismatch = mismatches[i];

                var decoder = mismatch.Decoder;

                row.Cells[0].Value = decoder.Address;
                row.Cells[1].Value = decoder.ClassName;
                row.Cells[2].Value = decoder.Name;
                row.Cells[3].Value = mismatch.Reason;
                row.Cells[4].Value = decoder.DeviceName;
                row.Cells[5].Value = decoder.LocationHint;

                if (mismatch.MappedLocation != null)
                    row.Cells[6].Value = mismatch.MappedLocation;

                if (decoder.Broken)
                    row.DefaultCellStyle.BackColor = Color.Red;
            }            
        }
    }
}

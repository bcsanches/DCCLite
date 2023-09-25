using System;
using System.Drawing;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class RemoteLocationUserControl : UserControl
    {
        public RemoteLocationUserControl()
        {
            InitializeComponent();
        }

        public RemoteLocationUserControl(String name, int beginAddress, int endAddress, RemoteDecoder[] decoders) :
            this()
        {
            int num = endAddress - beginAddress;
            if (num == 0)
                return;

            m_gridMain.Rows.Add(num);

            for(int i = beginAddress, pos = 0;i < endAddress; ++i, ++pos)
            {
                var row = m_gridMain.Rows[pos];

                row.Cells[0].Value = i;

                if (decoders[pos] == null)
                    continue;

                row.Cells[1].Value = decoders[pos].ClassName;
                row.Cells[2].Value = decoders[pos].Name;
                row.Cells[3].Value = decoders[pos].DeviceName;

                if (decoders[pos].Broken)
                    row.DefaultCellStyle.BackColor = Color.Red;
            }

            m_lbTitle.Text += " " + name;
        }
    }
}

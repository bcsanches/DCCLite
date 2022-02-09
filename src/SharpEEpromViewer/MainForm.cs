using System;
using System.Windows.Forms;

namespace SharpDude
{
    public partial class MainForm : Form
    {        
        public MainForm()
        {
            InitializeComponent();
        }        

        private void btnExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.Filter = "rom files (*.rom)|*.rom|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 2;

                if (openFileDialog.ShowDialog() != DialogResult.OK)
                    return;

                //Get the path of specified file
                var filePath = openFileDialog.FileName;

                //Read the contents of the file into a stream
                var fileStream = openFileDialog.OpenFile();

                //fileStream.ReadByte
            }
        }
    }
}

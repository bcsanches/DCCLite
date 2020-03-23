using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpDude
{
    public partial class MainForm : Form
    {
        IAvrDude mAvrDude;
        System.Diagnostics.Process mAvrDudeProcess;

        //https://saezndaree.wordpress.com/2009/03/29/how-to-redirect-the-consoles-output-to-a-textbox-in-c/
        public MainForm()
        {
            InitializeComponent();

            try
            {
                mAvrDude = AvrDudeService.Create();

                tbAvrDude.Text = mAvrDude.GetName();
            }
            catch(Exception ex)
            {
                MessageBox.Show("Avrdude detection failed: " + ex.Message, "Fatal error", MessageBoxButtons.OK, MessageBoxIcon.Error);

                tbAvrDude.Text = ex.Message;                
            }

            string[] ports = SerialPort.GetPortNames();
            cbComPorts.Items.AddRange(ports);

            cbComPorts.SelectedIndex = 0;

            var boards = ArduinoService.GetAvailableConfigs();
            
            if((boards == null) || (boards.Length == 0))
            {
                cbArduinoTypes.Items.Add("Error: No image files found.");                
            }
            else
            {
                cbArduinoTypes.DataSource = boards;
            }

            UpdateBurnButtonState();
        }

        private void btnExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void cbArduinoTypes_SelectedIndexChanged(object sender, EventArgs e)
        {
            var selectedItem = cbArduinoTypes.SelectedItem as IArduino;
            if(selectedItem == null)
            {
                btnBurn.Enabled = false;

                return;
            }            

            btnBurn.Enabled = true;
            tbImageName.Text = System.IO.Path.GetFileName(selectedItem.ImageName);
            tbVersion.Text = selectedItem.Version;

            this.UpdateBurnButtonState();
        }

        private void btnBurn_Click(object sender, EventArgs e)
        {
            btnBurn.Enabled = false;            

            var boardInfo = cbArduinoTypes.SelectedItem as IArduino;

            var port = cbComPorts.SelectedItem as string;
            if (port == null)
                port = cbComPorts.Text;

            if (string.IsNullOrEmpty(port))
                port = "COM1";

            // ./avrdude -v -p atmega2560 -C "..\etc\avrdude.conf" -c wiring -b 115200 -D -P "COM4" -U flash:w:"F:\develop\bcs\DCCLite\src\LiteDecoder\.pio\build\megaatmega2560\firmware.hex":i
            string command =
                "-v -p " + boardInfo.AvrDudeName +
                " -C \"" + mAvrDude.GetConfPath() + "\"" + 
                " -c " + boardInfo.Programmer + " -b " + boardInfo.BaudRate + " -D -P " + port +
                " -U flash:w:\"" + boardInfo.ImageName + "\":i"
            ;

            var process = new System.Diagnostics.Process();
            process.EnableRaisingEvents = true;
            process.Exited += new EventHandler(AvrDudeProcess_Exited);
            process.StartInfo.FileName = mAvrDude.GetName();
            process.StartInfo.WorkingDirectory = System.IO.Path.GetDirectoryName(mAvrDude.GetName());
            process.StartInfo.Arguments = command;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;

            mAvrDudeProcess = process;

            mAvrDudeProcess.OutputDataReceived += MAvrDudeProcess_OutputDataReceived;
            mAvrDudeProcess.ErrorDataReceived += MAvrDudeProcess_OutputDataReceived;

            tbOutput.Text = mAvrDude.GetName();
            tbOutput.AppendText("\r\n");
            tbOutput.AppendText(command);
            tbOutput.AppendText("\r\n\r\n");

            mAvrDudeProcess.Start();
            
            //mAvrDudeProcess.BeginOutputReadLine();
            mAvrDudeProcess.BeginErrorReadLine();
        }

        private void MAvrDudeProcess_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            if (e.Data == null)
                return;

            if(tbOutput.InvokeRequired)
            {
                tbOutput.Invoke(new Action(() => this.MAvrDudeProcess_OutputDataReceived(sender, e)));
            }
            else
            {
                tbOutput.AppendText(e.Data);
                tbOutput.AppendText("\r\n");

                tbOutput.SelectionStart = tbOutput.Text.Length;
                tbOutput.SelectionLength = 0;
            }
        }

        private void AvrDudeProcess_Exited(object sender, EventArgs e)
        {
            if (mAvrDudeProcess == null)
                return;            

            if(btnBurn.InvokeRequired)
            {
                btnBurn.Invoke(new Action(() => this.AvrDudeProcess_Exited(sender, e)));
            }
            else
            {                
                if(mAvrDudeProcess.ExitCode != 0)
                {
                    MessageBox.Show("avrdude failed with code: " + mAvrDudeProcess.ExitCode);
                }

                mAvrDudeProcess = null;

                UpdateBurnButtonState();
            }            
        }

        private void UpdateBurnButtonState()
        {            
            btnBurn.Enabled = (cbArduinoTypes.SelectedItem != null) && (mAvrDudeProcess == null);
        }
    }
}

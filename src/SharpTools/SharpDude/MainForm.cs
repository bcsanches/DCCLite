
using System.IO.Ports;
using System.Management;


namespace SharpDude
{
    public partial class MainForm : Form
    {
        IAvrDude ?mAvrDude;
        System.Diagnostics.Process ?mAvrDudeProcess;
        string[] mComPorts = [];

		private ManagementEventWatcher? mArrivalWatcher;
		private ManagementEventWatcher? mRemovalWatcher;

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

            this.LoadPorts();

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

            MonitorDeviceChanges();

            this.SetStatus(string.Empty);
		}

		public enum EventType
		{
			Insertion,
			Removal,
		}

        private void SetStatus(string text)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new Action(() => { this.SetStatus(text); }));

                return;
            }

            m_lblStatus.Text = text;
        }

		//Thanks: https://stackoverflow.com/a/4269883/440867
		private void MonitorDeviceChanges()
		{
			try
			{
				var deviceArrivalQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2");
				var deviceRemovalQuery = new WqlEventQuery("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 3");

				mArrivalWatcher = new ManagementEventWatcher(deviceArrivalQuery);
				mRemovalWatcher = new ManagementEventWatcher(deviceRemovalQuery);

				mArrivalWatcher.EventArrived += (o, args) => RaisePortsChangedIfNecessary(EventType.Insertion);
				mRemovalWatcher.EventArrived += (sender, eventArgs) => RaisePortsChangedIfNecessary(EventType.Removal);

				// Start listening for events
				mArrivalWatcher.Start();
				mRemovalWatcher.Start();
			}
			catch (ManagementException ex)
			{
                MessageBox.Show("Failed to install com port watcher: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void RaisePortsChangedIfNecessary(EventType eventType)
		{
			lock (mComPorts)
			{
				var availableSerialPorts = SerialPort.GetPortNames();
				if (!mComPorts.SequenceEqual(availableSerialPorts))
				{
                    this.SetStatus(eventType == EventType.Insertion ? "New com port detected" : "Com port removed");

					mComPorts = availableSerialPorts;
                    this.RefreshPorts();
				}
			}
		}

        private void RefreshPorts()
        {
            if(cbComPorts.InvokeRequired)
            {
                cbComPorts.Invoke(new MethodInvoker(() => { this.RefreshPorts(); }));

                return;
            }

			var previousPort = cbComPorts.SelectedItem as string;

			cbComPorts.SuspendLayout();

			cbComPorts.Items.Clear();
			cbComPorts.Items.AddRange(mComPorts);

			cbComPorts.SelectedIndex = 0;
			if (previousPort != null)
			{
				foreach (var port in mComPorts)
				{
					if (port == previousPort)
					{
						cbComPorts.SelectedItem = port;
						break;
					}
				}
			}

			cbComPorts.ResumeLayout();
		}

		private void LoadPorts()
        {            			
			mComPorts = SerialPort.GetPortNames();

            this.RefreshPorts();
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

        private static async Task TouchSerialPort(string portName)
        {
            try
            {
                using SerialPort port = new();
                
                port.PortName = portName;
                port.BaudRate = 1200;
                port.DtrEnable = false;

                port.Open();                
            }    
            catch(Exception )
            {
                //if the board is reset, the connection to port is lost and we cannot close / dispose it without errors
                //so just swallow the exception and let it go...
            }

            //Do not know why, but pio does, so we do...
            await Task.Delay(400);            
        }

        private async Task<string> WaitNewPort(string prevPort)
        {
            string ?newPort = null;

            System.Diagnostics.Debug.Assert(mComPorts != null);

            for(long timeout = 0; (timeout < 5000) && (newPort == null); timeout += 250)
            {            
                string[] currentPorts = SerialPort.GetPortNames();

                foreach(var p in currentPorts)
                {
                    if (mComPorts.Any(x => x == p))
                        continue;

                    newPort = p;
                    break;
                }

                if (newPort != null)
                    break;
                    
                await Task.Delay(250);
            }

            if(newPort == null)
            {
                newPort = prevPort;
            }

#if false
            //try it
            using (SerialPort port = new SerialPort())
            {
                port.PortName = newPort;
                port.BaudRate = 9600;
                port.DataBits = 8;
                port.Parity = Parity.None;
                port.StopBits = StopBits.One;
                port.RtsEnable = false;
                port.DtrEnable = false;

                port.Open();
                port.Close();
            }          
#endif

            return newPort;
        }

        private async void btnBurn_Click(object sender, EventArgs e)
        {
            btnBurn.Enabled = false;

			System.Diagnostics.Debug.Assert(cbArduinoTypes.SelectedItem != null);
			var boardInfo = (IArduino)cbArduinoTypes.SelectedItem;

			System.Diagnostics.Debug.Assert(cbComPorts.SelectedItem != null);

			//Allow user to type a port not detected...
			var port = (string)cbComPorts.SelectedItem;

            tbOutput.Text = string.Empty;

            System.Diagnostics.Debug.Assert(mAvrDude != null);			

			try
            {
                if (boardInfo.RequiresReset)
                {
                    this.AppendMessage("Touching com port " + port);
                    await TouchSerialPort(port);

                    this.AppendMessage("Waiting for new COM port");
                    port = await this.WaitNewPort(port);

                    this.AppendMessage("Using new port: " + port);
                }

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

                this.AppendMessage(mAvrDude.GetName());
                this.AppendMessage(command);
                
                mAvrDudeProcess.Start();

                //mAvrDudeProcess.BeginOutputReadLine();
                mAvrDudeProcess.BeginErrorReadLine();
            }
            catch(Exception ex)
            {
                MessageBox.Show("Error: " + ex.Message, "Burn failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
                mAvrDudeProcess = null;
                btnBurn.Enabled = true;
            }
        }

        private void MAvrDudeProcess_OutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            if (e.Data == null)
                return;

            this.AppendMessage(e.Data);            
        }

        private void AppendMessage(string message)
        {
            if (tbOutput.InvokeRequired)
            {
                tbOutput.Invoke(new Action(() => this.AppendMessage(message)));
            }
            else
            {
                tbOutput.AppendText(message);
                tbOutput.AppendText("\r\n");

                tbOutput.SelectionStart = tbOutput.Text.Length;
                tbOutput.SelectionLength = 0;
            }
        }

        private void AvrDudeProcess_Exited(object ?sender, EventArgs e)
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
            btnBurn.Enabled = (cbArduinoTypes.SelectedItem != null) && (cbComPorts.SelectedItem != null) && (mAvrDudeProcess == null);
        }
    }
}

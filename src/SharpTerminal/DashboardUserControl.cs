using System;
using System.Data;
using System.Drawing;
using System.Net.Http.Headers;
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class DashboardUserControl : UserControl
    {
        public DashboardUserControl()
        {
            InitializeComponent();
        }

        public DashboardUserControl(IConsole console, RemoteRoot root) :
            this()
        {
            mLoadTimer_Tick(null, null);

            mConsole = console ?? throw new ArgumentNullException(nameof(console));
            mRoot = root ?? throw new ArgumentNullException(nameof(root));
        }

        protected async override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            
            mLoadTimer.Start();

            //Visual Studio Designer?
            if (mConsole == null)
                return;

            var result = await mConsole.RequestAsync("Get-RNames");
            mLoadTimer.Stop();

            if (result["classname"] != "RNames")
            {
                m_lbTitle.Text = "Error loading data";

                MessageBox.Show("Error: classname is " + result["classname"]);
                
            }
            else
            {
                m_lbTitle.Text = "Broker data";

                var namesData = result["rnames"];

                //
                //Use a DataTable because it is much, much faster than adding directly to the DataGridView
                // https://stackoverflow.com/a/36770807/440867
                var dataTable = new DataTable();                

                dataTable.Columns.Add("Index");
                dataTable.Columns.Add("Pos");
                dataTable.Columns.Add("Cluster");
                dataTable.Columns.Add("Length");
                dataTable.Columns.Add("Name");                

                for (int i = 0;i < namesData.Count; ++i)
                {
                    var nameData = namesData[i];

                    var row = dataTable.NewRow();
                    row["Pos"] = (int)nameData["position"];
                    row["Index"] = (int)nameData["index"];
                    row["Cluster"] = (int)nameData["cluster"];                    

                    var rname = nameData["name"].ToString();

                    row["Length"] = rname.Length;
                    row["Name"] = rname;

                    dataTable.Rows.Add(row);
                }

                m_gridMain.SuspendLayout();
                m_gridMain.Columns.Clear();
                m_gridMain.DataSource = dataTable;

                for(int i = 0;i < 4;++i)                 
                    m_gridMain.Columns[i].AutoSizeMode = DataGridViewAutoSizeColumnMode.AllCells;

                m_gridMain.Columns[4].AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill;

                m_gridMain.ResumeLayout();
            }            
        }

        private IConsole    mConsole;
        private RemoteRoot  mRoot;

        private int         mCurrentChar = 0;

        private static char[] gChars = {'|', '/', '-', '\\'};

        private void mLoadTimer_Tick(object sender, EventArgs e)
        {
            ++mCurrentChar;

            if(mCurrentChar == gChars.Length)
                mCurrentChar = 0;

            m_lbTitle.Text = "Loading data " + gChars[mCurrentChar];
        }
    }
}

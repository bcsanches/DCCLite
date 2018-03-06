
using System.Windows.Forms;

namespace SharpTerminal
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

            TerminalClient client = new TerminalClient("localhost", 4190);

            m_tbConsole.Text += "Connected\n";
        }
    }
}

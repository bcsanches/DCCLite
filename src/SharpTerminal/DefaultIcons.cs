using Properties;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SharpTerminal
{
    static class DefaultIcons
    {
        public static string DISCONNECTED_DRIVE_ICON = "disconnected_drive";
        public static string CONNECTED_DRIVE_ICON = "connected_drive";

        public static void LoadIcons(ImageList imageList)
        {
            imageList.Images.Add(CONNECTED_DRIVE_ICON, Resources.connected_drive);
            imageList.Images.Add(DISCONNECTED_DRIVE_ICON, Resources.disconnected_drive);
        }
    }
}

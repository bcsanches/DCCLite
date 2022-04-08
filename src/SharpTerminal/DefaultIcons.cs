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
        public static string CONNECTED_DRIVE_ICON = "connected_drive";
        public static string CONNECTING_DRIVE_ICON = "connecting_drive";
        public static string DISCONNECTED_DRIVE_ICON = "disconnected_drive";
        public static string FILE_GEAR_ICON = "file_gear";
        public static string FOLDER_ICON = "folder";
        public static string LAMP_OFF_ICON = "lamp_off";
        public static string LAMP_ON_ICON = "lamp_on";
        public static string SIGNAL_ICON = "signal";
        public static string SENSOR_OFF_ICON = "sensor_off";
        public static string SENSOR_ON_ICON = "sensor_on";
        public static string TURNOUT_OFF_ICON = "TURNOUT_off";
        public static string TURNOUT_ON_ICON = "TURNOUT_on";


        public static void LoadIcons(ImageList imageList)
        {
            imageList.Images.Add(FOLDER_ICON, Resources.folder);
            imageList.Images.Add(CONNECTED_DRIVE_ICON, Resources.connected_drive);
            imageList.Images.Add(CONNECTING_DRIVE_ICON, Resources.connecting_drive);
            imageList.Images.Add(DISCONNECTED_DRIVE_ICON, Resources.disconnected_drive);
            imageList.Images.Add(FILE_GEAR_ICON, Resources.file_gear);            
            imageList.Images.Add(LAMP_OFF_ICON, Resources.lamp_off);
            imageList.Images.Add(LAMP_ON_ICON, Resources.lamp_on);            
            imageList.Images.Add(SENSOR_OFF_ICON, Resources.sensor_off);
            imageList.Images.Add(SENSOR_ON_ICON, Resources.sensor_on);
            imageList.Images.Add(SIGNAL_ICON, Resources.signal);
            imageList.Images.Add(TURNOUT_OFF_ICON, Resources.turnout_off);
            imageList.Images.Add(TURNOUT_ON_ICON, Resources.turnout_on);
        }
    }
}

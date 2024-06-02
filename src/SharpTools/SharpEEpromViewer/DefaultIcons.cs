
using System.Runtime.Versioning;
using System.Windows.Forms;

namespace SharpEEPromViewer
{
	[SupportedOSPlatform("windows")]
	static class DefaultIcons
    {        
        public static string FILE_GEAR_ICON = "file_gear";
        public static string FOLDER_ICON = "folder";        

        public static void LoadIcons(ImageList imageList)
        {
            imageList.Images.Add(FOLDER_ICON, Properties.Resources.folder);            
            imageList.Images.Add(FILE_GEAR_ICON, Properties.Resources.file_gear);                        
        }
    }
}


project(SharpTerminal VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

set(ICON_RESOURCE_CONNECTED_DRIVE "Resources/connected_drive.png")
set(ICON_RESOURCE_DISCONNECTED_DRIVE "Resources/disconnected_drive.png")
set(ICON_RESOURCE_FOLDER "Resources/folder.png")
set(ICON_RESOURCE_FILE_GEAR "Resources/file_gear.png")

add_executable(SharpTerminal 
  App.config
  ConsoleUserControl.cs
  ConsoleUserControl.Designer.cs
  ConsoleUserControl.resx
  DefaultIcons.cs
  Form1.cs
  Form1.Designer.cs
  Form1.resx
  IConsole.cs
  ITerminalClientListener.cs
  ObjectsTreeViewUserControl.cs
  ObjectsTreeViewUserControl.Designer.cs
  ObjectsTreeViewUserControl.resx  
  Program.cs
  RequestManager.cs
  RemoteDevice.cs
  RemoteLocationManagerService.cs
  RemoteObject.cs
  TerminalClient.cs
  
  Properties/AssemblyInfo.cs
  Properties/Resources.Designer.cs
  Properties/Resources.resx
  Properties/Settings.Designer.cs
  Properties/Settings.settings

  ${ICON_RESOURCE_CONNECTED_DRIVE}
  ${ICON_RESOURCE_DISCONNECTED_DRIVE}
  ${ICON_RESOURCE_FOLDER}
  ${ICON_RESOURCE_FILE_GEAR}
)

set_property(TARGET SharpTerminal PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")
set_property(TARGET SharpTerminal PROPERTY WIN32_EXECUTABLE TRUE)

set(CMAKE_CSharp_FLAGS "/langversion:latest")

csharp_set_windows_forms_properties(
    ConsoleUserControl.cs
    ConsoleUserControl.Designer.cs
    ConsoleUserControl.resx
    
    Form1.cs
    Form1.Designer.cs
    Form1.resx
    
    Program.cs
    
    Properties/AssemblyInfo.cs
    Properties/Resources.Designer.cs
    Properties/Resources.resx
    Properties/Settings.Designer.cs
    Properties/Settings.settings
    
    ObjectsTreeViewUserControl.cs
    ObjectsTreeViewUserControl.Designer.cs
    ObjectsTreeViewUserControl.resx    
)

set_property(TARGET SharpTerminal PROPERTY VS_DOTNET_REFERENCES
    "Microsoft.CSharp"    
    "System"
    "System.Core"
    "System.Data"
    "System.Data.DataSetExtensions"
    "System.Deployment"
    "System.Drawing"
    "System.Json"
    "System.Net.Http"
    "System.Windows.Forms"    
    "System.Xml"
    "System.Xml.Linq"
)

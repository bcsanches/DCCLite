
project(SharpTerminal VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

set(ICON_RESOURCE_CONNECTED_DRIVE "Resources/connected_drive.png")
set(ICON_RESOURCE_CONNECTING_DRIVE "Resources/connecting_drive.png")
set(ICON_RESOURCE_DISCONNECTED_DRIVE "Resources/disconnected_drive.png")
set(ICON_RESOURCE_FOLDER "Resources/folder.png")
set(ICON_RESOURCE_FILE_GEAR "Resources/file_gear.png")
set(ICON_RESOURCE_LAMP_OFF "Resources/lamp_off.png")
set(ICON_RESOURCE_LAMP_ON "Resources/lamp_on.png")
set(ICON_RESOURCE_SENSOR_OFF "Resources/sensor_off.png")
set(ICON_RESOURCE_SENSOR_ON "Resources/sensor_on.png")
set(ICON_RESOURCE_SIGNAL "Resources/signal.png")
set(ICON_RESOURCE_TURNOUT_OFF "Resources/turnout_off.png")
set(ICON_RESOURCE_TURNOUT_ON "Resources/turnout_on.png")

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
  NotifyPropertyBase.cs
  ObjectsTreeViewUserControl.cs
  ObjectsTreeViewUserControl.Designer.cs
  ObjectsTreeViewUserControl.resx  
  Program.cs
  RequestManager.cs
  RemoteDecoder.cs
  RemoteObjectUserControl.cs
  RemoteObjectUserControl.Designer.cs
  RemoteObjectUserControl.resx
  RemoteDevice.cs  
  RemoteDeviceUserControl.cs
  RemoteDeviceUserControl.Designer.cs
  RemoteDeviceUserControl.resx
  RemoteLocationManager.cs
  RemoteLocationManagerUserControl.cs
  RemoteLocationManagerUserControl.Designer.cs
  RemoteLocationManagerUserControl.resx
  RemoteLocationUserControl.cs
  RemoteLocationUserControl.Designer.cs
  RemoteLocationUserControl.resx
  RemoteLoconetService.cs
  RemoteLoconetServiceUserControl.cs
  RemoteLoconetServiceUserControl.Designer.cs
  RemoteLoconetServiceUserControl.resx
  RemoteObject.cs
  RemoteShortcutUserControl.cs
  RemoteShortcutUserControl.Designer.cs
  RemoteShortcutUserControl.resx
  
  ServoTurnoutProgrammer.cs
  
  TerminalClient.cs
  
  Properties/AssemblyInfo.cs
  Properties/Resources.Designer.cs
  Properties/Resources.resx
  Properties/Settings.Designer.cs
  Properties/Settings.settings
  
  ServerSelectionForm.cs
  ServerSelectionForm.Designer.cs
  ServerSelectionForm.resx

  ${ICON_RESOURCE_CONNECTED_DRIVE}
  ${ICON_RESOURCE_CONNECTING_DRIVE}
  ${ICON_RESOURCE_DISCONNECTED_DRIVE}
  ${ICON_RESOURCE_FOLDER}
  ${ICON_RESOURCE_FILE_GEAR}
  ${ICON_RESOURCE_LAMP_OFF}
  ${ICON_RESOURCE_LAMP_ON}
  ${ICON_RESOURCE_SENSOR_OFF}
  ${ICON_RESOURCE_SENSOR_ON}
  ${ICON_RESOURCE_SIGNAL}
  ${ICON_RESOURCE_TURNOUT_OFF}
  ${ICON_RESOURCE_TURNOUT_ON}
  
  SharpTerminal.RemoteLoconetSlot.datasource
)

set_property(TARGET SharpTerminal PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
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
    
    RemoteObjectUserControl.cs
    RemoteObjectUserControl.Designer.cs
    RemoteObjectUserControl.resx

    RemoteDeviceUserControl.cs
    RemoteDeviceUserControl.Designer.cs
    RemoteDeviceUserControl.resx

    RemoteLocationManagerUserControl.cs
    RemoteLocationManagerUserControl.Designer.cs
    RemoteLocationManagerUserControl.resx

    RemoteLocationUserControl.cs
    RemoteLocationUserControl.Designer.cs
    RemoteLocationUserControl.resx
    
    RemoteLoconetServiceUserControl.cs
    RemoteLoconetServiceUserControl.Designer.cs
    RemoteLoconetServiceUserControl.resx
    
    RemoteShortcutUserControl.cs
    RemoteShortcutUserControl.Designer.cs
    RemoteShortcutUserControl.resx
    
    ServerSelectionForm.cs
    ServerSelectionForm.Designer.cs
    ServerSelectionForm.resx        
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

target_link_libraries(SharpTerminal SharpCommon)

set_property(TARGET SharpTerminal PROPERTY VS_PACKAGE_REFERENCES "System.Json")

install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/System.Json.dll DESTINATION bin)
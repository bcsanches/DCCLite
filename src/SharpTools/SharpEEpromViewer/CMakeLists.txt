
project(SharpEEpromViewer VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

set(ICON_RESOURCE_FOLDER "Resources/folder.png")
set(ICON_RESOURCE_FILE_GEAR "Resources/file_gear.png")

add_executable(SharpEEpromViewer 
  App.config    
  DefaultIcons.cs
  MainForm.cs
  MainForm.Designer.cs
  MainForm.resx
  ObjectInspectorUserControl.cs
  ObjectInspectorUserControl.Designer.cs
  ObjectInspectorUserControl.resx    
  Program.cs  
  Storage.cs
  
  Properties/AssemblyInfo.cs
  Properties/Resources.Designer.cs
  Properties/Resources.resx
  Properties/Settings.Designer.cs
  Properties/Settings.settings
  
  ${ICON_RESOURCE_FOLDER}
  ${ICON_RESOURCE_FILE_GEAR}
)

set_property(TARGET SharpEEpromViewer PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
set_property(TARGET SharpEEpromViewer PROPERTY WIN32_EXECUTABLE TRUE)

set(CMAKE_CSharp_FLAGS "/langversion:latest")

csharp_set_windows_forms_properties(      
    MainForm.cs
    MainForm.Designer.cs
    MainForm.resx
    
    ObjectInspectorUserControl.cs
    ObjectInspectorUserControl.Designer.cs
    ObjectInspectorUserControl.resx
    
    Program.cs
    
    Properties/AssemblyInfo.cs
    Properties/Resources.Designer.cs
    Properties/Resources.resx
    Properties/Settings.Designer.cs
    Properties/Settings.settings                   
)

set_property(TARGET SharpEEpromViewer PROPERTY VS_DOTNET_REFERENCES
    "Microsoft.CSharp"    
    "System"
    "System.Core"
    "System.Data"
    "System.Data.DataSetExtensions"
    "System.Deployment"
    "System.Design"
    "System.Drawing"    
    "System.Net.Http"
    "System.Windows.Forms"    
    "System.Xml"
    "System.Xml.Linq"
)

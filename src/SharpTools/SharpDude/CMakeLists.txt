
project(SharpDude VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

add_executable(SharpDude 
  App.config  
  AvrDudeService.cs
  ArduinoService.cs
  MainForm.cs
  MainForm.Designer.cs
  MainForm.resx    
  Program.cs  
  
  Properties/AssemblyInfo.cs
  Properties/Resources.Designer.cs
  Properties/Resources.resx
  Properties/Settings.Designer.cs
  Properties/Settings.settings
)

set_property(TARGET SharpDude PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
set_property(TARGET SharpDude PROPERTY WIN32_EXECUTABLE TRUE)

set(CMAKE_CSharp_FLAGS "/langversion:latest")

csharp_set_windows_forms_properties(      
    MainForm.cs
    MainForm.Designer.cs
    MainForm.resx
    
    Program.cs
    
    Properties/AssemblyInfo.cs
    Properties/Resources.Designer.cs
    Properties/Resources.resx
    Properties/Settings.Designer.cs
    Properties/Settings.settings            
)

set_property(TARGET SharpDude PROPERTY VS_DOTNET_REFERENCES
    "Microsoft.CSharp"    
    "System"
    "System.Core"
    "System.Data"
    "System.Data.DataSetExtensions"
    "System.Deployment"
    "System.Drawing"    
    "System.Net.Http"
    "System.Windows.Forms"    
    "System.Xml"
    "System.Xml.Linq"
)

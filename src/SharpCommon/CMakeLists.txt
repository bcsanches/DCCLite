
project(SharpCommon VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

add_library(SharpCommon    
    IServoProgrammer.cs
    IServoTurnout.cs

    ServoTurnoutProgramerForm.cs
    ServoTurnoutProgramerForm.Designer.cs
    ServoTurnoutProgramerForm.resx  
  
    Properties/AssemblyInfo.cs  
)

set_property(TARGET SharpCommon PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")

set(CMAKE_CSharp_FLAGS "/langversion:latest")

csharp_set_windows_forms_properties(            
    Properties/AssemblyInfo.cs       

    ServoTurnoutProgramerForm.cs
    ServoTurnoutProgramerForm.Designer.cs
    ServoTurnoutProgramerForm.resx
)

set_property(TARGET SharpCommon PROPERTY VS_DOTNET_REFERENCES
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
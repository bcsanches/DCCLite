SET QUERALT_DIR=c:\temp\queralt\

mkdir %QUERALT_DIR%

copy ..\src\Embedded\Queralt\src\main.* %QUERALT_DIR%
copy ..\src\Embedded\lib\DecodersCommon\*.cpp %QUERALT_DIR%
copy ..\src\Embedded\lib\DecodersCommon\*.h %QUERALT_DIR%
copy ..\src\Embedded\lib\Shared\*.cpp %QUERALT_DIR%
copy ..\src\Embedded\lib\Shared\*.h %QUERALT_DIR% 


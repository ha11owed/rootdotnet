@ECHO OFF
REM Build the root wrappers
REM argument 1: where ROOTSYS should point
REM argument 2: The output directory name
REM argument 3: Where to copy the results. If blank, no copy is done.

REM Setup the environment
set "ROOTSYS=%1"
set "PATH=%1\bin;%PATH%"

REM Build everything so we can actually run programs! :-)
devenv "Wrapper Generators.sln" /build "Release|Win32"

REM Find all bad headers in this distro of ROOT.
release\FindBadRootHeaders

REM Copy down the templates
copy release\cpp_template_project.vcproj .
copy release\solution_template.sln .

REM Next, build the wrapers!
"release\ROOT.NET Library Converter" -d ..\Wrappers\%2

REM Copy over the property sheet...
copy "ROOT Directories.vsprops" ..\Wrappers\%2\

REM And build the thing!
devenv "..\Wrappers\%2\ROOT.NET Lib.sln" /build "Release"

REM Now, see if we are going to copy the thing to a nice resting place.
:justcopy
if "%3"=="" goto :EOF
if not exist "%3" mkdir "%3"
copy ..\Wrappers\%2\Release\* "%3"

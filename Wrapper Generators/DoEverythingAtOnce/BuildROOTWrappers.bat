@ECHO OFF
REM Build the root wrappers
REM argument 1: where ROOTSYS should point
REM argument 2: The output directory name
REM argument 3: Where to copy the results. If blank, no copy is done.
REM
REM Expect to be run from the Wrapper Generators directory
REM

REM Setup the environment
set "ROOTSYS=%1"
set "PATH=%1\bin;%PATH%"

REM Build everything so we can actually run programs! :-)
echo Building code to do the translation...
devenv /nologo "Wrapper Generators.sln" /project "FindBadRootHeaders" /build "Release|Win32"
devenv /nologo "Wrapper Generators.sln" /project "ROOT.NET Library Converter" /build "Release|Win32"

REM Find all bad headers in this distro of ROOT.

if not exist FindBadRootHeadersStatus.txt (
  echo Looking for bad ROOT headers - this will take a few minuts...
  release\FindBadRootHeaders > FindBadRootheaders.log
  echo Done with finding headers > FindBadRootHeadersStatus.txt
) else (
  echo Using header scan results from last run. Remove FindBadRootHeadersStatus.txt to force re-run"
  )

REM Copy down the templates
copy release\cpp_template_project.vcxproj .
copy release\solution_template.sln .

REM Next, build the wrapers!
echo "Building the wrappers"
"release\ROOT.NET Library Converter" -d ..\Wrappers\%2

REM Copy over the property sheet...
copy "ROOT Directories.props" ..\Wrappers\%2\

REM And build the thing!
devenv "..\Wrappers\%2\ROOT.NET Lib.sln" /build "Release"

REM Now, see if we are going to copy the thing to a nice resting place.
:justcopy
if "%3"=="" goto :EOF
if not exist "%3" mkdir "%3"
if not exist "%3\lib" mkdir "%3\lib"
if not exist "%3\include" mkdir "%3\include"
copy ..\Wrappers\%2\Release\* "%3\lib"
copy ..\Wrappers\WrapperPlumbingLibrary\NetArrayTranslator.hpp "%3\include"
copy ..\Wrappers\%2\converted_items.txt "%3"

#<#
#.Synopsis
#	Create a new version of the ROOT wrappers
#.Description
#   Invoking this command will build a new set of ROOT .NET Wrappers. This command can take hours to run!
#.Parameter $WrapperBuildDirectory
#    Where the wrappers should be built. The directory does not have to be there already.
#.Example
#	new-wrapper D:\rootresults
##>
param (
	[Parameter(Mandatory=$True)][string] $WrapperBuildDirectory
)

# NOTE:
# It would be nice to pass in the ROOT package name that we are going to build this against,
# however, the only way to make that work is if we can invoke VS to run the nuget add/remove, as
# it must alter the solution and project files. This is not possible from the raw nuget command line.
#
# See: https://nuget.codeplex.com/discussions/562283 for where i attempt to find help.
#
# As a result we can only use what is currently checked in - so the config must have been done
# previously.
#

# Look for the output directory
if (-not $(test-path $WrapperBuildDirectory)) {
	$null = new-item $WrapperBuildDirectory -type directory
}

# Look for this script location. It must be run relative to its current location.
$wrapper_generators_path = $(split-path -parent $(Split-Path -parent $MyInvocation.MyCommand.Definition))

# If we don't have nuget downloaded and ready to go, get it.
$nuget_path = "$WrapperBuildDirectory\nuget.exe"
set-alias nuget $nuget_path
if (-not $(test-path $nuget_path)) {
	copy-item "$wrapper_generators_path\DoEverythingAtOnce\nuget.exe" $nuget_path
	# Update it to the latest
	nuget update -self
}

# Great. Next make sure that everything to build the wrappers is properly built.

$dotNetFrameworkVersion = "v4.0.30319"
$dotNetFrameworkPath = "${env:SystemRoot}\Microsoft.NET\Framework\$dotNetFrameworkVersion"
if (-not $(test-path $dotNetFrameworkPath)) {
  Write-Host "Can't find .NET framework version $dotNetFrameworkVersion."
  return
}
set-alias msbuild "$dotNetFrameworkPath\msbuild.exe"

$wrapperGenerateSolutionPath = "$WrapperBuildDirectory\Wrapper Generators.sln"
msbuild $wrapperGenerateSolutionPath /project "FindBadRootHeaders" /t:build /p:Configuration=Release
#devenv /nologo $wrapperGenerateSolutionPath /project "FindBadRootHeaders" /build "Release|Win32"
#devenv /nologo $wrapperGenerateSolutionPath /project "ROOT.NET Library Converter" /build "Release|Win32"
#devenv /nologo $wrapperGenerateSolutionPath /project "ROOT.NET Addon Library Converter" /build "Release|Win32"
#devenv /nologo $wrapperGenerateSolutionPath /project "DumpConfigInfo" /build "Release|Win32"



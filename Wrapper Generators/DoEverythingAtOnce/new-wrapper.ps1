#<#
#.Synopsis
#	Create a new version of the ROOT wrappers
#.Description
#   Invoking this command will build a new set of ROOT .NET Wrappers. This command can take hours to run!
#.Parameter $ROOTNugetPackageName
#    The nuget name of the version of ROOT to build against. For example, ROOT_5_34_19. This
#    most recent version is used.
#.Parameter $WrapperBuildDirectory
#    Where the wrappers should be built. The directory does not have to be there already.
#.Example
#	new-wrapper ROOT_5_34_19 D:\rootresults
##>
param (
	[Parameter(Mandatory=$True)][string] $ROOTNugetPackageName,
	[Parameter(Mandatory=$True)][string] $WrapperBuildDirectory
)

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

# Great. Next, copy 
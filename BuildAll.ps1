#
# Build instructions for all known versions of ROOT
#

#$VersionROOTdotNET = "2.2"
#$ChangeSetROOTdotNET = 75433
$VersionROOTdotNET = "HEAD"
$ChangeSetROOTdotNET = "HEAD"

$buildLocation = "c:\users\gwatts\Documents\rootbuilds"

#######
# Load external functions
#

$scriptLocation = Split-Path -parent $MyInvocation.MyCommand.Definition

& "$scriptLocation\ReleaseBuild.ps1"
& "$scriptLocation\GenerateReport.ps1"
& "$scriptLocation\BuildNuGetPacakges.ps1"

######
# Helper functions
#

#
# Build a given root package
function BuildROOTWrapper($version, $changeSet, $rootURL)
{
    $baseDirName = ($rootURL -split "/")[-1]
    $baseDir = "$buildLocation\$baseDirName"
    
    ReleaseBuild $baseDir $changeSet $rootURL
    Write-Host "Generating build report..."
    GenerateReport "$baseDir\logs"
    Write-Host "Generating nuget packages..."
    BuildNuGetPacakges $baseDir
}

#
# Do the build for a change set and a version
#
function BuildAllKnownROOTWrappers($version, $changeSet)
{
    BuildROOTWrapper $version $changeSet "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz"
#$rootDirectory = "c:\users\gwatts\Documents\rootrelease"
#$ROOTdotNETSource = "HEAD"
#rootRelease = "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz"
#ReleaseBuild $rootDirectory $ROOTdotNETSource rootRelease
}

#
# Start it off! :-)
#

BuildAllKnownROOTWrappers $VersionROOTdotNET $ChangeSetROOTdotNET


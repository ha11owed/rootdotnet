#
# Build instructions for all known versions of ROOT
#

#
# Check that we were given at least one argument that tells us
# where the release file is!
#

if (($Args.length -eq 0) -or -not (test-path $Args[0]))
{
    $f = ""
    if ($Args.length -ne 0)
    {
        $f = $Args[0]
    }
    throw "Release file can't be found or a filename argument wasn't given as argument to this script ('$f')"
}
$releaseFile = $Args[0]

#######
# Load external functions and configuration for this machine.
#

$scriptLocation = Split-Path -parent $MyInvocation.MyCommand.Definition
& "$scriptLocation\ReleaseBuild.ps1"
& "$scriptLocation\GenerateReport.ps1"
& "$scriptLocation\BuildNuGetPacakges.ps1"

#
# Build a given root package
#
function BuildROOTWrapper($version, $changeSet, $rootURL)
{
    $baseDirName = ($rootURL -split {$_ -eq "/" -or $_ -eq "\"})[-1]
    $baseDir = "$buildLocation\RDN-$version-$baseDirName"
    
    $lastRevBuilt = ReleaseBuild $baseDir $changeSet $version $rootURL
    Write-Host "Generating build report..."
    GenerateReport "$baseDir\logs"
    Write-Host "Generating nuget packages..."
    BuildNuGetPacakges $baseDir $version -currentBuildRevision $lastRevBuilt
}

#
# This function (part of a pipe) does the actual work of processing
# a build request. Each line starts with the version number for the ROOT.NET
# software, and the second item is the location where we can copy the
# root tar-ball from.
#

function ProcessBuildRequest
{
    process
    {
        $info = $_ -split " "
        BuildROOTWrapper $info[0] $info[1] $info[2]        
    }
}

#
# Read in the release. We will be looping over them and extracting the proper
# versions of the build scripts, and then invoking them to get the actual
# builds done. We will treat the HEAD version specially. Other versions once
# tagged we don't expect them to ever be updated.
#

get-content -Path $releaseFile | ProcessBuildRequest

#
# Now we do the main build. The build is run by reading the release file
# - which contains version numbers and net locations of the root tar-balls
#   (usually from the ROOT web site).
#

#if (-not $buildall)
#{
#    $rootVersions = @("ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz")
#    BuildAllKnownROOTWrappers $VersionROOTdotNET $ChangeSetROOTdotNET
#} else
#{
#    $rootVersions = @(
#        "ftp://root.cern.ch/root/root_v5.28.00.win32.vc10.tar.gz"
#        "ftp://root.cern.ch/root/root_v5.28.00.win32.vc10.debug.tar.gz"
#        "ftp://root.cern.ch/root/root_v5.28.00d.win32.vc10.tar.gz"
#        "ftp://root.cern.ch/root/root_v5.28.00d.win32.vc10.debug.tar.gz"
#    )
#
#    #
#    # Run the master build now
#    #
        
#    BuildAllKnownROOTWrappers "HEAD" "HEAD"
#}

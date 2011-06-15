#
# Build instructions for all known versions of ROOT
#

#######
# Load external functions and configuration for this machine.
#

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
    foreach($v in $rootVersions)
    {
        BuildROOTWrapper $version $changeSet $v
    }
}

#
# Now we do the main build
#

if (-not $buildall)
{
    $rootVersions = @("ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz")
    BuildAllKnownROOTWrappers $VersionROOTdotNET $ChangeSetROOTdotNET
} else
{
    $rootVersions = @(
        "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz",
        "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.debug.tar.gz"
        "ftp://root.cern.ch/root/root_v5.28.00.win32.vc10.tar.gz"
        "ftp://root.cern.ch/root/root_v5.28.00.win32.vc10.debug.tar.gz"
        "ftp://root.cern.ch/root/root_v5.28.00d.win32.vc10.tar.gz"
        "ftp://root.cern.ch/root/root_v5.28.00d.win32.vc10.debug.tar.gz"
    )

    #
    # Run the master build now
    #
        
    BuildAllKnownROOTWrappers "HEAD" "HEAD"
}

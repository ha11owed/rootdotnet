#
# Build all requested releases. This will scan the 
# packages availible and issue a build for each new one.
#
#
#
[CmdletBinding()]
param (
  [Parameter(mandatory=$true)]
  [string] $minROOTVersion,
  [Parameter(mandatory=$true)]
  [string] $wrapperVersion,
  [Parameter(mandatory=$true)]
  [string] $nugetDir
  )

#
# Capture a few things about our environment before anything happens
#

$baseDir = $PWD.Path

#
# Some quick x-checsk and initalization
#

#
# We depend on a number of other modules up and running.
#

Import-Module -Force -DisableNameChecking ./ROOTRepositoryAccess
Import-Module -DisableNameChecking -Force ./BuildControl

#
# Helper function - check to see if a given release has already been
# built
#
function Check-If-Built ($version, $nugetDir, $wrapperVersion)
{
    $corePkg = "ROOTNET-Core-v$($version.VersionMajor).$($version.VersionMinor).$($version.VersionSubMinor).win32.vc10.$($wrapperVersion).nupkg"
    return Test-Path -Path "$nugetDir/$corePkg"
}

#
# Get a list of all packages that are up on ROOT servers. It is very
# likely that we will have build some of them already, so we should
# x-check that to see if we can find them.
#

$goodReleases = Get-Subsequent-Releases $minROOTVersion | ? {$_.DownloadType -eq "win32.vc10"} | ? {-not $(Check-If-Built $_ $nugetDir $wrapperVersion)}

#
# Build each release that we've not built before!
#

$libDir="$baseDir\Wrappers\AutoBuild"
foreach ($r in $goodReleases)
{
    Set-Location $baseDir

    #
    # Clean out anything that has happened already
    #

    if (Test-Path $libDir)
    {
		Write-Host "Removing results of previous build in $libDir"
        Remove-Item -Recurse $libDir
    } else
	{
		Write-Host "No previous results found in $libDir"
	}

    #
    # Next, do the build.
    #

    Build-ROOT-Dot-Net -Version $wrapperVersion -BuildLocation $baseDir -ROOTURL $r.URL -showLog $false -nugetDir $nugetDir
}

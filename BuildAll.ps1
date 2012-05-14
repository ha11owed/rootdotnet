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

Import-Module -Force -DisableNameChecking $baseDir/ROOTRepositoryAccess
Import-Module -DisableNameChecking -Force $baseDir/BuildControl
Import-Module -DisableNameChecking -Force $baseDir/BuildNuGetPacakges

#
# Helper function - check to see if a given release has already been
# built
#
function Check-If-Built ($version, $nugetDir, $wrapperVersion)
{
	$versionInfo = "v$($version.VersionMajor).$($version.VersionMinor).$($version.VersionSubMinor)$($version.VersionExtraInfo).win32.vc10.$($wrapperVersion)"
	$pkgNames = Get-NuGet-Package-Names $versionInfo | % {"$($_).nupkg"}
	$foundPkgNames = $pkgNames | ? {Test-Path -Path "$nugetDir/$_" }
	return $pkgNames.Count -eq $foundPkgNames.Count
}

function Get-Build-Attempted-Filename ($version, $nugetDir, $wrapperVersion)
{
    $corePkg = "ROOTNET-Core-v$($version.VersionMajor).$($version.VersionMinor).$($version.VersionSubMinor).win32.vc10.$($wrapperVersion).attempted"
    return "$nugetDir/$corePkg"
}

#
# Helper function to see if a given release build was attempted already
#
function Check-If-Build-Attempted ($version, $nugetDir, $wrapperVersion)
{
    return Test-Path -Path $(Get-Build-Attempted-Filename $version $nugetDir $wrapperVersion)
}

#
# Get a list of all packages that are up on ROOT servers. It is very
# likely that we will have build some of them already, so we should
# x-check that to see if we can find them.
#

$unbuiltReleases = Get-Subsequent-Releases $minROOTVersion | ? {$_.DownloadType -eq "win32.vc10"} | ? {-not $(Check-If-Built $_ $nugetDir $wrapperVersion)}

$ignoreReleases = $unbuiltReleases | ? {Check-If-Build-Attempted $_ $nugetDir $wrapperVersion}
if ($ignoreReleases)
{
    Write-Host "Ignoring the following releases because we've already attempted (and failed) to build them:"
    $ignoreReleases | Format-Table -Property VersionMajor,VersionMinor,VersionSubMinor,VersionExtraInfo,DownloadType
    Write-Host ""
}

$goodReleases = $unbuiltReleases | ? { -not $(Check-If-Build-Attempted $_ $nugetDir $wrapperVersion)}
Write-Host "Found the following releases that need building:"
$goodReleases | Format-Table -Property VersionMajor,VersionMinor,VersionSubMinor,VersionExtraInfo,DownloadType

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
        remove-item "$baseDir\Wrapper Generators\*.txt"
    } else
	{
		Write-Host "No previous results found in $libDir"
	}

    #
    # Next, do the build.
    #

	$buildOK = $false
	try {
	    Build-ROOT-Dot-Net -Version $wrapperVersion -BuildLocation $baseDir -ROOTURL $r.URL -showLog $false -nugetDir $nugetDir
		$buildOK = $true
	} catch
	{
		$buildOK = $false
		Write-Host "Build failed:"
		Write-Host $_.exception
	}
	
	#
	# Do a check to make sure all worked correctly. Complain in a way that the build server can sort out when things don't
	# go well. And also make sure that we leave behind an attempted file so we don't keep trying to redo this guy.
	#
	
	if ($buildOK)
	{
		# Not sure why i have to re-import, but it would seem that I do...
		Import-Module -DisableNameChecking -Force $baseDir/BuildNuGetPacakges
		$buildOK = Check-If-Built $r $nugetDir $wrapperVersion
	}

	if (-not $buildOK)
	{
		Write-Host "Wrapper build failed for $($r.VersionMajor).$($r.VersionMinor).$($r.VersionSubMinor)$($r.VersionExtraInfo)"
		Write-Host "   -> $($r.URL)"
        $attemptedFile = Get-Build-Attempted-Filename $r $nugetDir $wrapperVersion
        "done" | Out-File $attemptedFile
	}
}

﻿#
# These function help control and run the build for a ROOT.NET set of wrappers.
#

#
# Download the root package from the web.
#
function DownloadFromWeb($url, $local)
{
    if (-not (test-path $local))
    {
        if (-not $url.Contains("//"))
        {
            copy-item $url $local
        }
        else
        {
            Write-Host "Downloading $url"
            $client = new-object System.Net.WebClient
            $client.DownloadFile( $url, $local )
        }
    }
}

# Uncompress the file - as long as it hasn't been uncompressed already, in
# which case skip this step.
$SevenZipExe = "C:\Program Files\7-Zip\7z.exe"
function Uncompress($path, $logDir)
{
    if (-not $path.EndsWith(".tar.gz"))
    {
        throw "Only know how to uncompress .tar.gz files - $path"
    }

    $logFile = "$logDir/uncompress.log"
    $uncompressFlag = "$path-uncompressed"
    
    if (-not (test-path $uncompressFlag)) {
      Write-Host "Uncompressing $path"
      "Uncompressing $path" | out-file -filepath $logFile -append

      $tarfileName = split-path -leaf ($path.SubString(0, $path.Length-3))

      if (-not (test-path $tarfileName)) {
        
        & "$SevenZipExe" x -y $path | out-file -filepath $logFile -append
      }
      Write-Host $tarfileName
      if (-not (test-path $tarfileName)) {
        throw "Could not find the tar file $tarfile after uncompressoing $path"
      }

      & "$SevenZipExe" x -y $tarfileName | out-file -filepath $logfile -append

      $bogus = new-item $uncompressFlag -type file
    }
}

#
# Copy an item with copy-item as long as it already exists.
#
function copy-if-present ($source, $dest)
{
    if (test-path $source)
    {
        copy-item $source $dest
    }
}

#
# Build the wrappers - everything...
#
function BuildROOTNET ($rootLoc, $rootdotnetloc, $finalbuild, $fullVersion, $logDir)
{
	#
	# Strip any pre-release crap out of the version number for including in the DLL's.
	#
	
	$version = $fullVersion.Split("-")[0]
	
	#
	# Check to see if the build went forward or not
	#
	
	Set-Location $rootdotnetloc
	if (Test-Path "Wrappers\AutoBuild\Release\libCoreWrapper.dll")
	{
		return
	}
	
	#
	# Now, do the build.
	#

    $buildDir = "$rootdotnetloc/Wrapper Generators"
    set-location $buildDir
    Write-Host "Building wrappers..."
    Write-Host "Location is $($PWD.PATH)"
    & DoEverythingAtOnce\BuildROOTWrappers.bat "$rootLoc" AutoBuild "$finalbuild" $version 2>&1 | out-file "$logDir/build.log"
    
    #
    # A few log files are generated by that script. We need to copy them over
    # to our logs directory so they can be used by report generation.
    #
     
    copy-if-present "$buildDir/bad_headers.txt" $logDir
    copy-if-present "$buildDir/conversion_errors.txt" $logDir    
    copy-if-present "$rootdotnetloc\Wrappers\AutoBuild\library_dependencies.txt" $logDir
}

#
# Build a root release in the given spot.
# $baseDir $Version $ROOTURL
#
# releaseDir - where the wrappers stuff has been pulled down.
# version - the wrapper version number
# ROOTURL - the URL of the root dude we are going to download.
#
function ReleaseBuild($buildDir, $ROOTURL, $version)
{
    #
    # Create the directory if it isn't already
    #

    if (-not (test-path $buildDir -PathType container))
    {
        $bogus = New-Item $buildDir -type directory
    }
    $logDir = "$buildDir/logs"
    if (-not (test-path $logDir -PathType container))
    {
        $bogus = new-item $logDir -type directory
    }

	#
	# See if the last build file is there. If so, then the build was just fine.
	#
	
	$lastBuildFile = "$buildDir/build_done.txt"
	if (Test-Path $lastBuildFile)
	{
		return
	}

    #
    # Next, see if the ROOT file isn't downloaded, then go get it.
    #

    $rootFilename = ($ROOTURL -split {$_ -eq "/" -or $_ -eq "\"})[-1]
    $rootArchivePath = "$buildDir\$rootFilename"
    DownloadFromWeb $ROOTURL $rootArchivePath
    $rootsys = "$buildDir/root"
    if (-not (test-path $rootsys))
    {
        $bogus = new-item $rootsys -type directory
    }
    set-location $buildDir
    Uncompress $rootArchivePath $logDir

    #
    # Next, download the proper version of ROOT.NET for the build... this is the
	# original guy handed into us.
    #

    $rootdotnetloc = "$buildDir\.."
    
    #
    # If we are here, then we need to do the build.
    #

    $finalLibraryBuild = "$buildDir/build"
	Set-Location $buildDir
    BuildROOTNET "$buildDir/root" $rootdotnetloc $finalLibraryBuild $version $logDir

    #
    # Generate a log report of some of the stuff we disabled for later use
    #
    
    & "$rootdotnetloc\Wrapper Generators\Release\DumpConfigInfo.exe" > "$logDir\disabled_items.txt"
    
    #
    # Remember we did the build
    #
    
    "Done" | out-file -filepath $lastBuildFile
    
    return 
}

#
# Given a location where the base ROOT.NET wrapers have been unwrapped, and a URL
# where we should get the ROOT file from, build the ROOT.NET... If the stuff is here,
# no need to re-build!
#
# Version is the version of the wrappers we are building against.
#
function Build-ROOT-Dot-Net($Version, $BuildLocation, $ROOTURL, $showLog = $false, $nugetDir = "", $NuGetApiKey = "", [switch] $Publish)
{
    # Sanity check. Mostly needed when we run from the command line to test
    if (-not (Test-Path "./Wrapper Generators")) {
        throw "Not in the ROOT.NET main directory. Aborting"
    }

    # Build the directories, load what we need.
    $baseDirName = ($ROOTURL -split {$_ -eq "/" -or $_ -eq "\"})[-1]
    $baseDir = "$BuildLocation\RDN-$Version-$baseDirName"
	Import-Module -DisableNameChecking -Force ./GenerateReport
	Import-Module -DisableNameChecking -Force ./BuildNuGetPacakges
    
    ReleaseBuild $baseDir $ROOTURL $Version

	Write-Host "Generating build report..."
    Generate-Report "$baseDir\logs"

	Write-Host "Generating nuget packages..."
    $isDebug = $baseDirName.Contains(".debug.")
    $packages = Build-NuGet-Pacakges $BuildLocation "$baseDir\logs" $Version -KeepPDB:$isDebug -CopyTo $nugetDir -Publish:($Publish) -NuGetApiKey $NuGetApiKey
	
	if ($showLog)
	{
		foreach($l in $(Get-Content $baseDir\logs\buildreport.txt))
		{
			Write-Host $l
		}
	}
}

Export-ModuleMember -Function Build-ROOT-Dot-Net
# Example:
# Build-ROOT-Dot-Net -Version v2.7 -BuildLocation ${PWD} -ROOTURL ftp://root.cern.ch/root/root_v5.34.20.win32.vc11.tar.gz -showLog $true
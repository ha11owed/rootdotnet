#
# Download the ROOT.NET software and a version of ROOT.NET and do the complete
# build, including the nuget packages.
#

#
# Config
#

$SevenZipExe = "C:\Program Files\7-Zip\7z.exe"
# We depend on svn path from http://sourceforge.net/projects/win32svn/files/
$SVNExe = "C:\Users\gwatts\Documents\ATLAS\Projects\svn\svn-win32-1.6.17\bin\svn.exe"

#
# Helper funciton
#

# download from the web if the file is not already there
function global:DownloadFromWeb($url, $local)
{
    if (-not (test-path $local))
    {
        Write-Host "Downloading $url"
        $client = new-object System.Net.WebClient
        $client.DownloadFile( $url, $local )
     }
}

#
# Do the actual build
#
function global:BuildROOTNET ($rootdotnetloc, $finalbuild, $logDir)
{
    $buildDir = "$rootdotnetloc/Wrapper Generators"
    set-location $buildDir
    Write-Host "Building wrappers..."
    & DoEverythingAtOnce\BuildROOTWrappers.bat "$rootdotnetloc" AutoBuild "$finalbuild" 2>&1 | out-file "$logDir/build.log"
    
    #
    # A few log files are generated by that script. We need to copy them over
    # to our logs directory so they can be used by report generation.
    #

    copy-item "$buildDir/bad_headers.txt" $logDir
    copy-item "$buildDir/conversion_errors.txt" $logDir    
}

# Uncompress the file - as long as it hasn't been uncompressed already, in
# which case skip this step.
function global:Uncompress($path, $logDir)
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
      if (-not (test-path $tarfileName)) {
        throw "Could not find the tar file $tarfile after uncompressoing $path"
      }

      & "$SevenZipExe" x -y $tarfileName | out-file -filepath $logfile -append

      $bogus = new-item $uncompressFlag -type file
    }
}

#
# Down load the ROOTNET tools from some source.
#
function global:DownloadROOTNET ($source, $destpath, $logDir)
{
    if ($source -ne "HEAD")
    {
        throw "Don't know how to get ROOT.NET from '$source'"
    }
    
    if (-not (test-path $destpath))
    {
        $bogus = new-item $destpath -type directory
    }
    
    #
    # We always do the update - just incase there has been a change
    #

    Write-Host "Updating ROOT.NET from SVN HEAD"    
    & $SVNExe co https://rootdotnet.svn.codeplex.com/svn $destpath | out-file -filepath "$logDir/snvco.log" -append
}

#
# Build a root release in the given spot
#
function global:ReleaseBuild($rootDirectory, $ROOTdotNETSource, $rootRelease)
{
    #
    # Create the directory if it isn't already
    #

    if (-not (test-path $rootDirectory -PathType container))
    {
        $bogus = New-Item $rootDirectory -type directory
    }
    $logDir = "$rootDirectory/logs"
    if (-not (test-path $logDir -PathType container))
    {
        $bogus = new-item $logDir -type directory
    }

    #
    # Next, see if the ROOT file isn't downloaded, then go get it.
    #

    $rootFilename = ($rootRelease -split "/")[-1]
    $rootArchivePath = "$rootDirectory\$rootFilename"
    DownloadFromWeb $rootRelease $rootArchivePath
    $rootsys = "$rootDirectory/root"
    if (-not (test-path $rootsys))
    {
        $bogus = new-item $rootsys -type directory
    }
    set-location $rootDirectory
    Uncompress $rootArchivePath $logDir

    #
    # Next, download the proper version of ROOT.NET for the build
    #

    $rootdotnetloc = "$rootDirectory/rootdotnet"
    DownloadROOTNET $ROOTdotNETSource $rootdotnetloc $logDir

    #
    # Great! Now we are ready to do the actual build
    #

    $finalLibraryBuild = "$rootDirectory/build"
    BuildROOTNET $rootdotnetloc $finalLibraryBuild $logDir
}

#
# Uncomment the below lines for testing
#
#$rootDirectory = "c:\users\gwatts\Documents\rootrelease"
#$ROOTdotNETSource = "HEAD"
#$rootRelease = "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz"
#ReleaseBuild $rootDirectory $ROOTdotNETSource $rootRelease

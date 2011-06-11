#
# Download the ROOT.NET software and a version of ROOT.NET and do the complete
# build, including the nuget packages.
#

#
# Parameters
#

$rootDirectory = "c:\users\gwatts\Documents\rootrelease"
$ROOTdotNETSource = "HEAD"
$rootRelease = "ftp://root.cern.ch/root/root_v5.30.00-rc1.win32.vc10.tar.gz"

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
function DownloadFromWeb($url, $local)
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
function BuildROOTNET ($rootdotnetloc, $finalbuild)
{
    set-location "$rootdotnetloc/Wrapper Generators"
    & DoEverythingAtOnce\BuildROOTWrappers.bat "$rootdotnetloc" AutoBuild "$finalbuild"
}

# Uncompress the file - as long as it hasn't been uncompressed already, in
# which case skip this step.
function Uncompress($path)
{
    if (-not $path.EndsWith(".tar.gz"))
    {
        throw "Only know how to uncompress .tar.gz files - $path"
    }

    $uncompressFlag = "$path-uncompressed"
    if (-not (test-path $uncompressFlag)) {
      Write-Host "Uncompressing $path"

      $tarfileName = split-path -leaf ($path.SubString(0, $path.Length-3))

      if (-not (test-path $tarfileName)) {
        & "$SevenZipExe" x -y $path
      }
      if (-not (test-path $tarfileName)) {
        throw "Could not find the tar file $tarfile after uncompressoing $path"
      }
      
      & "$SevenZipExe" x -y $tarfileName
      Write-Host $runResult
      
      $bogus = new-item $uncompressFlag -type file
    }
}

#
# Down load the ROOTNET tools from some source.
#
function DownloadROOTNET ($source, $destpath)
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
    
    & $SVNExe co https://rootdotnet.svn.codeplex.com/svn $destpath
}

#
# Create the directory if it isn't already
#

if (-not (test-path $rootDirectory -PathType container))
{
    $bogus = New-Item $rootDirectory -type directory
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
set-location $rootsys
Uncompress $rootPath

#
# Next, download the proper version of ROOT.NET for the build
#

$rootdotnetloc = "$rootDirectory/rootdotnet"
DownloadROOTNET $ROOTdotNETSource $rootdotnetloc

#
# Great! Now we are ready to do the actual build
#

$finalLibraryBuild = "$rootDirectory/build"
BuildROOTNET $rootdotnetloc $finalLibraryBuild

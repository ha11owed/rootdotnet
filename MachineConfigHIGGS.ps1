#
# This contains the various config parameters
# for this machine for the build process. Please customize
# and place in the same directory as all the other scripts
#

# Where the build should be written... hopefully a spare disk!
$global:buildLocation = "D:\users\gwatts\ROOT.NET\builds"

# Where finally built nuget packages should end up
$global:nugetPackageLocation = "\\deeptalk.phys.washington.edu\Packages"

# Where can we find a recent version of the nuget executable!?
$global:nugetExe = "D:\users\gwatts\ROOT.NET\nuget.exe"

# We use 7zip to unzip the downloads from the ROOT web site
$global:SevenZipExe = "C:\Program Files\7-Zip\7z.exe"

# Copy the build logs to a final directory
$global:BuildLogDirectory = "\\DEEPTALK\RDNBuildLogs"
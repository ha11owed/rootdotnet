#
# This contains the various config parameters
# for this machine for the build process. Please customize
# and place in the same directory as all the other scripts
#

# Where the build should be written... hopefully a spare disk!
$global:buildLocation = "c:\users\gwatts\Documents\rootbuilds"

# Where can we find a recent version of the nuget executable!?
$global:nugetExe = "C:\Users\gwatts\Documents\ATLAS\Projects\LINQToROOT\LINQToTTree\nuget.exe"

# We use 7zip to unzip the downloads from the ROOT web site
$global:SevenZipExe = "C:\Program Files\7-Zip\7z.exe"

# temp place to copy over nuget packages
$global:nugetPackageLocation = "C:\Users\gwatts\Documents\nuget"
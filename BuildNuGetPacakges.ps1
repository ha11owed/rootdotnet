#
# Builds the nuget packages for this root guy.
# Starts with a well and totally build set of libraries
# and wrapper applications built by the global build script.
#

$rootDirectory = "c:\users\gwatts\Documents\rootrelease"

#
# Config info
#
$nugetExe = "C:\Users\gwatts\Documents\ATLAS\Projects\LINQToROOT\LINQToTTree\nuget.exe"

################################
# Helper functions

#
# Extract the version number info from the location where we are looking
#
function GetRootVersion($path)
{
    $files = "$path\*.tar.gz" | resolve-path
    $name = $files | get-childitem -name
    $name = $name.substring(0, $name.Length-7) # Get rid of .tar.gz
    if (-not $name.StartsWith("root_"))
    {
        throw "Root package download name $name is not in the expected format"
    }
    $name = $name.Substring(5)
    return $name
}

#
# Search library path for the library name that is coming in.
#
filter FindLibrary($libPath)
{
    if (test-path "$libPath\$_")
    {
        return "$libPath\$_"
    }
    if (test-path "$libPath\${_}.dll")
    {
        return "$libPath\${_}.dll"
    }
    if (test-path "$libPath\lib${_}Wrapper.dll")
    {
        return "$libPath\lib${_}Wrapper.dll"
    }
    throw "Unable to locate library $_"
}

# Build the long package name from the short package name
filter BuildLongPackageName($rootVersion)
{
        return "ROOTNET-$_-$rootVersion"
}

#
# Build a nuget package. Reference for file layout from
# http://docs.nuget.org/docs/reference/nuspec-reference
#
function BuildNugetPackage ($packShortName, $packageVersion, $rootVersion, $libraryList, $dependencyList, $nuGetPacakgesPath)
{
    $packageName = $packShortName | BuildLongPackageName($rootVersion)

    #
    # First job is to translate the library list into a list of actual dll's that we can
    # package up!
    #
    
    $libPath = "$rootDirectory\rootdotnet\Wrappers\AutoBuild\Release"
    $libraries = $libraryList | FindLibrary($libPath)
    $libraryNames = ($libraries | get-childitem -name)
    
    #
    # Now, dump out the nuspec file. We don't use conventions - why copy
    # things over multiple times? :-)
    #

    $path = "$nuGetPacakgesPath\$packageName.nuspec"
    set-location $nuGetPackagesPath
    
    '<?xml version="1.0"?>' > $path
    '<package xmlns="http://schemas.microsoft.com/packaging/2010/07/nuspec.xsd">' >> $path
    '  <metadata>' >> $path
    "    <id>$packageName</id>" >> $path
    #"    <title>ROOT.NET $packageSinglename for ROOT version $rootVersion</title>" >> $path
    "    <version>$packageVersion</version>" >> $path
    "    <authors>Gordon Watts</authors>" >> $path
    "    <owners>Gordon Watts</owners>" >> $path
    "    <licenseUrl>http://rootdotnet.codeplex.com/license</licenseUrl>" >> $path
    "    <projectUrl>http://rootdotnet.codeplex.com/</projectUrl>" >> $path
    #"    <iconUrl>http://ICON_URL_HERE_OR_DELETE_THIS_LINE</iconUrl>" >> $path
    "    <requireLicenseAcceptance>false</requireLicenseAcceptance>" >> $path
    "    <description>Part of the ROOT.NET library that allows you to access the C++ ROOT data anlaysis pacakge (http://root.cern.ch) from .NET languages. Contains $libraryNames, and is only built for this version of root!</description>" >> $path
    "    <tags>ROOT Data Analysis Science</tags>" >> $path
    if ($dependencyList)
    {
        "    <dependencies>" >> $path
        foreach ($dep in ($dependencyList | BuildLongPackageName($rootVersion)))
        {
            "      <dependency id=`"$dep`" version=`"[$packageVersion]`" />" >> $path
        }
        "    </dependencies>" >> $path
    }
    "  </metadata>" >> $path
    
    #
    # First, the library files
    #
    
    "  <files>" >> $path
    
    foreach ($l in $libraries)
    {
        "    <file src=`"$l`" target=`"lib\net40`" />" >> $path
    }
    
    #
    # Done!
    #
    
    
    "  </files>" >> $path
    "</package>" >> $path
    
    #
    # Final task, run nuget on the thing to actually build it!
    #
    
    $nugetlogs = "$rootdirectory\logs\$packageName.log"
    & $nugetExe pack $path
}

################################

#
# Make sure the final output area is there
#

$nuGetPackagesPath = "$rootDirectory\nuget"
if (-not (test-path $nuGetPackagesPath))
{
    $bogus = new-item $nuGetPackagespath -type directory
}

$rootVersionInfo = GetRootVersion $rootDirectory
$rootDotNetVersionInfo = "1.0.0"

$packages = @{
                "Core" = @{
                            "Libraries" = "Core", "MathCore", "Hist", "RIO", "WrapperPlumbingLibrary";
                            "Dependencies" = @()
                          };
                "Graphing" = @{
                                "Libraries" = "Gpad", "Graf", "Graf3d";
                                "Dependencies" = @("Core")
                              }
             }
foreach ($pack in $packages.Keys)
{
    #"ROOT.NET-$pack-$rootVersionInfo"
    BuildNugetPackage $pack $rootDotNetVersionInfo $rootVersionInfo $packages[$pacK]["Libraries"] $packages[$pacK]["Dependencies"] $nuGetPackagesPath
}
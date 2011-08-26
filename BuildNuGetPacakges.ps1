#
# Builds the nuget packages for this root guy.
# Starts with a well and totally build set of libraries
# and wrapper applications built by the global build script.
#

#
# Config info
#

################################
# Helper functions

#
# Extract the version number info from the location where we are looking
#
function global:GetRootVersion($path)
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
filter global:FindLibrary($libPath)
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
filter global:BuildLongPackageName($rootVersion)
{
    return "ROOTNET-$_-$rootVersion"
}

#
# Build a nuget package. Reference for file layout from
# http://docs.nuget.org/docs/reference/nuspec-reference
#
function global:BuildNugetPackage ($packShortName, $packageVersion, $rootVersion, $libraryList, $dependencyList, $nuGetPacakgesPath, $logDir)
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
    
    & $nugetExe pack $path 2>&1 > $logPath
    
    return "$nuGetPacakgesPath\$packageName.$packageVersion.nupkg"
}

filter global:GetDependentLibraryNames($allPacks)
{
    $packages = @()
    $allDepLibsDeep = $allPacks[$_]["Dependencies"] | GetDependentLibraryNames $allPacks
    $allDepLibs = @(@($allDepLibsDeep, $allPacks[$_]["Libraries"]) | % {$_}) | ? {$_}
    return $allDepLibs
}

function global:NotOneOf ($originals, $val)
{
    foreach($i in $originals)
    {
        if ($i -eq $val)
        {
            return 0
        }
    }
    return 1
}

################################

function global:BuildNuGetPacakges($rootDirectory, $currentBuildRevision=-1)
{
    #
    # Make sure the final output area is there
    #

    $nuGetPackagesPath = "$rootDirectory\nuget"
    $logsPath = "$rootDirectory\logs"
    if (-not (test-path $nuGetPackagesPath))
    {
        $bogus = new-item $nuGetPackagespath -type directory
    }

    #
    # Check to see if we can find the last-built rev number. If so, then we
    # have already done as much as we can, so off we go.
    #
    
    $lastNugetBuildRevPath = "$nuGetPackagesPath\last-rev.txt"
    if (($currentBuildRevision -eq -1) -or (test-path $lastNugetBuildRevPath))
    {
        $lastRev = get-content -Path $lastNugetBuildRevPath
        if ($lastRev -eq $currentBuildRevision)
        {
            return
        }
    }
    
    #
    # Ok, next we can do the actual extraction
    #
    
    $rootVersionInfo = GetRootVersion $rootDirectory
    $rootDotNetVersionInfo = "1.0.0"

    $packages = @{
                    "Core" = @{
                                "Libraries" = "Core", "MathCore", "Hist", "RIO", "Physics", "WrapperPlumbingLibrary";
                                "Dependencies" = @()
                              };
                    "Graphing" = @{
                                    "Libraries" = "Gpad", "Graf", "Graf3d";
                                    "Dependencies" = @("Core")
                                  }
                    "PROOF" = @{
                                    "Libraries" = "Proof", "ProofPlayer", "ProofDraw", "Proofx";
                                    "Dependencies" = @("Core")
                               }
                    "All" = @{
                                "Dependencies" = @("Graphing", "PROOF")
                                "Libraries" = @()
                             }
                 }
    
    #
    # Correctly build the "All" package - this package needs all the wrappers we
    # have built, and should depend on the other two.
    #

    $libPath = "$rootDirectory\rootdotnet\Wrappers\AutoBuild\Release"
    $allRefLibraries = "All" | GetDependentLibraryNames $packages | FindLibrary $libPath | get-childitem -name
    $allLeftoverLibraries = get-childitem -name "$libPath\*.dll" | ? {NotOneOf $allRefLibraries $_}
    $packages["All"]["Libraries"] = $allLeftoverLibraries

    #
    # Package up all the libraries now. Once created, copy them over if we know where to copy them to.
    #
    
    foreach ($pack in $packages.Keys)
    {
        #"ROOT.NET-$pack-$rootVersionInfo"
        $pkg = BuildNugetPackage $pack $rootDotNetVersionInfo $rootVersionInfo $packages[$pacK]["Libraries"] $packages[$pacK]["Dependencies"] $nuGetPackagesPath $logsPath
        if ($nugetPackageLocation)
        {
            copy-item $pkg $nugetPackageLocation
        }
    }
    
    #
    # Record what we've done so we don't need to do it again!
    #
    
    $currentBuildRevision | out-file -filepath $lastNugetBuildRevPath
}

# Uncomment for testing.
$scriptLocation = Split-Path -parent $MyInvocation.MyCommand.Definition
#& "$scriptLocation\MachineConfigTOUCHME.ps1"
#BuildNuGetPacakges "C:\Users\gwatts\Documents\rootbuilds\root_v5.30.00-rc1.win32.vc10.tar.gz"

#& "$scriptLocation\MachineConfigHIGGS.ps1"
#BuildNuGetPacakges "D:\users\gwatts\ROOT.NET\builds\root_v5.28.00d.win32.vc10.debug.tar.gz"

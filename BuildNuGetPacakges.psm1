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
	$l = $_
	$mustFind = $true
	if ($l[0] -eq "?")
	{
		$mustFind = $false
	}
    if (test-path "$libPath\$l")
    {
        return "$libPath\$l"
    }
    if (test-path "$libPath\${l}.dll")
    {
        return "$libPath\${l}.dll"
    }
    if (test-path "$libPath\lib${l}Wrapper.dll")
    {
        return "$libPath\lib${l}Wrapper.dll"
    }

	#
	# Depending on how important it is...
	#
	
	if (-not $mustFind) {
		return ""
	}
	
    throw "Unable to locate library $l (in '$libPath')"
}

# Build the long package name from the short package name
filter BuildLongPackageName($rootVersion)
{
    return "ROOT.NET.$_"
}

function Get-ScriptDirectory ()
{
    Split-Path $ExecutionContext.SessionState.Module.Path
}

#
# Build a nuget package. Reference for file layout from
# http://docs.nuget.org/docs/reference/nuspec-reference
#
function BuildNugetPackage ($packShortName, $packageVersion, $rootVersion, $libraryList, $dependencyList, $nuGetPacakgesPath, $logDir, [Switch] $KeepPDB)
{
    $packageName = $packShortName | BuildLongPackageName($rootVersion)

    #
    # First job is to translate the library list into a list of actual dll's that we can
    # package up!
    #
    
    $libPath = "$rootDirectory\Wrappers\AutoBuild\Release"
    $libraries = $libraryList | FindLibrary($libPath) | ? {$_}
	$libraryNames = ($libraries | get-childitem -name)

    #
    # The version number is a combination of the ROOT version and our version.
    #
        
    $splitVersion = $rootVersion -split "\."
    $ROOTVersion = $splitVersion[0].Substring(1) + "." + $splitVersion[1] + "." + $splitVersion[2]
    $nugetVersion = $rootVersion + "." + $packageVersion

    #
    # Now, dump out the nuspec file. We don't use conventions - why copy
    # things over multiple times? :-)
    #

    $path = "$nuGetPacakgesPath\$packageName.nuspec"
    set-location $nuGetPackagesPath
    
    '<?xml version="1.0"?>' > $path
    '<package xmlns="http://schemas.microsoft.com/packaging/2010/07/nuspec.xsd">' >> $path
    '  <metadata>' >> $path
    "    <id>ROOT.NET.$packShortName</id>" >> $path
    "    <title>ROOT.NET $packShortName for ROOT version $rootVersion</title>" >> $path
    "    <version>$nugetVersion</version>" >> $path
    "    <authors>Gordon Watts</authors>" >> $path
    "    <owners>Gordon Watts</owners>" >> $path
    "    <licenseUrl>http://rootdotnet.codeplex.com/license</licenseUrl>" >> $path
    "    <projectUrl>http://rootdotnet.codeplex.com/</projectUrl>" >> $path
    #"    <iconUrl>http://ICON_URL_HERE_OR_DELETE_THIS_LINE</iconUrl>" >> $path
    "    <requireLicenseAcceptance>false</requireLicenseAcceptance>" >> $path
    "    <description>Part of the ROOT.NET library that allows you to access the C++ ROOT data anlaysis pacakge (http://root.cern.ch) from .NET languages. Contains $libraryNames, and is only built for this version of root!</description>" >> $path
    "    <summary>ROOT.NET wrappers for $libraryNames</summary>" >> $path
    "    <tags>ROOT Data Analysis Science</tags>" >> $path
    if ($dependencyList)
    {
        "    <dependencies>" >> $path
        foreach ($dep in ($dependencyList | BuildLongPackageName($rootVersion)))
        {
            "      <dependency id=`"$dep`" version=`"[$nugetVersion]`" />" >> $path
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
        if ($KeepPDB)
        {
            $pdb = $l.Replace(".dll", ".pdb")
            "    <file src=`"$pdb`" target=`"lib\net40`" />" >> $path
        }
    }
    
    #
    # The dynamic library is special, as we need to copy over the targets file that will download and install
    # the ROOT binaries if they aren't already on the system.
    #

    if ($packShortName -eq "Dynamic") {
        $scriptDir = Get-ScriptDirectory
        ($(Get-Content "$scriptDir/ROOT.targets") -replace "5.34.20",$ROOTVersion) -replace "vc11",$splitVersion[4] | Set-Content "$nuGetPacakgesPath/ROOT.NET.Dynamic.targets"
        "    <file src=`"ROOT.NET.Dynamic.targets`" target=`"build\net40`" />" >> $path
        "    <file src=`"..\..\ROOTPackageBuilders\ROOTMSBuildTasks\bin\Release\ROOTMSBuildTasks.dll`" target=`"tools\net40`" />" >> $path
    }

    #
    # Done!
    #
    
    "  </files>" >> $path
    "</package>" >> $path
    
    #
    # Final task, run nuget on the thing to actually build it!
    #
    
    "Nuget Package $packageName" | Out-File $logDir/nuget.txt -Append
    nuget pack $path 2>&1 >> $logDir/nuget.txt
    
    return "$nuGetPacakgesPath\$packageName.$nugetVersion.nupkg"
}

filter GetDependentLibraryNames($allPacks)
{
    $packages = @()
    $allDepLibsDeep = $allPacks[$_]["Dependencies"] | GetDependentLibraryNames $allPacks
    $allDepLibs = @(@($allDepLibsDeep, $allPacks[$_]["Libraries"]) | % {$_}) | ? {$_}
    return $allDepLibs
}

function NotOneOf ($originals, $val)
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

# Load a dictionary from a x: x y z type of file.
function load-dict ($fname)
{
    $result = @{}
    $sLines = get-content $fname
    foreach ($l in (get-content $fname))
    {
        $lst = $l -split " ", 2
        $k = ($lst[0].Substring(3)).Replace(":", "")
        $v = $lst[1] -split " " | ? {$_.Length -gt 0} | % {$_.Substring(3)}
        $result[$k] = $v
    }
    return $result
}

#
# Key-value pair with lists that reference each other - see if we can find
# everything below.
#
function recursive-resolve ($dict)
{
    process
    {
        if (-not $dict.ContainsKey($_))
        {
            return @($_)
        }
        $sublist = $dict[$_] | recursive-resolve($dict)
        return ((@($_) + $sublist) | sort -unique)
    }
}

#
# We use some special characters to indicate things like
# optional includes. Strip those off and get the naked library
# name here.
#
function actual-library-name ($libname)
{
    if ($libname[0] -eq '?')
    {
        return $libname.SubString(1)
    }
    return $libname
}

#
# Pipe that takes a library name as input and attempts to find everything that is needed for the dependncy
# as output. We assume there is no circular dependency! :-)
#
function get-library-dependencies ($depFile)
{
    begin
    {
        $baseDict = load-dict $depFile
        $final = @{}
        foreach ($k in $baseDict.Keys)
        {
            $final[$k] = $baseDict[$k] | recursive-resolve $baseDict | sort -unique
        }
    }
    
    process
    {
        return @($_) + $final[$(actual-library-name $_)]
    }
}

################################

#
# This is the listing of the packages that we are going to build. This is a map of maps. The key
# for the top level map is the name of the nuget packages.
#
# The second level map contians two keys:
#  Libraries => List of the ROOT library names (add Wrapper to them, or raw names) for the .NET distribution.
#               These libraries must exist in the distribution otherwise there is a failure.
#               A "?" in front of the library name means it is ok if the library isn't found.
#  Dependencies => The nuget dependency packages. The library dependency mapper will make sure that all the
#                  right libraries are included in order to make each package a complete set.
#

$packages = @{
				"Dynamic" = @{
								"Libraries" = "Core", "WrapperPlumbingLibrary";
								"Dependencies" = @()
							};
                "Core" = @{
                            "Libraries" = "MathCore", "Hist", "RIO", "Physics";
                            "Dependencies" = @("Dynamic")
                          };
                "Tree" = @{
                            "Libraries" = "Tree", "TreePlayer";
                            "Dependencies" = @("Core")
                          };
                "Graphing" = @{
                                "Libraries" = "Gpad", "Graf", "Graf3d";
                                "Dependencies" = @("Core")
                              }
                "PROOF" = @{
                                "Libraries" = "Proof", "ProofPlayer", "ProofDraw", "?Proofx";
                                "Dependencies" = @("Tree")
                           }
                "All" = @{
                            "Dependencies" = @("Graphing", "PROOF", "Tree")
                            "Libraries" = @()
                         }
             }

#
# Given the version number info, build a list of package names. This is useful from an external
# POV for getting the list of packages we should be building. rootVersion should be of the fairly
# complex form "".
#
function Get-NuGet-Package-Names ($rootVersion)
{
	return $packages.Keys | BuildLongPackageName ($rootVersion)
}

#
# Build the actual nuget packages
#
function Build-NuGet-Pacakges($rootDirectory, $logsPath, $version, [switch] $KeepPDB, $CopyTo = "", $NuGetApiKey = "", [switch] $Publish)
{
    #
    # Make sure the final output area is there
    #

    $nuGetPackagesPath = "$rootDirectory\nuget"
    if (-not (test-path $nuGetPackagesPath))
    {
        $bogus = new-item $nuGetPackagespath -type directory
    }

	$depFile = "$logsPath\library_dependencies.txt"
    if (-not (test-path $depFile))
    {
        throw "Unable to find library dependency file $depFile"
    }
    
    #
    # Get the version info we will need to build the nuget packages.
    #
    
    $rootVersionInfo = GetRootVersion "$logsPath\.."
    $rootDotNetVersionInfo = $version
    
    #
    # The above is what we want. The only problem is that there are lots of dependencies
    # and as a result things like libRIO might pull in libThread or similar. We need to keep
    # our packages "normalized" in that sense - so we need to fix up the above list. We do this
    # now so that when we sort out the "All" package below we don't get it wrong!
    # Once done we need to remove packages that are dependent lower down.
    #
    
    foreach ($pack in $packages.Keys)
    {
        $actual = $packages[$pack]["Libraries"] | get-library-dependencies $depFile | sort -unique
        $packages[$pack]["Libraries"] = $actual
    }
    
    foreach ($pack in $packages.Keys)
    {
        $depLibs = $packages[$pack]["Dependencies"] | GetDependentLibraryNames $packages |? {$_}
        $lookup = @{}
        foreach ($d in $depLibs | ? {$_})
        {
            $lookup[$d] = 1
        }
        $packages[$pack]["Libraries"] = $packages[$pack]["Libraries"] | ? {$_} | ? {-not $lookup.ContainsKey($_)}
    }
    
    #
    # Correctly build the "All" package - this package needs all the wrappers we
    # have built, and should depend on the other two.
    #

    $libPath = "$rootDirectory\Wrappers\AutoBuild\Release"
    $allRefLibraries = "All" | GetDependentLibraryNames $packages | FindLibrary $libPath | get-childitem -name
    $allLeftoverLibraries = get-childitem -name "$libPath\*.dll" | ? {NotOneOf $allRefLibraries $_}
    $packages["All"]["Libraries"] = $allLeftoverLibraries

    #
    # Package up all the libraries now. Once created, copy them over if we know where to copy them to.
    #
    
	$packageLocations = $packages.Keys | % {BuildNugetPackage $_ $rootDotNetVersionInfo $rootVersionInfo $packages[$_]["Libraries"] $packages[$_]["Dependencies"] $nuGetPackagesPath $logsPath -KeepPDB:$KeepPDB}
	
	if ($CopyTo)
	{
		$packageLocations | %{ Copy-Item $_ $CopyTo }
	}
	
	if ($Publish)
	{
		if ($NuGetApiKey)
		{
			$packageLocations | % { nuget push $_ -ApiKey $NuGetApiKey }
		} else
		{
			$packageLocations | % { nuget push $_  }
		}
	}
}

Export-ModuleMember -Function Build-NuGet-Pacakges
Export-ModuleMember -Function Get-NuGet-Package-Names

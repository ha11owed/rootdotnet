#
# Create a report from the log files left over
# from a release build. This file, called buildreport.txt,
# will contain everything to see what happened at a glance.
#

##############
# Helper functions

#
# Parse a vc build file for errors that satisfy one or another type of string
#

function ParseErrors($filePath, $searchString)
{
    $lines = get-content $filePath | select-string $searchString
    $buildMatches = @{}
    foreach ($l in $lines)
    {
        if ($l -match "(\d+)>")
        {
            $buildMatches[$matches[1]] = 1
        }
    }

    $lineDict = @{}
    foreach ($key in $buildMatches.Keys)
    {
        $lineDict[$key] = $lines | select-string -pattern "^$key>"
    }
    return $lineDict
}

function CountStrings($table, $index, $ignoreStrings)
{
    if (-not $table.ContainsKey($index))
    {
        return 0
    }
    
    $goodStrings = $table[$index] | select-string -NotMatch $ignoreStrings
    if (-not $goodStrings)
    {
        return 0
    }
        
    return $goodStrings.Count
}

##############

function Generate-Report($logDir)
{
    #
    # We start off with some general text
    #

    $reportPath = "$logDir\buildreport.txt"
    "ROOT.NET Wrapper Build Report" | out-file -filepath $reportPath
    "=============================" | out-file -filepath $reportPath -append
    Get-Date | out-file -FilePath $reportPath -Append
    "" | out-file -filepath $reportPath -append

    #
    # Dump the SVN number if we can find it.
    #

    $svnInfo = Select-String -Pattern "SVN" -Path $logDir/build.log | % {$_.Line}
    if ($svnInfo)
    {
        $svnInfo | Out-File -FilePath $reportPath -Append
    }

    #
    # Do any headers that failed the conversion process
    #

    "Header Files That Couldn't Be used" | out-file -filepath $reportPath -append
    "----------------------------------" | out-file -filepath $reportPath -append
    "This is usually because they require a header that can't be included by the C++ compiler" | out-file -filepath $reportPath -append
    "These probably cna't be used on Windows at all" | out-file -filepath $reportPath -append
    "" | out-file -filepath $reportPath -append
    get-content "$logDir/bad_headers.txt" | out-file -filepath $reportPath -append

    #
    # Next, do errors we found in the build. Look for "warning" or "error" and filter them out
    #

    "" | out-file -filepath $reportPath -append
    "" | out-file -filepath $reportPath -append
    "Build errors and warnings" | out-file -filepath $reportPath -append
    "----------------------------------" | out-file -filepath $reportPath -append
    "" | out-file -filepath $reportPath -append


    $errorTable = ParseErrors "$logDir/build.log" "fatal error|: error"
    $warningTable = ParseErrors "$logDir/build.log" "warning"

    $projects = get-content "$logDir/build.log" | select-string "Build started:"
    $ignoreWarnings = "C4309",  "LNK4248", "C4305"
    $ignoreErrors = @("freak out dude for empyt array")
    foreach ($proj in $projects)
    {
        if ($proj -match "(\d+)>.+Project: (.+), Configuration") {
            $buildNumber = $matches[1]
            $libName = $matches[2]
            
            $numberErrors = CountStrings $errorTable $buildNumber $ignoreErrors
            $warningErrors = CountStrings $warningTable $buildNumber $ignoreWarnings
            if (($numberErrors -ne 0) -or ($warningErrors -ne 0))
            {
                "" | out-file -filepath $reportPath -append
                "$libName" | out-file -filepath $reportPath -append

                if ($errorTable.ContainsKey($buildNumber))
                {
                    $errorTable[$buildNumber] | out-file -filepath $reportPath -append -width 10000
                }
                if ($warningTable.ContainsKey($buildNumber))
                {
                    $warningTable[$buildNumber] | select-string -NotMatch $ignoreWarnings | out-file -filepath $reportPath -append -width 10000 
                }
            }
        }
    }
    
    #
    # Next, anything that isn't "good" from the config dump
    #
    
    if (Test-Path "$logDir/disabled_items.txt") {
        get-content "$logDir/disabled_items.txt" | out-file -filepath $reportPath -append
    }

    #
    # The conversion failure table
    #

    $conversionErrors = "conversion-errors.txt was not found! Did step fail to complete? See full build log!"
    if (test-path "$logDir/conversion_errors.txt")
    {
        $conversionErrors = get-content "$logDir/conversion_errors.txt" | select-string "In order of least frequent to most frequent:" -context 0,10000
    }
    "" | out-file -filepath $reportPath -append
    "Methods and Classes not converted" | out-file -filepath $reportPath -append
    "---------------------------------" | out-file -filepath $reportPath -append
    "" | out-file -filepath $reportPath -append
    $conversionErrors | out-file -filepath $reportPath -append -width 500

    #
    # And the library dependency table
    #
    
    "" | out-file -filepath $reportPath -append
    "Wrapper Library Dependencies" | out-file -filepath $reportPath -append
    "----------------------------" | out-file -filepath $reportPath -append
    if (Test-Path "$logDir/library_dependencies.txt")
    {
        get-content "$logDir/library_dependencies.txt" | out-file -filepath $reportPath -append
    } else
    {
        "library_dependencies.txt not found - did step fail to run? See full build log!" | out-file -filepath $reportPath -Append
    }
    
    #
    # And last, if there is a log directory, copy everything over there.
    #
    
    if ($BuildLogDirectory)
    {
        $basdir = split-path -leaf (split-path -parent $logDir)
        copy-item $reportPath "$BuildLogDirectory\$basdir.txt"
    }
}

#
# Uncomment the next two lines for testing
#
#$logDir = "C:\Users\gwatts\Documents\rootbuilds\root_v5.30.00-rc1.win32.vc10.tar.gz\logs"
#enerateReport($logDir)

Export-ModuleMember -Function Generate-Report


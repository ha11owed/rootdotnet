#
# Boostrap the build process. This is mostly so
# we can update the other scripts before loading them.
#

$scriptLocation = Split-Path -parent $MyInvocation.MyCommand.Definition

$hostname = gc env:COMPUTERNAME
$machineConfigScript = "$scriptLocation\MachineConfig$hostname.ps1"
if (-not (test-path $machineConfigScript))
{
    Write-Host "$machineConfigScript not found."
    Write-Host "  Please rename MachineConfig.ps1, customize, and run again"
    exit
}

& $machineConfigScript

import-module $scriptLocation\source-control.psm1

#
# Now that we have it, do an svn update locally. This will
# update the master build scripts. Hopefully if this one is updated,
# it isn't in some horrible way!
#

Write-Host "Updating build scripts"
$r = set-revision $scriptLocation -TopLevelOnly

#
# Figure out what file we should use to drive the build
#

$releaseFile = "releases.txt"
if ($Args.length -gt 0)
{
    $releaseFile = $Args[0]
}

if (-not (test-path $releaseFile))
{
    $t = "$scriptLocation\$releaseFile"
    if (test-path $t)
    {
        $releaseFile = $t
    }
}

#
# Now run the master build
#

& "$scriptLocation\BuildAll.ps1" $releaseFile

#
# Build all requested releases. This will scan the 
# packages availible and issue a build for each new one.
#
#
#
[CmdletBinding()]
param (
  [Parameter(mandatory=$true)]
  [string] $minROOTVersion
)

#
# We depend on a number of other modules up and running.
#

#
# Get the list of versions that are the same or
# larger than the version passed in.
#
# For even releases: do everything
# For odd releases: do only the latest
# For a "-rc" release, only do the most recent if there
#  is no non-rc release.
#
function Get-All-Releases ($minVersion)
{
    Import-Module -DisableNameChecking -Force ./
}

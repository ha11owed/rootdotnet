#
# Some functions to get access to the
# stuff in the ROOT repository.
#
# G. Watts
#

filter extract-root-link
{
	return $_.href
}

# Given a ftp to a root file, parse out version, etc. numbers
# from it, and turn it into an object we can use further on down the pipe-line.
filter parse-root-filename
{
	# The last bit of the ftp guy is the name of the file.
	$s = $_.Split("/")
	$fname = $s[$s.length-1]

	if ($fname.StartsWith("root_v")) {
		$mainInfo = $fname.SubString(6).Split('.')
		
		# Sometimes v3 contains extra info...
		$v3 = $mainInfo[2]
		$v3Extra = $v3.SubString(2)
		$v3 = $v3.SubString(0,2)
		
		# Next, figure out how many items are the file type (tar.gz, or msi...)
		$lastindex = $mainInfo.length-1
		if ($mainInfo[$lastindex] = ".gz")
		{
			$lastindex = $lastindex - 2
		}
		$downloadType = [string]::Join(".", $mainInfo[3..$lastindex])
		
		$r = New-Object System.Object
		$r | Add-Member -type NoteProperty -Name "VersionMajor" -Value $mainInfo[0]
		$r | Add-Member -type NoteProperty -Name "VersionMinor" -Value $mainInfo[1]
		$r | Add-Member -type NoteProperty -Name "VersionSubMinor" -Value $v3
		$r | Add-Member -type NoteProperty -Name "VersionExtraInfo" -Value $v3Extra
		$r | Add-Member -type NoteProperty -Name "DownloadType" -Value $downloadType
		$r | Add-Member -type NoteProperty -Name "url" -Value $_
	
		
		return $r
	}
}

#
# filter that returns an array of matches for a givin input string. Use % {$_} to flatten
# the array that comes back.
filter get-matches ([string] $regex = “”) 
{ 
   $returnMatches = new-object System.Collections.ArrayList 

   $resultingMatches = [Regex]::Matches($_, $regex, “IgnoreCase”) 
   foreach($match in $resultingMatches)  
   {  
      [void] $returnMatches.Add($match.Groups[1].Value.Trim()) 
   } 

   return $returnMatches    
} 

#
# Urls in web pages need some fixing up to make them usable out of the context of the webpage.
# Given where we found the web page, do the work.
#
filter fixup-relative-url ($domain, $base)
{
	if ($_.IndexOf("://") -gt 0)
	{
		return $_
	}
	if ($_[0] -eq "/")
	{
		return "$domain$_"
	}

	return "$base$_".Replace("/./", "/")
}

#
# Look at html and extract the a href, and from there, put the links
# together and return them as is.
#
filter parse-html-for-links ($hpath)
{
  $base = $hpath.Substring(0,$hpath.LastIndexOf(“/”) + 1) 
  $baseSlash = $base.IndexOf(“/”, $base.IndexOf(“://”) + 3) 
  $domain = $base.Substring(0, $baseSlash) 
   
  $regex = “<\s*a\s*[^>]*?href\s*=\s*[`"']*([^`"'>]+)[^>]*?>” 
  $allmatches = $_ | get-matches $regex | % {$_} | fixup-relative-url $domain $base
  return $allmatches
}

#
# Returns all versions of root that are on the server currently.
#
function getAllROOTVersions ($htmlPath = "ftp://root.cern.ch/root")
{
	#
	# Use IE to fetch the main download page, and get all the references.
	#
	
	$ie = New-Object -ComObject "InternetExplorer.Application"
	$ie.Navigate("ftp://root.cern.ch/root")
	Start-Sleep -Milliseconds 100
	$links = $ie.Document.body.InnerHTML | parse-html-for-links $htmlPath | % {$_}
	
	return $links | parse-root-filename
}

getAllROOTVersions | Format-Table
#"ftp://root.cern.ch/root/root_v5.33.02.win32.vc10.debug.msi" | parse-root-filename | Format-Table
#"ftp://root.cern.ch/root/root_v5.32.00-rc2.source.tar.gz" | parse-root-filename | Format-Table
#"ftp://root.cern.ch/root/root_v5.28.00g.win32.vc90.tar.gz" | parse-root-filename | Format-Table
#"ftp://root.cern.ch/root/root_v5.32.00.macosx106-i386-gcc-4.2.tar.gz" | parse-root-filename | Format-Table
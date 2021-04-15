if (1 -ne $args.count)
{
	Write-Error "FATAL: expected HDD storage path argument" -ErrorAction Stop
}

$HDDStoragePath = $args[0]
$filterVHD="*.avhd"
$latest1064 = Get-ChildItem -Path $HDDStoragePath -Filter $filterVHD | Sort-Object LastAccessTime -Descending | Select-Object -First 1 $latest1064.name
Write-Host "DisMount VM HDD: "$latest1064
$latest1064 = $HDDStoragePath+"\"+$latest1064
DisMount-VHD -Path $latest1064

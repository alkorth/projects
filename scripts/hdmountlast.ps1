$filterVHD="*.avhd"
$latest1064 = Get-ChildItem -Path "d:\Users\Public\Documents\Hyper-V\Virtual Hard Disks" -Filter "*.avhd" | Sort-Object LastAccessTime -Descending | Select-Object -First 1 $latest1064.name
Write-Host "Mount VM HDD: "$latest1064
$latest1064 = "d:\Users\Public\Documents\Hyper-V\Virtual Hard Disks\"+$latest1064
Mount-VHD -Path $latest1064

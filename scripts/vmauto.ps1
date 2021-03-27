Write-Host "Starts VM execution automation"
Start-VM -Name 'Win81x86'
Stop-VM -Name 'Win81x86' -TurnOff
Mount-VHD -Path 'd:\Users\Public\Documents\Hyper-V\Virtual Hard Disks\vhd3.vhd'
Copy-Item -Path G:\Collected -Destination d:\dataset -recurse -Force
Dismount-VHD -Path 'd:\Users\Public\Documents\Hyper-V\Virtual Hard Disks\vhd3.vhd'
Restore-VMSnapshot -Name 'AV test ready' -VMName Win81x86 -Confirm

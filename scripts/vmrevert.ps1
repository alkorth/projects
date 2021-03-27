Write-Host "Reverts VM execution automation"
Restore-VMSnapshot -Name 'TestReady' -VMName Win81x86 -confirm:$false

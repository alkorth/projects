Write-Host "Reverts VM execution automation"
if (2 -ne $args.count)
{
	Write-Error "FATAL: expected HDD name and Snapshot name arguments" -ErrorAction Stop
}
$VMName = $args[0]
$SnapshotLabel = $args[1]

Restore-VMSnapshot -Name $SnapshotLabel -VMName $VMName -confirm:$false

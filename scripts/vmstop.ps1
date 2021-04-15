Write-Host "Stops VM execution automation"
if (1 -ne $args.count)
{
	Write-Error "FATAL: expected HDD name argument" -ErrorAction Stop
}
$VMName = $args[0]

Stop-VM -Name $VMName -Save

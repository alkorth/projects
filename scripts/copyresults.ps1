Write-Host "Copy run results"
if (2 -ne $args.count)
{
	Write-Error "FATAL: expected HDD mapped folder and destination results folder arguments" -ErrorAction Stop
}

$src = $args[0]
$dest = $args[1]
$num=1

Get-ChildItem -Path $src -Filter *.etl -Recurse | ForEach-Object {

    $nextName = Join-Path -Path $dest -ChildPath $_.name

    while(Test-Path -Path $nextName)
    {
       $nextName = Join-Path $dest ($_.BaseName + "_$num" + $_.Extension)    
       $num+=1   
    }

    $_ | Move-Item -Destination $nextName
}


Get-ChildItem -Path $src -Filter *.syslog -Recurse | ForEach-Object {

    $nextName = Join-Path -Path $dest -ChildPath $_.name

    while(Test-Path -Path $nextName)
    {
       $nextName = Join-Path $dest ($_.BaseName + "_$num" + $_.Extension)    
       $num+=1   
    }

    $_ | Move-Item -Destination $nextName
}


Get-ChildItem -Path $src -Filter *.pid -Recurse | ForEach-Object {

    $nextName = Join-Path -Path $dest -ChildPath $_.name

    while(Test-Path -Path $nextName)
    {
       $nextName = Join-Path $dest ($_.BaseName + "_$num" + $_.Extension)    
       $num+=1   
    }

    $_ | Move-Item -Destination $nextName
}


Get-ChildItem -Path $src -Filter *.hash -Recurse | ForEach-Object {

    $nextName = Join-Path -Path $dest -ChildPath $_.name

    while(Test-Path -Path $nextName)
    {
       $nextName = Join-Path $dest ($_.BaseName + "_$num" + $_.Extension)    
       $num+=1   
    }

    $_ | Move-Item -Destination $nextName
}

Get-ChildItem -Path $src -Filter *.log -Recurse | ForEach-Object {

    $nextName = Join-Path -Path $dest -ChildPath $_.name

    while(Test-Path -Path $nextName)
    {
       $nextName = Join-Path $dest ($_.BaseName + "_$num" + $_.Extension)    
       $num+=1   
    }

    $_ | Move-Item -Destination $nextName
}

param (
    [Parameter(Mandatory = $true)]
    [string]$FolderName
)

$configPath = "D:\Documents\hw_autotune\vitis\$FolderName\hls_config.cfg"

$command = "vitis-run.bat --mode hls --package --config `"$configPath`" --work_dir `"$FolderName`""

Write-Host $command

Invoke-Expression $command

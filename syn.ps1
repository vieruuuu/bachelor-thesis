param (
    [Parameter(Mandatory = $true)]
    [string]$FolderName
)

$configPath = "D:\Documents\hw_autotune\vitis\$FolderName\hls_config.cfg"
$reportFile = "D:\Documents\hw_autotune\vitis\$FolderName\hls\syn\report\csynth.rpt"

$command = "v++.bat -c --mode hls --config `"$configPath`" --work_dir `"$FolderName`""

Write-Host $command

Invoke-Expression $command

Write-Host $reportFile
code $reportFile
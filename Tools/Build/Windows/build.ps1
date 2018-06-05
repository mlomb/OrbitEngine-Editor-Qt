$targetSystem = "Windows"
$ScriptRoot = Split-Path $MyInvocation.MyCommand.Path
$basePath = (Get-Item $ScriptRoot/../../../).FullName
$buildPath = $basePath + "Build\" + $targetSystem

New-Item -ItemType Directory -Force -Path $buildPath | Out-Null

echo "Base Path: $basePath"
echo "Build Path: $buildPath"

echo "********** BUILDING FOR WINDOWS ON WINDOWS **********"

Push-Location $buildPath
cmake -DCMAKE_BUILD_TYPE=Release "$basePath"
Pop-Location
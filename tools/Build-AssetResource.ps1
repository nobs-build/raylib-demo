param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('image', 'audio')]
    [string]$Mode,

    [Parameter(Mandatory = $true, Position = 1)]
    [string]$InputPath,

    [Parameter(Mandatory = $true, Position = 2)]
    [string]$GeneratedPath,

    [Parameter(Mandatory = $true, Position = 3)]
    [string]$ObjectPath
)

$ErrorActionPreference = 'Stop'

& (Join-Path $PSScriptRoot 'Optimize-Asset.ps1') `
    -Mode $Mode `
    -InputPath $InputPath `
    -OutputPath $GeneratedPath

$objcopy = 'C:\raylib\w64devkit\bin\objcopy.exe'
$generated = [System.IO.Path]::GetFullPath($GeneratedPath)
$object = [System.IO.Path]::GetFullPath($ObjectPath)

Push-Location (Split-Path -Parent $generated)
try {
    & $objcopy `
        -I binary `
        -O pe-x86-64 `
        -B i386:x86-64 `
        (Split-Path -Leaf $generated) `
        (Split-Path -Leaf $object)

    if ($LASTEXITCODE -ne 0) {
        throw "objcopy failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}

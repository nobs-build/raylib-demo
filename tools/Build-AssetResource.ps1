param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$InputPath,

    [Parameter(Mandatory = $true, Position = 1)]
    [string]$ObjectPath
)

$ErrorActionPreference = 'Stop'

$input = (Resolve-Path -LiteralPath $InputPath).Path
$object = [System.IO.Path]::GetFullPath($ObjectPath)
$generated = Join-Path (Split-Path -Parent $object) (Split-Path -Leaf $input)

switch ([System.IO.Path]::GetExtension($input).ToLowerInvariant()) {
    '.png' { $mode = 'image' }
    '.ogg' { $mode = 'audio' }
    default { throw "Unsupported asset extension: $([System.IO.Path]::GetExtension($input))" }
}

& (Join-Path $PSScriptRoot 'Optimize-Asset.ps1') `
    -Mode $mode `
    -InputPath $input `
    -OutputPath $generated

$objcopy = 'C:\raylib\w64devkit\bin\objcopy.exe'

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

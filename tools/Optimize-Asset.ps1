param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('image', 'audio')]
    [string]$Mode,

    [Parameter(Mandatory = $true)]
    [string]$InputPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

function Find-Ffmpeg {
    if ($env:FFMPEG) {
        if (Test-Path -LiteralPath $env:FFMPEG -PathType Leaf) {
            return (Resolve-Path -LiteralPath $env:FFMPEG).Path
        }

        throw "FFMPEG points to a missing file: $env:FFMPEG"
    }

    $command = Get-Command ffmpeg -CommandType Application -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $wingetRoot = Join-Path $env:LOCALAPPDATA 'Microsoft\WinGet\Packages'
    if (Test-Path -LiteralPath $wingetRoot) {
        $command = Get-ChildItem -LiteralPath $wingetRoot -Directory -Filter 'Gyan.FFmpeg_*' |
            Get-ChildItem -Filter ffmpeg.exe -File -Recurse |
            Sort-Object FullName -Descending |
            Select-Object -First 1

        if ($command) {
            return $command.FullName
        }
    }

    throw 'FFmpeg was not found. Install it with: winget install Gyan.FFmpeg, or set FFMPEG to ffmpeg.exe.'
}

$ffmpeg = Find-Ffmpeg
$input = (Resolve-Path -LiteralPath $InputPath).Path
$output = [System.IO.Path]::GetFullPath($OutputPath)
$outputDirectory = Split-Path -Parent $output
[System.IO.Directory]::CreateDirectory($outputDirectory) | Out-Null

$commonArgs = @('-hide_banner', '-loglevel', 'error', '-y', '-i', $input)

if ($Mode -eq 'image') {
    $filter = "[0:v]scale='min(960,iw)':-2:flags=lanczos,split[a][b];[a]palettegen=max_colors=256:reserve_transparent=0[p];[b][p]paletteuse=dither=sierra2_4a"
    $encodeArgs = @('-filter_complex', $filter, '-frames:v', '1', '-compression_level', '9', $output)
}
else {
    $encodeArgs = @('-map_metadata', '-1', '-vn', '-c:a', 'libvorbis', '-b:a', '64k', $output)
}

& $ffmpeg @commonArgs @encodeArgs
if ($LASTEXITCODE -ne 0) {
    throw "FFmpeg failed with exit code $LASTEXITCODE"
}

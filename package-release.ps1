param(
    [switch]$Force
)

$ErrorActionPreference = "Stop"

$root         = $PSScriptRoot
$buildDir     = Join-Path $root "build\msvc2022"
$distDir      = Join-Path $root "dist"
$releasesFile = Join-Path $root "releases.json"
$staging      = Join-Path $root "_staging"

$appName      = "password"
$targetName   = "password"
$exeName      = "$targetName.exe"
$zipPrefix    = "cybershow-password"

function Test-GitRepo {
    git -C $root rev-parse --is-inside-work-tree 2>$null | Out-Null
    return ($LASTEXITCODE -eq 0)
}

$hasGit = Test-GitRepo
if ($hasGit) {
    $commitShort = git -C $root rev-parse --short HEAD
    $commitMsg   = git -C $root log -1 --pretty=%s
} else {
    $commitShort = "nogit"
    $commitMsg   = "Unversioned password package"
}

if (-not (Test-Path $releasesFile)) {
    [PSCustomObject]@{ releases = @() } | ConvertTo-Json -Depth 5 | Set-Content $releasesFile -Encoding UTF8
}

$data     = Get-Content $releasesFile | ConvertFrom-Json
$releases = @($data.releases)
$last     = if ($releases.Count -gt 0) { $releases[-1] } else { $null }

if (-not $Force -and $last -and $last.commit -eq $commitShort) {
    Write-Host "Already packaged as $($last.zip). Nothing to do."
    Write-Host "Use -Force to repackage the same commit."
    exit 0
}

$versionNum = if ($releases.Count -eq 0) { 0 } else { [int]$last.version + 1 }
$versionTag = "v{0:D2}" -f $versionNum
$zipName    = "$zipPrefix-$versionTag.zip"
$zipPath    = Join-Path $distDir $zipName

Write-Host ""
Write-Host "=== Packaging $versionTag ==="
Write-Host "  Commit : $commitShort - $commitMsg"
Write-Host "  Output : $zipName"
Write-Host ""

Write-Host ">> Configuring..."
cmake -S $root -B $buildDir -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null

Write-Host ">> Building Release..."
cmake --build $buildDir --config Release
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed."; exit 1 }

if (Test-Path $staging) { Remove-Item $staging -Recurse -Force }
New-Item -ItemType Directory "$staging\plugins\platforms" | Out-Null

$out = Join-Path $buildDir "Release"
if (-not (Test-Path (Join-Path $out $exeName))) {
    $out = $buildDir
}

Copy-Item (Join-Path $out $exeName)                         $staging
Copy-Item (Join-Path $out "Qt6Core.dll")                    $staging
Copy-Item (Join-Path $out "Qt6Gui.dll")                     $staging
Copy-Item (Join-Path $out "Qt6Widgets.dll")                 $staging
Copy-Item (Join-Path $out "plugins\platforms\qwindows.dll") "$staging\plugins\platforms\"
Copy-Item (Join-Path $root "README.md")                     $staging
Copy-Item (Join-Path $root "RUNBOOK.md")                    $staging

Write-Host ">> Creating zip..."
if (-not (Test-Path $distDir)) { New-Item -ItemType Directory $distDir | Out-Null }
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path "$staging\*" -DestinationPath $zipPath
Remove-Item $staging -Recurse -Force

$entry = [PSCustomObject]@{
    version = $versionNum
    commit  = $commitShort
    date    = (Get-Date -Format "yyyy-MM-ddTHH:mm:ss")
    message = $commitMsg
    zip     = $zipName
}
$releases += $entry
$data.releases = $releases
$data | ConvertTo-Json -Depth 5 | Set-Content $releasesFile -Encoding UTF8

if ($hasGit) {
    Write-Host ">> Tagging commit as $versionTag..."
    git -C $root tag $versionTag 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "   Note: tag $versionTag already exists, skipped."
    }
} else {
    Write-Host ">> Not in a git repository; tag creation skipped."
}

$sizeMB = [math]::Round((Get-Item $zipPath).Length / 1MB, 2)
Write-Host ""
Write-Host "=== Done ==="
Write-Host "  App     : $appName"
Write-Host "  Version : $versionTag"
Write-Host "  Commit  : $commitShort - $commitMsg"
Write-Host "  Zip     : $zipName ($sizeMB MB)"
Write-Host "  Path    : $zipPath"
Write-Host ""


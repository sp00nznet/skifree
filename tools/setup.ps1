# One-click setup for SkiFree: Reborn on Windows. Double-click Setup.cmd.
#
#   1. check the tools (Python 3, CMake, Visual Studio's C++ tools; winget
#      installs what's missing) and fetch SDL2's prebuilt libraries into deps\
#   2. pull the sprites (and sounds, from ski32sounds.exe) out of your ski32.exe
#   3. build build\Release\skifree.exe
#   4. leave a shortcut here: SkiFree (recomp).cmd
#
# -Yes installs what is missing without asking; -Game (or -GameDir) is
# ski32.exe, a zip of it, or the folder it is in. RECOMP_LAUNCHER=1 (the
# recomp launcher) skips the "Press Enter". Everything goes to setup.log.
# -Targets pc,psp,wii: pc is this computer (the default); the rest are
# scripts\targets.tsv's, built by tools\setup-consoles.ps1 in Docker, which
# downloads only their toolchains.
# ski32.exe is free from its author: https://ski.ihoc.net/
param([switch]$Yes, [Alias('GameDir')][string]$Game = "", [string]$Targets = "pc")

$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Log = Join-Path $Root 'setup.log'
Set-Location $Root
$Steps = 4
function Log($t) { try { Add-Content -Path $Log -Value $t -Encoding UTF8 -ErrorAction Stop } catch {} }
Log "==== setup $(Get-Date -Format s)"
function Say($t, $c = 'Gray') { Write-Host $t -ForegroundColor $c; Log $t }
function Pause-End { if (-not $env:RECOMP_LAUNCHER) { Read-Host "Press Enter to close" | Out-Null } }
function Step($n, $t) { Write-Host ""; Say "[$n/$Steps] $t" 'Cyan' }
function Fail($t) {
  Say ""; Say "Setup stopped: $t" 'Red'
  Say "The details are in $Log. Fix that and run Setup.cmd again." 'Yellow'
  Pause-End; exit 1
}
$picked = @($Targets -split ',' | ForEach-Object { $_.Trim() } | Where-Object { $_ })
$others = @($picked | Where-Object { $_ -ne 'pc' })
function Consoles {
  & (Join-Path $PSScriptRoot 'setup-consoles.ps1') -Yes:$Yes -Game $Game -Targets ($others -join ',')
  $code = if ($?) { 0 } else { 1 }   # its exit 1 on 'Setup stopped'; a stale docker code otherwise
  Pause-End; exit $code
}
if ($others.Count -and $picked -notcontains 'pc') { Consoles }
function Ask($q) { if ($Yes) { Say "$q yes"; return $true }; $a = Read-Host "$q [Y/n]"; return -not ($a -match '^[nN]') }
function Refresh-Path { $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User') }
# Run a program with its output in the log. Windows PowerShell turns a native
# program's stderr into errors, so 'Stop' is off while it runs.
function Exec([string[]]$cmd) {
  $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
  $rest = @($cmd | Select-Object -Skip 1)
  & $cmd[0] @rest 2>&1 | ForEach-Object { Log "$_" }
  $code = $LASTEXITCODE
  $ErrorActionPreference = $old
  return $code
}
function Run($what, [string[]]$cmd) { Say "  $what..."; $code = Exec $cmd; if ($code -ne 0) { Fail "$what failed (exit code $code)." } }
function Winget($id, $what) {
  if (-not (Ask "  Install $what now (winget)?")) { Fail "$what is required." }
  if (-not (Get-Command winget -ErrorAction SilentlyContinue)) { Fail "winget is missing. Install 'App Installer' from the Microsoft Store, or install $what yourself." }
  Exec (@('winget', 'install', '-e', '--id', $id, '--accept-package-agreements', '--accept-source-agreements') + $args) | Out-Null
  Refresh-Path
}

Say "SkiFree: Reborn - setup" 'White'

# ---------------------------------------------------------------- tools
Step 1 "Checking the tools"
# A Python that answers "Python 3.x" (the Store's placeholder answers nothing).
function Find-Python {
  foreach ($c in @(@('py', '-3'), @('python'), @('python3'))) {
    if (-not (Get-Command $c[0] -ErrorAction SilentlyContinue)) { continue }
    $old = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    $v = (& $c[0] @($c | Select-Object -Skip 1) --version 2>&1 | Out-String).Trim()
    $ErrorActionPreference = $old
    if ($v -match '^Python 3\.') { return ,$c }
  }
  return $null
}
$py = Find-Python
if (-not $py) {
  Say "  Python 3 is not installed."
  Winget 'Python.Python.3.12' 'Python 3.12 (for your user only, about 30 MB)' '--scope' 'user'
  $py = Find-Python
  if (-not $py) { Fail "Python installed, but Windows has not picked it up yet: close this window and run Setup.cmd again." }
}
Say "  Python: $($py -join ' ')"
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Say "  CMake is not installed."
  Winget 'Kitware.CMake' 'CMake (about 30 MB)'
  if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { Fail "CMake installed, but Windows has not picked it up yet: close this window and run Setup.cmd again." }
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
function Find-VS { if (Test-Path $vswhere) { & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath } }
if (-not (Find-VS)) {
  Say "  Visual Studio's C++ tools are not installed."
  Winget 'Microsoft.VisualStudio.2022.BuildTools' 'the Visual Studio 2022 Build Tools with C++ (about 3 GB, asks for admin)' '--override' '--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
  if (-not (Find-VS)) { Fail "the Visual Studio Build Tools did not install. Install them with 'Desktop development with C++', then run Setup.cmd again." }
}
Say "  Visual Studio: $(Find-VS)"
# SDL2 and SDL2_ttf, the official prebuilt Visual C++ packages
$Deps = Join-Path $Root 'deps'
$Sdl = @(
  @('SDL2-2.32.10', 'https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-devel-2.32.10-VC.zip'),
  @('SDL2_ttf-2.24.0', 'https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.24.0/SDL2_ttf-devel-2.24.0-VC.zip'))
New-Item -ItemType Directory -Force $Deps | Out-Null
foreach ($d in $Sdl) {
  if (Test-Path (Join-Path $Deps $d[0])) { continue }
  Say "  Downloading $($d[0])..."
  $zip = Join-Path $Deps "$($d[0]).zip"
  try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    (New-Object Net.WebClient).DownloadFile($d[1], $zip)
    Expand-Archive -Force $zip $Deps
    Remove-Item $zip
  } catch { Fail "could not download $($d[1]): $_" }
}

# ---------------------------------------------------------------- the game
Step 2 "Taking the sprites and sounds out of your ski32.exe"
# ski32sounds.exe (Alice Averlong's, with the sounds) over ski32.exe; a zip of either.
function Pick($dir) {
  $f = @(Get-ChildItem -Path $dir -File -ErrorAction SilentlyContinue | Where-Object { $_.Name -match '^ski32.*\.(exe|zip)$' } | Sort-Object { $_.Name -notmatch 'sounds' }, Name)
  if ($f.Count) { return $f[0].FullName } else { return $null }
}
$Game = $Game.Trim('"', ' ')
$src = if ($Game -and (Test-Path $Game -PathType Leaf)) { $Game }
       elseif ($Game) { Pick $Game }
       else { $p = Pick $Root; if (-not $p) { $p = Pick (Join-Path $env:USERPROFILE 'Downloads') }; $p }
if (-not $src) {
  if (Test-Path 'resources\ski32_1.bmp') { Say "  None found, but resources\ already has the sprites: keeping them." }
  else { Fail "ski32.exe was not found. Get it free from https://ski.ihoc.net/ and run Setup.cmd again with it in this folder or in Downloads." }
} else {
  Say "  $src"
  $exe = $src
  if ($src -match '\.zip$') {
    $tmp = Join-Path $env:TEMP "skifree-setup"
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
    Expand-Archive -Force $src $tmp
    $exe = @(Get-ChildItem $tmp -Recurse -Filter *.exe | Sort-Object { $_.Name -notmatch 'sounds' })[0].FullName
    if (-not $exe) { Fail "$src has no .exe in it." }
  }
  Run "Extracting" ($py + @((Join-Path $Root 'tools\extract_resources.py'), $exe))
}

# ---------------------------------------------------------------- build
Step 3 "Building build\Release\skifree.exe (a few minutes)"
$prefix = ($Sdl | ForEach-Object { (Join-Path $Deps $_[0]) -replace '\\', '/' }) -join ';'
Run "Configuring" @('cmake', '-S', $Root, '-B', (Join-Path $Root 'build'), '-A', 'x64', "-DCMAKE_PREFIX_PATH=$prefix")
Run "Compiling" @('cmake', '--build', (Join-Path $Root 'build'), '--config', 'Release')
if (-not (Test-Path 'build\Release\skifree.exe')) { Fail "the build finished but build\Release\skifree.exe is missing." }

# ---------------------------------------------------------------- shortcut
Step 4 "Making the shortcut"
$lnk = Join-Path $Root 'SkiFree (recomp).cmd'
Set-Content -Path $lnk -Encoding ASCII -Value @('@echo off', 'cd /d "%~dp0"', 'start "" build\Release\skifree.exe %*')
Say "  $lnk"
Write-Host ""
Say "Done. Double-click 'SkiFree (recomp).cmd' to play. F to go faster; the yeti is waiting." 'Green'
if ($others.Count) { Write-Host ""; Consoles }
Pause-End

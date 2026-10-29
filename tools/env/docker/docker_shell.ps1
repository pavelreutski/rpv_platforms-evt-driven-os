# Run the RPV environment image of a target on Windows, building it first if
# needed. Same interface as docker_shell.sh (Linux/WSL). The repository is
# mounted at /work.
#
#   tools\env\docker\docker_shell.ps1 x86_64                   # interactive shell
#   tools\env\docker\docker_shell.ps1 x86_64 make x86_64-iso   # single command
#   tools\env\docker\docker_shell.ps1 --rebuild x86_64         # rebuild the image first
#
# If scripts are blocked by the execution policy, use docker_shell.cmd instead.

# No $ErrorActionPreference = 'Stop': Windows PowerShell 5.1 would stop on the
# stderr of 'docker image inspect' below. Exit codes are checked instead.

function Show-Usage {
    Write-Host 'Usage: docker_shell.ps1 [--rebuild] <x86_64|microblaze|avr|all> [command...]'
    Write-Host ''
    Write-Host '  --rebuild   rebuild the image even if it exists'
}

$Rest = @($args)
$Rebuild = $false

if ($Rest.Count -gt 0 -and $Rest[0] -in @('--rebuild', '-Rebuild')) {
    $Rebuild = $true
    $Rest = @($Rest | Select-Object -Skip 1)
}

if ($Rest.Count -gt 0 -and $Rest[0] -in @('-h', '--help')) {
    Show-Usage
    exit 0
}

if ($Rest.Count -eq 0 -or $Rest[0] -notin @('x86_64', 'microblaze', 'avr', 'all')) {
    Show-Usage
    exit 1
}

$Target = $Rest[0]
$Command = @($Rest | Select-Object -Skip 1)

$EnvDir = Split-Path -Parent $PSScriptRoot
$Repo = (Resolve-Path -ErrorAction Stop (Join-Path $EnvDir '..\..')).Path
$Image = "rpv-env:$Target"

docker image inspect $Image *> $null
if ($Rebuild -or $LASTEXITCODE -ne 0) {
    docker build -f (Join-Path $EnvDir 'docker\Dockerfile') --target $Target -t $Image $EnvDir
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$RunArgs = @('run', '--rm', '-i', '-v', "${Repo}:/work", '-w', '/work', '-e', 'TERM=xterm-256color')

if (-not [Console]::IsInputRedirected -and -not [Console]::IsOutputRedirected) {
    $RunArgs += '-t'
}

# Mark the mounted repository as safe for Git inside the container.
$RunArgs += @(
    '-e', 'GIT_CONFIG_COUNT=1',
    '-e', 'GIT_CONFIG_KEY_0=safe.directory',
    '-e', 'GIT_CONFIG_VALUE_0=/work'
)

# The Windows ssh-agent can't be forwarded. The container starts its own;
# the keys are mounted read-only for 'ssh-add', they never enter the image.
$SshDir = Join-Path $env:USERPROFILE '.ssh'
if (Test-Path $SshDir) {
    $RunArgs += @('-v', "${SshDir}:/mnt/ssh:ro")
    Write-Host 'note: %USERPROFILE%\.ssh is mounted at /mnt/ssh.'
    Write-Host '      All keys will be installed at container startup only. When additional ones required while container is already running, load a key with:'
    Write-Host '      install -Dm600 /mnt/ssh/<key> ~/.ssh/<key> && ssh-add ~/.ssh/<key>'
}

$RunArgs += $Image

# Run repository-managed Linux startup logic.
$RunArgs += @('/bin/bash', '/work/tools/env/docker/docker_start.sh')
$RunArgs += $Command

& docker @RunArgs
exit $LASTEXITCODE

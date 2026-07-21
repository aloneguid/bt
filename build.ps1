param(
    [bool]$Build = $true,
    [bool]$Pack = $false
)

$MD_VERSION = (Get-Content -Path .\docs\release-notes.md -First 1).Trim('#', ' ')

Write-Host "version $MD_VERSION"

$env:VERSION=$MD_VERSION

Write-Host "configuring..."
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release --preset x64-debug

if ($Build)
{
    Write-Host "building..."
    cmake --build build --config Release
}

if ($Pack)
{
    Write-Host "packaging..."
    cpack --config build/CPackConfig.cmake -C Release
}

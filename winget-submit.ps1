$version = (Get-Content -Path .\docs\release-notes.md -First 1).Trim('#', ' ')

Write-Host "submiting version $($version)..."

wingetcreate update `
    --urls https://github.com/aloneguid/bt/releases/download/$($version)/bt-$($version).zip `
    --version $($version) `
    --submit `
    aloneguid.bt
$version = (Get-Content -Path .\docs\release-notes.md -First 1).Trim('#', ' ')

Write-Host "using version $version"

$v1 = $version -replace "\.", ","
$v1 += ",0"
$v2 = $version + ".0"

# update version in bt.rc
(Get-Content .\bt\bt.rc) `
    -replace "4,\d+,\d+,\d+", "$v1" |
    Out-File .\bt\bt.rc
(Get-Content .\bt\bt.rc) `
    -replace "4\.\d+\.\d+\.\d+", "$v2" |
    Out-File .\bt\bt.rc
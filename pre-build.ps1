$version = (Get-Content -Path .\docs\release-notes.md -First 1).Trim('#', ' ')

Write-Host "using version $version"

$v1 = ($version -replace "\.", ",") + ",0"
$v2 = $version + ".0"

Write-Host "Replacement values: vCommaed='$v1', vDotted='$v2'"

$Subfolder = "bt"
$rcFiles = Get-ChildItem -Path $Subfolder -Filter "*.rc" -File -Recurse
Write-Host "Found $($rcFiles.Count) .rc file(s)"

if ($rcFiles.Count -eq 0) {
    Write-Host "No .rc files found under '$Subfolder'"
}

foreach ($rcFile in $rcFiles) {
    Write-Host "Processing: $($rcFile.FullName)"

    (Get-Content $rcFile.FullName) `
        -replace "\d,\s*\d+,\s*\d+,\s*\d+", "$v1" |
        Out-File $rcFile.FullName

    (Get-Content $rcFile.FullName) `
        -replace "\d\.\d+\.\d+\.\d+", "$v2" |
        Out-File $rcFile.FullName

    Write-Host "Updated: $($rcFile.FullName)"
}

Write-Host "Completed pre-build version update"

# update version in globals.h, which is set as:
# "#define APP_VERSION "0.0.0"

$globalsPath = Join-Path $Subfolder "globals.h"
if (Test-Path -Path $globalsPath) {
    Write-Host "Updating version in: $globalsPath"
    
    (Get-Content $globalsPath) `
        -replace "#define APP_VERSION "".*""", "#define APP_VERSION `"$version`"" |
        Out-File $globalsPath

    Write-Host "Updated: $globalsPath"
}
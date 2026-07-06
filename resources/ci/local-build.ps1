$MD_VERSION = (Get-Content -Path .\docs\release-notes.md -First 1).Trim('#', ' ')

Write-Host "version $MD_VERSION"

$env:VERSION=$MD_VERSION

cmake -B build -S . -D "CMAKE_BUILD_TYPE=Release" `
    -D "CMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -D "VCPKG_TARGET_TRIPLET=x64-windows-static"

cmake --build build --config Release
param([switch]$Blog = $false)


$srcLog = ".\log.json"
$log = Get-Content $srcLog | ConvertFrom-Json

$version = $log[0].version
$v1 = $version -replace "\.", ","
$v1 += ",0"
$v2 = $version + ".0"
$cppLog = ".\bt\log.h"

Write-Host "building version $version"

# update version in globals.h
(Get-Content .\bt\globals.h) `
    -replace "Version\s*=\s*""(3.*)""", "Version = ""$version""" |
    Out-File .\bt\globals.h

$txtLog = ""
$cppStructs = ""

foreach($lv in $log) {
    $txtLog += "$($lv.version) ($($lv.date))`n"
    $cppStructs += "    change_version {""$($lv.version)"", ""$($lv.date)"",`n";

    $c = $lv.changes

    $cppStructs += "        {";
    if($c.new) {
        $txtLog += "New Features:`n"
        foreach($item in $c.new) {
            $ea = $item -replace """", "\"""
            $txtLog += "! $item`n"
            $cppStructs += "`n            ""$ea"","
        }
        $cppStructs += "`n        "
    }
    $cppStructs += "},"

    $cppStructs += "`n        {";
    if($c.improvements) {
        $txtLog += "Improvements:`n"
        foreach($item in $c.improvements) {
            $ea = $item -replace """", "\"""
            $txtLog += "+ $item`n"
            $cppStructs += "`n            ""$ea"","
        }
        $cppStructs += "`n        "
    }
    $cppStructs += "},"

    $cppStructs += "`n        {"
    if($c.bugs) {
        $txtLog += "Bugs Fixed:`n"
        foreach($item in $c.bugs) {
            $ea = $item -replace """", "\"""
            $txtLog += "- $item`n"
            $cppStructs += "`n            ""$ea"","
        }
        $cppStructs += "`n        "
    }
    $cppStructs += "}"

    $txtLog += "`n"
    $cppStructs += "},`n"
}

@"
#pragma once
#include <string>
#include <vector>

struct change_version {
    std::string number;
    std::string date;
    std::vector<std::string> news;
    std::vector<std::string> improvements;
    std::vector<std::string> bugs;
};

const std::vector<change_version> ChangeLog {
@cvs
};

"@ -replace "@cvs", $cppStructs | Out-File $cppLog


# update version in bt.rc
(Get-Content .\bt\bt.rc) `
    -replace "3,\d+,\d+,\d+", "$v1" |
    Out-File .\bt\bt.rc
(Get-Content .\bt\bt.rc) `
    -replace "3\.\d+\.\d+\.\d+", "$v2" |
    Out-File .\bt\bt.rc

# update version in cmake
(Get-Content .\CMakeLists.txt) `
    -replace "VERSION\s*(\d.\d.\d)", "VERSION $version" |
    Out-File .\CMakeLists.txt

# build executable
$buildType = "Release"
$triplet = "x64-windows-static" 
$cconfig = "Release"
iex "cmake -B build -S . -D CMAKE_BUILD_TYPE=$buildType -D CMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=$triplet"
iex "cmake --build build --config $cconfig"
cd build
cpack -C $buildType
cd ..

if($Blog) {
    # set content in latest.txt
    $version | Set-Content "$($blogBin)latest.txt"

    # zip executable
    Compress-Archive -Path .\build\bt\release\bt.exe `
        -DestinationPath "$($blogBin)bt-$($version).zip" `
        -CompressionLevel Optimal -Force

    # copy installer
    cp ".\build\bt.msi" "$($blogBin)bt-$($version).msi"

    # copy links
    (Get-Content $blogIndexMd) -replace "\[installer\]\(.+?\)", "[installer](bin/bt-$($version).msi)" | Out-File $blogIndexMd
    (Get-Content $blogIndexMd) -replace "\[portable\]\(.+?\)", "[portable](bin/bt-$($version).zip)" | Out-File $blogIndexMd
}

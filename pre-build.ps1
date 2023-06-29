$srcLog = ".\log.json"
$log = Get-Content $srcLog | ConvertFrom-Json

$version = $log[0].version
$v1 = $version -replace "\.", ","
$v1 += ",0"
$v2 = $version + ".0"
$cppLog = ".\bt\log.h"

$txtLog = ""
$cppStructs = ""
$mdRel = ""

# release notes for github
$md0 = $log[0].changes
if($md0.new) {
    $mdRel += "## New Features`n"
    foreach($item in $md0.new) {
        $mdRel += "- $item`n"
    }
    $mdResl += "`n"
}

if($md0.improvements) {
    $mdRel += "## Improvements`n"
    foreach($item in $md0.improvements) {
        $mdRel += "- $item`n"
    }
    $mdResl += "`n"
}

if($md0.bugs) {
    $mdRel += "## Bugs Fixed`n"
    foreach($item in $md0.bugs) {
        $mdRel += "- $item`n"
    }
    $mdResl += "`n"
}

Write-Host $mdRel
$mdRel | Out-File release-notes.md

# text release notes and c++ header file
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
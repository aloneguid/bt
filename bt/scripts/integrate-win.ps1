# Use this to register BT in the system, for development only.

param(
    [Parameter(Mandatory)]
    [ValidateScript({ Test-Path $_ -PathType Leaf })]
    [string]$Path,

    [switch]$InstallAll,
    [switch]$UninstallAll,
    [switch]$InstallCustomProtocol,
    [switch]$UninstallCustomProtocol
)
$env = "Dev"
$name = "Browser Tamer"
$slug = "BrowserTamer$env"
$customProtocol = "x-bt"
$resolvedPath = (Resolve-Path $Path).Path
$htmClass = "BrowserTamerHTM$env"
$pdfClass = "BrowserTamerPDF$env"
$epubClass = "BrowserTamerEPUB$env"
$appDescription = "Redirects open URLs to a browser of your choice."

if($env -ne "") {
    $name = "Browser Tamer $env"
}

Write-Host "slug: $slug, exe path: $resolvedPath"

# --- custom protocol (X-BT)

$protocolKey = "HKCU:\Software\Classes\$customProtocol"

if ($InstallAll -or $InstallCustomProtocol) {
    Write-Host "Registering custom protocol: $customProtocol"
    $commandKey  = "$protocolKey\shell\open\command"
    New-Item -Path $protocolKey -Value "URL:$customProtocol Protocol" -Force | Out-Null
    Set-ItemProperty -Path $protocolKey -Name "URL Protocol" -Value "" -Force | Out-Null
    New-Item -Path $commandKey -Value "`"$resolvedPath`" `"%1`"" -Force | Out-Null
}

if($UninstallAll -or $UninstallCustomProtocol) {
    Write-Host "Unregistering custom protocol: $customProtocol"
    Remove-Item -Path $protocolKey -Recurse -Force
}

# ---

$smiRoot = "HKCU:\Software\Clients\StartMenuInternet\$slug"
$classes = @(
    $htmClass,
    $pdfClass,
    $epubClass
)

if($InstallAll) {
    Write-Host "Registering as internet client: $name"
    New-Item -Path $smiRoot -Value $name -Force | Out-Null
    New-Item -Path "$smiRoot\DefaultIcon" -Value "$resolvedPath,0" -Force | Out-Null
    New-item -Path "$smiRoot\shell\open\command" -Value $resolvedPath -Force | Out-Null

    $capsKey = "$smiRoot\Capabilities"
    New-Item -Path $capsKey -Force | Out-Null
    Set-ItemProperty -Path $capsKey -Name "ApplicationName" -Value $name -Force | Out-Null
    Set-ItemProperty -Path $capsKey -Name "ApplicationDescription" -Value $appDescription -Force | Out-Null
    Set-ItemProperty -Path $capsKey -Name "ApplicationIcon" -Value "$resolvedPath,0" -Force | Out-Null

    $urlAssocs = @(
        "http",
        "https",
        "x-bt"
    )

    $fileAssocs = @(
        ".htm",
        ".html",
        ".mht",
        ".mhtml",
        ".shtml",
        ".svg",
        ".webp",
        ".xht",
        ".xhtml",
        ".pdf",
        ".epub"
    )

   $urlAssocKey = "$capsKey\URLAssociations"
   New-Item -Path $urlAssocKey -Force | Out-Null
   foreach ($urlAssoc in $urlAssocs) {
       Write-Host "registering url handler for $urlAssoc"
       Set-ItemProperty -Path $urlAssocKey -Name $urlAssoc -Value $htmClass -Force | Out-Null
   }

   $fileAssocKey = "$capsKey\FileAssociations"
   New-Item -Path $fileAssocKey -Force | Out-Null
   foreach ($fileAssoc in $fileAssocs) {
       Write-Host "registered file handler for $fileAssoc"
       $class = $htmClass
       if($fileAssoc -eq "pdf") {
           $class = $pdfClass
       }
       elseif($fileAssoc -eq "epub") {
           $class = $epubClass
       }
       Set-ItemProperty -Path $fileAssocKey -Name $fileAssoc -Value $class -Force | Out-Null
   }

   # classes
   foreach ($class in $classes) {
       $classKey = "HKCU:\Software\Classes\$class"

       Write-Host "Registering class: $class"

       New-Item -Path $classKey -Value "$name Document" -Force | Out-Null
       $appKey = "$classKey\Application"
       New-Item -Path $appKey -Force | Out-Null
       Set-ItemProperty -Path $appKey -Name "ApplicationName" -Value $name -Force | Out-Null
       Set-ItemProperty -Path $appKey -Name "ApplicationDescription" -Value $appDescription -Force | Out-Null

       New-Item -Path "$classKey\DefaultIcon" -Value "$resolvedPath,0" -Force | Out-Null
       New-Item -Path "$classKey\shell\open\command" -Value "`"$resolvedPath`" `"%1`"" -Force | Out-Null
   }

   # registered applications
   Write-Host "Registering application: $name"
   Set-ItemProperty -Path "HKCU:\Software\RegisteredApplications" -Name $name -Value "Software\Clients\StartMenuInternet\$slug\Capabilities" -Force | Out-Null
}

if($UninstallAll) {
    Write-Host "Unregistering internet client: $name from $smiRoot"
    Remove-Item -Path $smiRoot -Recurse -Force

    Write-Host "Unregistering classes: $classes"
    foreach ($class in $classes) {
        $classKey = "HKCU:\Software\Classes\$class"
        Remove-Item -Path $classKey -Recurse -Force
    }

    Write-Host "Unregistering application: $name"
    Remove-ItemProperty -Path "HKCU:\Software\RegisteredApplications" -Name $name -Force | Out-Null
}

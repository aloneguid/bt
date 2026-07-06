$processPath = "build/bt/Release/bt.exe"

Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms

New-Item -ItemType Directory -Path screenshots -Force | Out-Null

function Save-Screenshot([string]$Path) {
    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
}

$pick = Start-Process -FilePath $processPath -ArgumentList "discover" -PassThru
Start-Sleep -Seconds 3

$bt = Start-Process -FilePath $processPath -PassThru
Start-Sleep -Seconds 3
Save-Screenshot "$PWD\screenshots\config.png"
if ($bt -and -not $bt.HasExited) { Stop-Process -Id $bt.Id -Force }

$pick = Start-Process -FilePath $processPath -ArgumentList "pick", "https://bbc.co.uk" -PassThru
Start-Sleep -Seconds 3
Save-Screenshot "$PWD\screenshots\picker.png"

if ($pick -and -not $pick.HasExited) { Stop-Process -Id $pick.Id -Force }
if ($bt -and -not $bt.HasExited) { Stop-Process -Id $bt.Id -Force }

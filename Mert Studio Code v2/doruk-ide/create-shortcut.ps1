$exePath = "D:\Mevcut Projeler\Mert Studio Code Nihai Hali\doruk-ide\dist\Mert Studio Code-win32-x64\Mert Studio Code.exe"
$desktop  = [Environment]::GetFolderPath('Desktop')
$lnk      = Join-Path $desktop "Mert Studio Code.lnk"

$wsh = New-Object -ComObject WScript.Shell
$sc  = $wsh.CreateShortcut($lnk)
$sc.TargetPath       = $exePath
$sc.WorkingDirectory = Split-Path $exePath
$sc.Description      = "Mert Studio Code - DORUK Programlama Dili IDE"
$sc.Save()

Write-Host "Masaustu kisayolu olusturuldu: $lnk"

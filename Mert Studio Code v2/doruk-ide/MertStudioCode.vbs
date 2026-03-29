Option Explicit
' Mert Studio Code Launcher
' - Paketlenmiş .exe varsa onu açar (terminal yok)
' - Yoksa dev modda npm start çalıştırır (terminal gizli)

Dim WshShell, ideDir, exePath, cmd
Set WshShell = CreateObject("WScript.Shell")

' Mevcut klasörü belirle
ideDir = Left(WScript.ScriptFullName, InStrRev(WScript.ScriptFullName, "\"))

' Paketlenmiş exe yolunu kontrol et
exePath = ideDir & "dist\Mert Studio Code-win32-x64\Mert Studio Code.exe"

Dim fso
Set fso = CreateObject("Scripting.FileSystemObject")

If fso.FileExists(exePath) Then
    ' Paketlenmiş .exe'yi çalıştır (terminal olmadan)
    WshShell.Run """" & exePath & """", 1, False
Else
    ' Geliştirme modu: npm start (gizli terminal)
    cmd = "cmd /c cd /d """ & ideDir & """ && npm start"
    WshShell.Run cmd, 0, False
End If

Set fso = Nothing
Set WshShell = Nothing

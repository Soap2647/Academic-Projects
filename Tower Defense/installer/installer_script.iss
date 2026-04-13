; Inno Setup Kurulum Betiği — Tower Defense Game
; Versiyon: 1.0
;
; ÖNEMLİ: Bu betik tekil kurulum kilidi mekanizması içerir.
; Kayıt defteri anahtarı bir kez yazıldıktan sonra kaldırma işlemi
; tarafından silinmez — kalıcı kurulum yasağı böylece sağlanır.

[Setup]
AppName=Tower Defense Game
AppVersion=1.0
AppPublisher=TowerDefense Dev
AppPublisherURL=https://example.com
AppSupportURL=https://example.com/support
AppUpdatesURL=https://example.com/updates
DefaultDirName={autopf}\TowerDefenseGame
DefaultGroupName=Tower Defense Game
AllowNoIcons=yes
OutputDir=..\dist
OutputBaseFilename=TowerDefenseSetup_v1.0
SetupIconFile=
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
; Yönetici ayrıcalığı gerektirir (HKLM yazma için)
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Tasks]
Name: "desktopicon"; Description: "Masaüstü kısayolu oluştur"; GroupDescription: "Ek simgeler:"; Flags: unchecked
Name: "quicklaunchicon"; Description: "Hızlı başlatma çubuğu kısayolu"; GroupDescription: "Ek simgeler:"; Flags: unchecked; OnlyBelowVersion: 6.1

[Files]
; Ana çalıştırılabilir dosya
Source: "..\dist\TowerDefense.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Tower Defense Game"; Filename: "{app}\TowerDefense.exe"
Name: "{group}\{cm:UninstallProgram,Tower Defense Game}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Tower Defense Game"; Filename: "{app}\TowerDefense.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\TowerDefense.exe"; Description: "{cm:LaunchProgram,Tower Defense Game}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Kaldırma işleminde kayıt defteri anahtarı SİLİNMEZ — bu kasıtlıdır.
; Tekrar kurulumu engelleyen kalıcı kilit mekanizmasının temelidir.

[Code]
{ ──────────────────────────────────────────────────────────────────────────────
  Pascal Betik Bölümü
  Tekil kurulum kilidi mekanizması burada uygulanır.
  ────────────────────────────────────────────────────────────────────────────── }

const
  REG_PATH  = 'SOFTWARE\TowerDefenseGame';
  REG_KEY   = 'InstallationID';
  REG_VALUE = 'INSTALLED_2025_PERMANENT';

function InitializeSetup(): Boolean;
{ Kurulum başlamadan önce çalışır.
  Kayıt defterinde kilit değeri mevcutsa kurulumu reddeder. }
var
  RegValue: String;
begin
  { HKLM kontrolü }
  if RegQueryStringValue(HKLM, REG_PATH, REG_KEY, RegValue) then
  begin
    if RegValue = REG_VALUE then
    begin
      MsgBox(
        'Bu uygulama daha önce bu bilgisayara kurulmuştur.' + #13#10 +
        'Tekrar kurulum yapılamaz.' + #13#10 + #13#10 +
        'Eğer sorun yaşıyorsanız kurulu sürümü kullanmaya devam edin.',
        mbError,
        MB_OK
      );
      Result := False;
      Exit;
    end;
  end;

  { HKCU kontrolü (kullanıcı düzeyi kurulum geçmişi) }
  if RegQueryStringValue(HKCU, REG_PATH, REG_KEY, RegValue) then
  begin
    if RegValue = REG_VALUE then
    begin
      MsgBox(
        'Bu uygulama daha önce bu kullanıcı hesabına kurulmuştur.' + #13#10 +
        'Tekrar kurulum yapılamaz.',
        mbError,
        MB_OK
      );
      Result := False;
      Exit;
    end;
  end;

  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
{ Kurulum adımları değiştiğinde çağrılır.
  ssPostInstall adımında kayıt defteri kilidi yazılır. }
begin
  if CurStep = ssPostInstall then
  begin
    { HKLM'ye yaz — tüm kullanıcılar için geçerli }
    if not RegWriteStringValue(HKLM, REG_PATH, REG_KEY, REG_VALUE) then
    begin
      { HKLM başarısız — HKCU'ya geri düş }
      RegWriteStringValue(HKCU, REG_PATH, REG_KEY, REG_VALUE);
    end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
{ Kaldırma adımlarında çağrılır.
  NOT: Kayıt defteri anahtarı kasıtlı olarak SİLİNMİYOR.
  Bu, tekrar kurulumu engelleyen kalıcı kilit mekanizmasıdır. }
begin
  { Hiçbir işlem yapılmıyor — kilit korunuyor }
  if CurUninstallStep = usPostUninstall then
  begin
    { İsteğe bağlı: Kaldırma tamamlandı bildirimi }
    { MsgBox('Uygulama kaldırıldı. Tekrar kurulamaz.', mbInformation, MB_OK); }
  end;
end;

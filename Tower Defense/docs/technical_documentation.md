# Tower Defense — Teknik Dokümantasyon

**Versiyon:** 1.0  
**Platform:** Windows 10/11 (Python 3.10+, pygame)  
**Tarih:** 2026-04-13

---

## 1. Mimari Genel Bakış

### 1.1 Sınıf Hiyerarşisi (ASCII Diyagramı)

```
GameObject  (ABC — src/entities/game_object.py)
├── Enemy  (ABC — src/entities/enemies/enemy.py)
│   ├── Goblin   (src/entities/enemies/goblin.py)
│   ├── Troll    (src/entities/enemies/troll.py)
│   └── Boss     (src/entities/enemies/boss.py)
├── Tower  (ABC — src/entities/towers/tower.py)
│   ├── ArrowTower   (src/entities/towers/arrow_tower.py)
│   ├── CannonTower  (src/entities/towers/cannon_tower.py)
│   └── IceTower     (src/entities/towers/ice_tower.py)
└── Projectile  (ABC — src/entities/projectiles/projectile.py)
    ├── Arrow      (src/entities/projectiles/arrow.py)
    ├── Cannonball (src/entities/projectiles/cannonball.py)
    └── IceBlast   (src/entities/projectiles/ice_blast.py)

Destek Sınıfları (kalıtım içermez):
  GameMap   (src/map/game_map.py)
    └── Tile  (src/map/tile.py)
  Player    (src/player/player.py)

Yöneticiler:
  GameManager  (src/managers/game_manager.py)
    ├── WaveManager   (src/managers/wave_manager.py)
    ├── UIManager     (src/managers/ui_manager.py)
    ├── GameMap
    └── Player

Giriş Noktası:
  Game  (src/game.py)  →  GameManager
  main.py              →  Game
```

### 1.2 Katmanlı Mimari

```
┌─────────────────────────────────────────────────┐
│  Sunum Katmanı (Presentation)                   │
│  UIManager · FloatingText · Particle            │
├─────────────────────────────────────────────────┤
│  Oyun Mantığı Katmanı (Game Logic)              │
│  GameManager · WaveManager · GameState enum     │
├─────────────────────────────────────────────────┤
│  Varlık Katmanı (Entity)                        │
│  Enemy/Tower/Projectile alt sınıfları           │
├─────────────────────────────────────────────────┤
│  Veri Katmanı (Data)                            │
│  GameMap · Tile · Player                        │
├─────────────────────────────────────────────────┤
│  Konfigürasyon (Config)                         │
│  settings.py — tüm sabitler                     │
└─────────────────────────────────────────────────┘
```

**Veri Akışı:**

```
main.py → Game.run() → GameManager.update(dt)
                              │
               ┌──────────────┼──────────────┐
               ▼              ▼              ▼
         WaveManager    Enemy.update()  Tower.update()
         .update(dt)    (move + debuff)  → shoot()
               │                              │
               ▼                              ▼
         new enemies              Projectile.update()
         appended to list         → on_hit() → damage
```

---

## 2. OOP Prensipleri Uygulaması

### 2.1 Soyutlama (Abstraction)

**Kullanıldığı yerler:** `GameObject`, `Enemy`, `Tower`, `Projectile`

Her soyut sınıf, Python'ın `abc` modülünü kullanarak alt sınıfların uygulaması zorunlu olan sözleşmeleri tanımlar.

```python
# src/entities/game_object.py
from abc import ABC, abstractmethod

class GameObject(ABC):
    @abstractmethod
    def update(self, dt: float, *args, **kwargs) -> None: ...
    @abstractmethod
    def draw(self, surface: pygame.Surface) -> None: ...
    @abstractmethod
    def is_alive(self) -> bool: ...
```

`GameManager.update()` tüm düşmanları şu şekilde çağırır:
```python
for enemy in self.enemies:
    enemy.update(dt, path)   # Goblin mi, Boss mu olduğu bilinmez — polimorfizm
```

### 2.2 Kapsülleme (Encapsulation)

**Kullanıldığı yerler:** `Enemy`, `Tower`, `Player`, `GameMap`

Kritik iş verileri `__` (çift alt çizgi) ile name-mangling koruması altındadır:

```python
# src/entities/enemies/enemy.py
class Enemy(GameObject):
    def __init__(self, hp, speed, reward, color, size):
        self.__health     = hp      # Dışarıdan erişilemez
        self.__max_health = hp
        self.__speed      = speed
        self.__reward     = reward

    def get_health(self) -> int:    # Kontrollü okuma
        return self.__health

    def set_health(self, value: int) -> None:   # Doğrulama ile yazma
        self.__health = max(0, value)
        if self.__health == 0:
            self._alive = False
```

```python
# src/player/player.py
class Player:
    def spend_gold(self, amount: int) -> bool:
        if amount < 0:
            raise ValueError(...)
        if self.__gold >= amount:
            self.__gold -= amount
            return True
        return False   # Yetersiz altın — işlem reddedildi
```

### 2.3 Kalıtım (Inheritance)

**Kullanıldığı yerler:** Tüm `Enemy`, `Tower`, `Projectile` alt sınıfları

Troll, `take_damage()` metodunu override ederek zırh mekanik ekler:

```python
# src/entities/enemies/troll.py
ARMOR_REDUCTION = 0.30

class Troll(Enemy):
    def take_damage(self, amount: float) -> None:
        reduced = amount * (1.0 - ARMOR_REDUCTION)   # %30 hasar azaltma
        self._apply_base_damage(reduced)              # Üst sınıf yardımcısı
```

Boss, `get_spawn_on_death()` metodunu override eder:
```python
# src/entities/enemies/boss.py
class Boss(Enemy):
    def get_spawn_on_death(self) -> list:
        if self.__death_spawned:
            return []
        self.__death_spawned = True
        return [Goblin(), Goblin()]   # Ölüm anında 2 Goblin
```

### 2.4 Polimorfizm (Polymorphism)

**Kullanıldığı yerler:** `GameManager`, `WaveManager.spawn_enemy()`, `Tower.shoot()`

#### Fabrika Deseni — WaveManager:
```python
def spawn_enemy(self, enemy_type: str):
    factory = {'goblin': Goblin, 'troll': Troll, 'boss': Boss}
    cls = factory.get(enemy_type)
    return cls()   # Hangi tip olduğu çağıran koda şeffaftır
```

#### Polimorfik Kule Ateşlemesi:
```python
# Her kule kendi Projectile alt sınıfını döner:
ArrowTower.shoot()   → Arrow(...)       # Homing, tek hedef
CannonTower.shoot()  → Cannonball(...)  # AOE, sabit konum
IceTower.shoot()     → IceBlast(...)    # Homing + slow debuff

# GameManager tek tip çağrıyla kullanır:
new_proj = tower.update(dt, enemies)   # tower türü bilinmez
if new_proj:
    self.projectiles.append(new_proj)
```

#### Polimorfik Çizim:
```python
for enemy in self.enemies:
    enemy.draw(surface)   # Goblin, Troll veya Boss — hepsi aynı arayüz
```

---

## 3. Tekil Kurulum Mekanizması

### 3.1 Windows Registry Yaklaşımı

```
HKEY_LOCAL_MACHINE\SOFTWARE\TowerDefenseGame
    InstallationID = "INSTALLED_2025_PERMANENT"
```

**Neden çalışır?**

1. **Kurulum öncesi kontrol:** Inno Setup `InitializeSetup()` Pascal fonksiyonu, kurulum penceresi açılmadan önce HKLM ve HKCU anahtarlarını sorgular.
2. **Kurulum sonrası yazma:** `CurStepChanged(ssPostInstall)` adımında anahtar yazılır.
3. **Kaldırma sonrası kalıcılık:** `CurUninstallStepChanged()` içinde anahtar silinmez — bu kasıtlı bir tasarım kararıdır.

### 3.2 Kaldırma Sonrası Neden Çalışmaya Devam Eder?

```
Kurulum → EXE kopyalanır → Kayıt anahtarı yazılır
    ↓
Kaldırma → EXE silinir → Kayıt anahtarı KALIR
    ↓
Yeniden kurma denemesi → InitializeSetup() anahtarı bulur
    ↓
"Bu uygulama daha önce kurulmuştur" mesajı → Kurulum iptal
```

Windows kayıt defteri program dosyalarından bağımsız bir veri deposudur. Program silinse bile kayıt verileri `regedit` ile veya yönetici komutuyla (`reg delete`) elle silinmediği sürece kalır.

### 3.3 HKLM vs HKCU Farkı

| Özellik | HKLM (Local Machine) | HKCU (Current User) |
|---------|---------------------|---------------------|
| Kapsam | Tüm kullanıcılar | Yalnızca oturum açan kullanıcı |
| Yazma yetkisi | Yönetici gerektirir | Standart kullanıcı yazar |
| Kilit gücü | Güçlü (sistem geneli) | Orta (kullanıcı değişince atlanabilir) |
| Tercih sırası | Önce HKLM | HKLM başarısızsa HKCU |

`registry_check.py` içinde her iki hive da kontrol edilir:
```python
for hive in (winreg.HKEY_LOCAL_MACHINE, winreg.HKEY_CURRENT_USER):
    try:
        key = winreg.OpenKey(hive, REG_PATH)
        value, _ = winreg.QueryValueEx(key, REG_KEY)
        if value == REG_VALUE:
            return True
    except FileNotFoundError:
        continue
```

---

## 4. Oyun Mekanikleri

### 4.1 Dalga Sistemi

| Dalga | Düşmanlar | Toplam |
|-------|-----------|--------|
| 1     | 7 Goblin  | 7      |
| 2     | 9 Goblin  | 9      |
| 3     | 11 Goblin | 11     |
| 4     | 8 Goblin + 3 Troll | 11 |
| 5     | 10 Goblin + 4 Troll | 14 |
| 6     | 12 Goblin + 5 Troll | 17 |
| 7     | 10 Goblin + 4 Troll + 1 Boss | 15 |
| 8     | 12 Goblin + 6 Troll | 18 |
| 9     | 14 Goblin + 5 Troll + 1 Boss | 20 |
| 10    | 10 Goblin + 8 Troll + 2 Boss | 20 |

**Formül:** `enemy_count ≈ 5 + wave_number * 2`

**Doğma aralığı:** 0.8 saniye  
**Dalgalar arası:** "Dalga X Başlat" butonu — kullanıcı onayı gerekir

### 4.2 Ekonomi Dengesi

| Kaynak | Değer |
|--------|-------|
| Başlangıç altını | 150 |
| Goblin ödülü | 10 |
| Troll ödülü | 25 |
| Boss ödülü | 100 |
| Ok Kulesi maliyeti | 50 |
| Buz Kulesi maliyeti | 100 |
| Top Kulesi maliyeti | 150 |
| Yükseltme maliyeti | Orijinal × %60 |
| Satış getirisi | Toplam yatırım × %50 |

**Skor formülü:** `toplam_kazanılan_altın × ulaşılan_dalga`

### 4.3 Kule-Düşman Denge Tablosu

| Kule | Goblin (60 HP) | Troll (250 HP) | Boss (1000 HP) |
|------|---------------|----------------|----------------|
| Ok (20 hasar) | 3 isabet | ~18 isabet (zırh ile ~26) | ~72 isabet |
| Top (80 hasar, AOE) | 1 isabet | ~5 isabet (zırh ile ~7) | ~18 isabet |
| Buz (10 hasar + yavaş) | 6 isabet | ~36 isabet | ~143 isabet |

**Not:** Troll %30 zırh azaltmasına sahip olduğundan etkin HP değeri gösterilen HP'nin ~1.43 katıdır.

### 4.4 Yol Haritası

```
Giriş (0,5) → (4,5) → (4,2) → (8,2) → (8,8) → (14,8) → (14,4) → (19,4) Çıkış
```

Piksel koordinatlarında toplam yol uzunluğu ≈ 2050 piksel.  
En hızlı düşman (Goblin, 3.0 karo/sn): yolu ~10.7 saniyede geçer.  
En yavaş düşman (Boss, 0.8 karo/sn): yolu ~40.0 saniyede geçer.

---

## 5. Kurulum Talimatları

### 5.1 Geliştirici Kurulumu

```bash
# 1. Python bağımlılıklarını yükle
pip install pygame pyinstaller

# 2. Oyunu doğrudan çalıştır
python main.py
```

### 5.2 EXE Derleme

```bash
# Proje kökünden çalıştır:
pyinstaller --onefile --windowed --name="TowerDefense" main.py

# Çıktı: dist\TowerDefense.exe
```

### 5.3 Kurulum Paketi Oluşturma

```bash
# Tam derleme (EXE + Inno Setup paketi):
installer\build.bat

# Gereksinimler:
#   - Python 3.10+
#   - pip install pygame pyinstaller
#   - Inno Setup 6 (https://jrsoftware.org/isinfo.php)
```

### 5.4 Sistem Gereksinimleri

| Bileşen | Minimum |
|---------|---------|
| İşletim sistemi | Windows 11 (64-bit) |
| Python | 3.10+ (geliştirme için) |
| RAM | 256 MB |
| Depolama | 50 MB |
| Ekran çözünürlüğü | 1536 × 768 veya üstü |

### 5.5 Kontroller

| Eylem | Tuş/Fare |
|-------|----------|
| Kule kurulum seç | Yeşil karoya sol tık |
| Kule yönet (yükselt/sat) | Kurulu kuleye sağ tık veya sol tık |
| Seçimi iptal | ESC |
| Dalga başlat | SPACE veya "Başlat" butonu |
| Oyun hızı değiştir | F1 veya "Hız" butonu |

---

## 6. Dosya Yapısı

```
tower_defense/
├── main.py                          ← Giriş noktası
├── settings.py                      ← Tüm sabitler
├── src/
│   ├── game.py                      ← pygame sarmalayıcı
│   ├── entities/
│   │   ├── game_object.py           ← ABC taban
│   │   ├── enemies/
│   │   │   ├── enemy.py             ← ABC düşman + hareket motoru
│   │   │   ├── goblin.py
│   │   │   ├── troll.py             ← Zırh mekanik
│   │   │   └── boss.py              ← Ölüm spawn mekanik
│   │   ├── towers/
│   │   │   ├── tower.py             ← ABC kule + hedefleme + yükseltme
│   │   │   ├── arrow_tower.py
│   │   │   ├── cannon_tower.py      ← AOE
│   │   │   └── ice_tower.py         ← Slow debuff
│   │   └── projectiles/
│   │       ├── projectile.py        ← ABC mermi + hareket yardımcısı
│   │       ├── arrow.py
│   │       ├── cannonball.py        ← AOE patlama
│   │       └── ice_blast.py         ← Slow uygulama
│   ├── map/
│   │   ├── tile.py                  ← Karo + TileType enum
│   │   └── game_map.py              ← 20×12 ızgara + yol hesaplama
│   ├── player/
│   │   └── player.py                ← Altın/can kapsülleme
│   └── managers/
│       ├── wave_manager.py          ← Dalga + fabrika deseni
│       ├── ui_manager.py            ← HUD + popup + overlay
│       └── game_manager.py          ← Merkezi koordinatör
├── installer/
│   ├── registry_check.py            ← Kayıt defteri kilit API
│   ├── installer_script.iss         ← Inno Setup betiği
│   └── build.bat                    ← Tek tık derleme
└── docs/
    └── technical_documentation.md  ← Bu dosya
```

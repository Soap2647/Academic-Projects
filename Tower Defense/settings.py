"""
Oyun genelinde kullanılan tüm sabitler ve konfigürasyon değerleri.
Bu modül herhangi bir oyun sınıfı içermez; yalnızca yapılandırma verileri tutar.
"""

# Pencere ve harita boyutları
WINDOW_WIDTH  = 1536   # Harita (1280) + UI paneli (256)
WINDOW_HEIGHT = 768
TILE_SIZE     = 64
FPS           = 60

GRID_COLS      = 20
GRID_ROWS      = 12
UI_PANEL_WIDTH = 256
MAP_WIDTH      = GRID_COLS * TILE_SIZE   # 1280
MAP_HEIGHT     = GRID_ROWS * TILE_SIZE   # 768

# Yol ara noktaları  (sütun, satır) ızgara koordinatları
PATH_WAYPOINTS = [
    (0, 5), (4, 5), (4, 2), (8, 2),
    (8, 8), (14, 8), (14, 4), (19, 4),
]

# ── Renk paleti ────────────────────────────────────────────────────────────────
COLORS = {
    'background':        (30,  30,  30 ),
    'path':              (139, 90,  43 ),
    'path_dark':         (110, 70,  30 ),
    'buildable':         (34,  85,  34 ),
    'buildable_hover':   (55,  130, 55 ),
    'ui_bg':             (20,  20,  40 ),
    'ui_border':         (60,  60,  100),
    'text':              (255, 255, 255),
    'text_shadow':       (0,   0,   0  ),
    'gold':              (255, 215, 0  ),
    'health_bar_bg':     (60,  0,   0  ),
    'health_bar_fg':     (0,   200, 0  ),
    'lives':             (220, 50,  50 ),
    'wave_btn':          (50,  150, 50 ),
    'wave_btn_hover':    (70,  200, 70 ),
    'sell_btn':          (200, 60,  60 ),
    'upgrade_btn':       (50,  100, 200),
    'popup_bg':          (25,  25,  55 ),
    'popup_border':      (100, 100, 200),
    'speed_btn':         (80,  80,  120),
    'range_circle':      (255, 255, 255),
    'particle_flash':    (255, 255, 180),
    'damage_text':       (255, 230, 50 ),
    'gold_text':         (50,  220, 50 ),
    'entry_marker':      (50,  255, 50 ),
    'exit_marker':       (255, 50,  50 ),
}

# ── Kule verileri ──────────────────────────────────────────────────────────────
TOWER_DATA = {
    'arrow': {
        'damage':    20,
        'range':     150,
        'fire_rate': 1.5,          # atış/saniye
        'cost':      50,
        'color':     (139, 69, 19),
        'name':      'Ok Kulesi',
        'desc':      'Hızlı ateşler, tek hedef\nEn hızlı düşmanı hedefler',
    },
    'cannon': {
        'damage':    80,
        'range':     120,
        'fire_rate': 0.5,
        'cost':      150,
        'color':     (80, 80, 80),
        'name':      'Top Kulesi',
        'desc':      'Yavaş ateşler, alan hasarı\nİlk düşmanı hedefler',
    },
    'ice': {
        'damage':    10,
        'range':     120,
        'fire_rate': 1.0,
        'cost':      100,
        'color':     (100, 180, 255),
        'name':      'Buz Kulesi',
        'desc':      'Düşmanları yavaşlatır\nOrta hasar verir',
    },
}

# ── Düşman verileri ────────────────────────────────────────────────────────────
ENEMY_DATA = {
    'goblin': {'hp': 60,   'speed': 3.0, 'reward': 10,  'color': (50,  200, 50 )},
    'troll':  {'hp': 250,  'speed': 1.2, 'reward': 25,  'color': (50,  50,  200)},
    'boss':   {'hp': 1000, 'speed': 0.8, 'reward': 100, 'color': (200, 50,  50 )},
}

# ── Dalga konfigürasyonu ───────────────────────────────────────────────────────
# Her dalga için (düşman_tipi, adet) listesi
WAVE_ENEMIES = {
    1:  [('goblin',  7)],
    2:  [('goblin',  9)],
    3:  [('goblin', 11)],
    4:  [('goblin',  8), ('troll',  3)],
    5:  [('goblin', 10), ('troll',  4)],
    6:  [('goblin', 12), ('troll',  5)],
    7:  [('goblin', 10), ('troll',  4), ('boss', 1)],
    8:  [('goblin', 12), ('troll',  6)],
    9:  [('goblin', 14), ('troll',  5), ('boss', 1)],
    10: [('goblin', 10), ('troll',  8), ('boss', 2)],
}

TOTAL_WAVES      = 10
SPAWN_INTERVAL   = 0.8    # saniye — düşmanlar arası doğma aralığı
WAVE_COUNTDOWN   = 3.0    # saniye — dalgalar arası bekleme süresi

# ── Oyun dengesi sabitleri ─────────────────────────────────────────────────────
CANNON_AOE_RADIUS   = 60
ICE_SLOW_FACTOR     = 0.4
ICE_SLOW_DURATION   = 2.0
UPGRADE_DAMAGE_MULT = 1.25
UPGRADE_RANGE_MULT  = 1.10
UPGRADE_COST_RATIO  = 0.60   # orijinal maliyetin %60'ı kadar yükseltme ücreti
SELL_RATIO          = 0.50   # toplam yatırımın %50'si geri alınır
PROJECTILE_HIT_DIST = 12     # piksel — isabet mesafesi eşiği

# Başlangıç oyuncu değerleri
STARTING_GOLD  = 150
STARTING_LIVES = 20

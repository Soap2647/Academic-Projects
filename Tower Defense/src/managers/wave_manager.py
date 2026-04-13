"""
Dalga yöneticisi sınıfı.

OOP Rolü (Fabrika Deseni + Kapsülleme):
    WaveManager, düşman oluşturma fabrikasını ve dalga zamanlama
    mantığını kapsüller.  spawn_enemy() metodu, polimorfik Enemy
    alt sınıflarını tip adına göre örnekler (Fabrika Deseni).
"""

from __future__ import annotations

from settings import (
    WAVE_ENEMIES, TOTAL_WAVES, SPAWN_INTERVAL, WAVE_COUNTDOWN,
    PATH_WAYPOINTS, TILE_SIZE,
)


class WaveManager:
    """
    Dalga sistemi ve düşman doğurma mantığını yöneten sınıf.

    Kapsülleme: aktif dalga, doğma sırası ve zamanlayıcılar
    dışarıdan değiştirilemez; yalnızca public metotlar aracılığıyla
    kontrol edilir.
    """

    def __init__(self) -> None:
        self.__current_wave    = 0        # Henüz başlamadı
        self.__spawn_queue: list[str] = []  # Doğacak düşmanların tip listesi
        self.__spawn_timer     = 0.0
        self.__wave_active     = False
        self.__all_complete    = False
        self.__countdown       = 0.0      # Dalga öncesi geri sayım
        self.__waiting         = False    # Kullanıcı 'Başlat' bekliyor

        # Giriş noktası piksel koordinatı
        first_wp = PATH_WAYPOINTS[0]
        self.__entry_x = first_wp[0] * TILE_SIZE + TILE_SIZE / 2
        self.__entry_y = first_wp[1] * TILE_SIZE + TILE_SIZE / 2

    # ── Public API ─────────────────────────────────────────────────────────────

    def start_next_wave(self) -> bool:
        """
        Bir sonraki dalgayı başlatır.

        Returns:
            Dalga başlatıldıysa True; tüm dalgalar bittiyse False.
        """
        if self.__all_complete:
            return False
        next_wave = self.__current_wave + 1
        if next_wave > TOTAL_WAVES:
            self.__all_complete = True
            return False

        self.__current_wave = next_wave
        self.__spawn_queue  = self.__build_spawn_queue(next_wave)
        self.__spawn_timer  = 0.0
        self.__wave_active  = True
        self.__waiting      = False
        self.__countdown    = 0.0
        return True

    def update(self, dt: float) -> list:
        """
        Doğma zamanlayıcısını günceller ve yeni düşman nesneleri döner.

        Args:
            dt: Delta zaman (saniye).

        Returns:
            Bu karede doğacak Enemy nesneleri listesi.
        """
        new_enemies: list = []

        if not self.__wave_active or not self.__spawn_queue:
            return new_enemies

        self.__spawn_timer += dt
        while self.__spawn_timer >= SPAWN_INTERVAL and self.__spawn_queue:
            self.__spawn_timer -= SPAWN_INTERVAL
            enemy_type         = self.__spawn_queue.pop(0)
            enemy              = self.spawn_enemy(enemy_type)
            new_enemies.append(enemy)

        if not self.__spawn_queue:
            self.__wave_active = False

        return new_enemies

    def spawn_enemy(self, enemy_type: str):
        """
        Fabrika deseni — düşman tipine göre somut sınıf örnekler.

        Polimorfizm: enemy_type string değerine göre farklı Enemy
        alt sınıfları oluşturulur; çağıran kod türü bilmez.

        Args:
            enemy_type: 'goblin' | 'troll' | 'boss'

        Returns:
            Oluşturulan Enemy nesnesi.
        """
        from src.entities.enemies.goblin import Goblin
        from src.entities.enemies.troll  import Troll
        from src.entities.enemies.boss   import Boss

        factory = {
            'goblin': Goblin,
            'troll':  Troll,
            'boss':   Boss,
        }
        cls   = factory.get(enemy_type)
        if cls is None:
            raise ValueError(f"Bilinmeyen düşman tipi: {enemy_type!r}")

        enemy   = cls()
        enemy.x = self.__entry_x
        enemy.y = self.__entry_y
        return enemy

    # ── Durum sorgu metotları ──────────────────────────────────────────────────

    def is_wave_active(self) -> bool:
        """Aktif olarak düşman doğuruluyorsa True döner."""
        return self.__wave_active

    def current_wave_spawning_done(self) -> bool:
        """Mevcut dalganın tüm düşmanları doğurulmuşsa True döner."""
        return not self.__wave_active and self.__current_wave > 0

    def all_waves_complete(self) -> bool:
        """Tüm dalgalar tamamlandıysa True döner."""
        return self.__all_complete

    def get_current_wave(self) -> int:
        return self.__current_wave

    def get_total_waves(self) -> int:
        return TOTAL_WAVES

    # ── İç yardımcılar ────────────────────────────────────────────────────────

    def __build_spawn_queue(self, wave_number: int) -> list[str]:
        """
        Dalga numarasına göre karıştırılmış doğma sırası oluşturur.

        Doğma sırası: tüm düşman tipleri, sıralı bloklar hâlinde değil,
        iyice karıştırılarak düzenlenir (daha ilginç oynanış için).
        """
        import random
        configs = WAVE_ENEMIES.get(wave_number, [])
        queue: list[str] = []
        for enemy_type, count in configs:
            queue.extend([enemy_type] * count)

        # Dalgalar arası karıştırma — Boss'lar en sona bırakılır
        non_boss = [e for e in queue if e != 'boss']
        bosses   = [e for e in queue if e == 'boss']
        random.shuffle(non_boss)
        return non_boss + bosses

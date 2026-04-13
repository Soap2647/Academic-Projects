"""
Boss düşman sınıfı.

OOP Rolü (Kalıtım + Polimorfizm + Özel Override):
    Boss, take_damage metodunu override ederek ölüm anında 2 Goblin
    spawner mekanik ekler.  Bu, polimorfizm ve kalıtımın aynı anda
    kullanıldığı en karmaşık düşman örneğidir.
"""

from __future__ import annotations

import pygame

from settings import ENEMY_DATA
from src.entities.enemies.enemy import Enemy


class Boss(Enemy):
    """
    Çok sağlıklı, yavaş ve ölümünde 2 Goblin doğuran boss düşmanı.

    Özellikler:
        HP            : 1000
        Hız           : 0.8 karo/saniye
        Ödül          : 100 altın
        Renk          : Kırmızı
        Boyut         : Normal düşmanın 2 katı
        Ölüm efekti   : 2 Goblin spawn eder
    """

    def __init__(self) -> None:
        data = ENEMY_DATA['boss']
        super().__init__(
            hp     = data['hp'],
            speed  = data['speed'],
            reward = data['reward'],
            color  = data['color'],
            size   = 32,   # Normal boyutun 2 katı
        )
        self.__death_spawned = False   # Ölüm spawn'ı bir kez yapılır

    def take_damage(self, amount: float) -> None:
        """
        Boss'a hasar uygular.  Sağlık sıfırlanırsa ölüm bayrağı kurulur.

        Args:
            amount: Ham hasar miktarı (Boss'un zırhı yoktur).
        """
        self._apply_base_damage(amount)

    def get_type(self) -> str:
        """Düşman türünü döner."""
        return 'boss'

    def get_spawn_on_death(self) -> list:
        """
        Boss öldüğünde çağrılır ve 2 Goblin nesnesi döner.
        Spawn yalnızca bir kez gerçekleşir (çift çağrıma karşı koruma).

        Returns:
            Mevcut pozisyondan başlayacak 2 Goblin nesnesi listesi.
        """
        if self.__death_spawned:
            return []
        self.__death_spawned = True

        from src.entities.enemies.goblin import Goblin

        goblins: list[Goblin] = []
        for offset in (-20, 20):
            g = Goblin()
            g.x          = self.x + offset
            g.y          = self.y
            g._path_index = self._path_index   # aynı yol noktasından devam
            goblins.append(g)
        return goblins

    def draw(self, surface: pygame.Surface) -> None:
        """Boss gövdesini, taç ve korona detaylarını çizer."""
        super().draw(surface)
        cx, cy = int(self.x), int(self.y)
        r = self._radius

        # Taç (kral sembolü)
        crown_color  = (255, 215, 0)
        crown_pts = [
            (cx - 14, cy - r + 2),
            (cx - 14, cy - r - 10),
            (cx - 7,  cy - r - 4),
            (cx,      cy - r - 12),
            (cx + 7,  cy - r - 4),
            (cx + 14, cy - r - 10),
            (cx + 14, cy - r + 2),
        ]
        pygame.draw.polygon(surface, crown_color, crown_pts)
        pygame.draw.polygon(surface, (180, 140, 0), crown_pts, 1)

        # Öfke çizgileri
        pygame.draw.line(surface, (255, 100, 50), (cx - r, cy - 6), (cx - r + 8, cy - 2), 2)
        pygame.draw.line(surface, (255, 100, 50), (cx + r, cy - 6), (cx + r - 8, cy - 2), 2)

        # Sağlık yüzdesini metin olarak göster
        font = pygame.font.SysFont(None, 16)
        hp_pct = int(self.get_health() / self.get_max_health() * 100)
        txt = font.render(f'{hp_pct}%', True, (255, 255, 255))
        surface.blit(txt, (cx - txt.get_width() // 2, cy - 6))

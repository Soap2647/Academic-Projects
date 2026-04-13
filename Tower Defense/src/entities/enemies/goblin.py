"""
Goblin düşman sınıfı.

OOP Rolü (Somut Uygulama / Polimorfizm):
    Goblin, Enemy'nin en temel somut uygulamasıdır.  Zırh ya da özel
    ölüm efekti içermez; hızlı ama düşük sağlıklıdır.
"""

import pygame

from settings import ENEMY_DATA
from src.entities.enemies.enemy import Enemy


class Goblin(Enemy):
    """
    Hızlı, düşük sağlıklı temel düşman türü.

    Özellikler:
        HP    : 60
        Hız   : 3.0 karo/saniye
        Ödül  : 10 altın
        Renk  : Yeşil
    """

    def __init__(self) -> None:
        data = ENEMY_DATA['goblin']
        super().__init__(
            hp     = data['hp'],
            speed  = data['speed'],
            reward = data['reward'],
            color  = data['color'],
            size   = 16,
        )

    def take_damage(self, amount: float) -> None:
        """
        Gobline doğrudan hasar uygular — zırh yok.

        Args:
            amount: Uygulanacak ham hasar miktarı.
        """
        self._apply_base_damage(amount)

    def get_type(self) -> str:
        """Düşman türünü döner."""
        return 'goblin'

    def draw(self, surface: pygame.Surface) -> None:
        """Goblin gövdesini ve kulak detayını çizer."""
        super().draw(surface)
        # Küçük kulak detayları
        cx, cy = int(self.x), int(self.y)
        r = self._radius
        ear_color = (30, 150, 30)
        pygame.draw.circle(surface, ear_color, (cx - r + 4, cy - r + 6), 5)
        pygame.draw.circle(surface, ear_color, (cx + r - 4, cy - r + 6), 5)

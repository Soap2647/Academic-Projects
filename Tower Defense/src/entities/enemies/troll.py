"""
Troll düşman sınıfı.

OOP Rolü (Kalıtım + Polimorfizm):
    Troll, Enemy'nin take_damage metodunu override ederek zırh mekanik
    ekler.  %30 hasar azaltma, polimorfik hasar dağıtımının somut
    göstergesidir: aynı 'take_damage(amount)' çağrısı Goblin'e tam hasar
    verirken Troll'a azaltılmış hasar verir.
"""

import pygame

from settings import ENEMY_DATA
from src.entities.enemies.enemy import Enemy

# Troll zırh oranı — hasarın bu kadarı bloke edilir
ARMOR_REDUCTION = 0.30


class Troll(Enemy):
    """
    Yavaş, dayanıklı ve zırhlı düşman türü.

    Özellikler:
        HP              : 250
        Hız             : 1.2 karo/saniye
        Zırh azaltma    : %30
        Ödül            : 25 altın
        Renk            : Mavi
    """

    def __init__(self) -> None:
        data = ENEMY_DATA['troll']
        super().__init__(
            hp     = data['hp'],
            speed  = data['speed'],
            reward = data['reward'],
            color  = data['color'],
            size   = 22,
        )

    def take_damage(self, amount: float) -> None:
        """
        Zırh ile hasar uygular — hasarın %30'u bloke edilir.

        Args:
            amount: Ham hasar miktarı (zırh uygulanmadan önce).
        """
        reduced = amount * (1.0 - ARMOR_REDUCTION)
        self._apply_base_damage(reduced)

    def get_type(self) -> str:
        """Düşman türünü döner."""
        return 'troll'

    def draw(self, surface: pygame.Surface) -> None:
        """Troll gövdesini ve zırh göstergesini çizer."""
        super().draw(surface)
        cx, cy = int(self.x), int(self.y)
        r = self._radius
        # Zırh plakası efekti — metalik dış çember
        pygame.draw.circle(surface, (100, 100, 140), (cx, cy), r, 3)
        # Zırh sembolü — küçük kalkan
        shield_pts = [
            (cx,     cy - 8),
            (cx - 7, cy - 3),
            (cx - 7, cy + 5),
            (cx,     cy + 9),
            (cx + 7, cy + 5),
            (cx + 7, cy - 3),
        ]
        pygame.draw.polygon(surface, (160, 160, 200), shield_pts)
        pygame.draw.polygon(surface, (80, 80, 120),   shield_pts, 1)

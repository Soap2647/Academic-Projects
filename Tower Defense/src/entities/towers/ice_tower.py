"""
Buz Kulesi sınıfı.

OOP Rolü (Somut Uygulama + Polimorfizm):
    IceTower, shoot() metodunu override ederek IceBlast mermisi döner.
    Hedefleme: yolda en önde olan düşman.
    Yavaşlama debuff IceBlast.on_hit() içinde uygulanır.
"""

from __future__ import annotations

from typing import Optional

import pygame

from settings import TOWER_DATA
from src.entities.towers.tower import Tower
from src.entities.projectiles.ice_blast import IceBlast


class IceTower(Tower):
    """
    Düşmanları yavaşlatan buz kulesi.

    Özellikler:
        Hasar      : 10
        Menzil     : 120
        Ateş hızı  : 1.0/sn
        Yavaşlama  : %60 (slow_factor = 0.4), 2 saniye
        Maliyet    : 100 altın
        Hedefleme  : İlk düşman
    """

    def __init__(self, x: float, y: float) -> None:
        data = TOWER_DATA['ice']
        super().__init__(
            x         = x,
            y         = y,
            damage    = data['damage'],
            rng       = data['range'],
            fire_rate = data['fire_rate'],
            cost      = data['cost'],
            color     = data['color'],
        )

    def shoot(self, enemies: list) -> Optional[IceBlast]:
        """
        Hedefe buz patlaması ateşler.

        Polimorfizm: Tower.shoot() → IceBlast döner.
        """
        target = self.find_target(enemies)
        if target is None:
            return None
        return IceBlast(self.x, self.y, self.get_damage(), target)

    def get_tower_type(self) -> str:
        return 'ice'

    def draw(self, surface: pygame.Surface) -> None:
        """Buz kulesi gövdesi ve kristal sembolü."""
        super().draw(surface)
        cx, cy = int(self.x), int(self.y)
        # Kristal sembolü — altı kollu yıldız
        crystal_color = (200, 240, 255)
        size = 10
        for angle_deg in range(0, 360, 60):
            import math
            rad = math.radians(angle_deg)
            ex  = cx + int(math.cos(rad) * size)
            ey  = cy + int(math.sin(rad) * size)
            pygame.draw.line(surface, crystal_color, (cx, cy), (ex, ey), 2)
        pygame.draw.circle(surface, (255, 255, 255), (cx, cy), 4)

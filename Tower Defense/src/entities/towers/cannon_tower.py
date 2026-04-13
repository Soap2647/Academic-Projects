"""
Top Kulesi sınıfı.

OOP Rolü (Somut Uygulama + Polimorfizm):
    CannonTower, shoot() metodunu override ederek Cannonball mermisi döner.
    Hedefleme: yolda en önde olan düşman (temel find_target mantığı).
    AOE hasar Cannonball.on_hit() içinde uygulanır.
"""

from __future__ import annotations

from typing import Optional

import pygame

from settings import TOWER_DATA, TILE_SIZE
from src.entities.towers.tower import Tower
from src.entities.projectiles.cannonball import Cannonball


class CannonTower(Tower):
    """
    Yavaş ateşleyen, geniş alan hasarlı top kulesi.

    Özellikler:
        Hasar       : 80  (AOE)
        Menzil      : 120
        Ateş hızı   : 0.5/sn
        AOE yarıçapı: 60 piksel
        Maliyet     : 150 altın
        Hedefleme   : İlk düşman (yolda en önde olan)
    """

    def __init__(self, x: float, y: float) -> None:
        data = TOWER_DATA['cannon']
        super().__init__(
            x         = x,
            y         = y,
            damage    = data['damage'],
            rng       = data['range'],
            fire_rate = data['fire_rate'],
            cost      = data['cost'],
            color     = data['color'],
        )

    def shoot(self, enemies: list) -> Optional[Cannonball]:
        """
        Hedefe top mermisi ateşler.
        Mermi sabit bir konuma gider (ateş anındaki düşman pozisyonu).

        Polimorfizm: Tower.shoot() → Cannonball döner.
        """
        target = self.find_target(enemies)
        if target is None:
            return None
        return Cannonball(
            x        = self.x,
            y        = self.y,
            damage   = self.get_damage(),
            target_x = target.x,
            target_y = target.y,
        )

    def get_tower_type(self) -> str:
        return 'cannon'

    def draw(self, surface: pygame.Surface) -> None:
        """Top kulesi gövdesi ve namlu sembolü."""
        super().draw(surface)
        cx, cy = int(self.x), int(self.y)
        # Namlu
        pygame.draw.rect(surface, (50, 50, 50),
                         (cx - 4, cy - 14, 8, 16))
        pygame.draw.rect(surface, (100, 100, 100),
                         (cx - 4, cy - 14, 8, 16), 1)
        # Top yuvarlağı
        pygame.draw.circle(surface, (120, 120, 120), (cx, cy + 2), 8)
        pygame.draw.circle(surface, (80, 80, 80), (cx, cy + 2), 8, 1)

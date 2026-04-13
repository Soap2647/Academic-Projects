"""
Ok Kulesi sınıfı.

OOP Rolü (Somut Uygulama + Polimorfizm):
    ArrowTower, shoot() metodunu override ederek Arrow mermisi döner.
    Hedefleme stratejisi: sahadaki en hızlı düşmanı (en yüksek hız statına
    sahip) seçer — find_target override edilir.
"""

from __future__ import annotations

from typing import Optional

import pygame

from settings import TOWER_DATA, TILE_SIZE
from src.entities.towers.tower import Tower
from src.entities.projectiles.arrow import Arrow


class ArrowTower(Tower):
    """
    Hızlı ateşleyen, tek hedefli ok kulesi.

    Özellikler:
        Hasar      : 20
        Menzil     : 150
        Ateş hızı  : 1.5/sn
        Maliyet    : 50 altın
        Hedefleme  : En hızlı düşman (hız statına göre)
    """

    def __init__(self, x: float, y: float) -> None:
        data = TOWER_DATA['arrow']
        super().__init__(
            x         = x,
            y         = y,
            damage    = data['damage'],
            rng       = data['range'],
            fire_rate = data['fire_rate'],
            cost      = data['cost'],
            color     = data['color'],
        )

    def find_target(self, enemies: list) -> Optional[object]:
        """
        Menzil içindeki en yüksek hıza sahip düşmanı seçer.

        Polimorfizm: Tower.find_target() override edilir.
        """
        in_range = [
            e for e in enemies
            if e.is_alive() and self.distance_to(e) <= self.get_range()
        ]
        if not in_range:
            return None
        return max(in_range, key=lambda e: e.get_speed())

    def shoot(self, enemies: list) -> Optional[Arrow]:
        """
        Hedef bulunursa Arrow mermisi oluşturur ve döner.

        Polimorfizm: Tower.shoot() → Arrow döner.
        """
        target = self.find_target(enemies)
        if target is None:
            return None
        return Arrow(self.x, self.y, self.get_damage(), target)

    def get_tower_type(self) -> str:
        return 'arrow'

    def draw(self, surface: pygame.Surface) -> None:
        """Ok kulesi gövdesi ve ok sembolü."""
        super().draw(surface)
        cx, cy = int(self.x), int(self.y)
        # Ok sembolü
        pygame.draw.line(surface, (255, 200, 100),
                         (cx - 10, cy), (cx + 10, cy), 2)
        pygame.draw.polygon(surface, (255, 200, 100), [
            (cx + 10, cy), (cx + 4, cy - 5), (cx + 4, cy + 5)
        ])

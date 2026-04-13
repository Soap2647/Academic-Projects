"""
Top mermisi sınıfı.

OOP Rolü (Somut Uygulama):
    Sabit bir konuma doğru hareket eder ve varışta AOE hasar verir.
    CannonTower tarafından oluşturulur.
"""

from __future__ import annotations

import pygame

from settings import CANNON_AOE_RADIUS
from src.entities.projectiles.projectile import Projectile


class Cannonball(Projectile):
    """
    Sabit hedefe giden ve varışta çevresel alan hasarı veren top mermisi.

    Özellikler:
        Hız          : 220 piksel/saniye
        Alan yarıçapı: CANNON_AOE_RADIUS (60 piksel)
        Renk         : Koyu gri
    """

    SPEED = 220.0

    def __init__(
        self,
        x:        float,
        y:        float,
        damage:   float,
        target_x: float,
        target_y: float,
    ) -> None:
        """
        Args:
            x:        Başlangıç x koordinatı.
            y:        Başlangıç y koordinatı.
            damage:   Verilen AOE hasarı.
            target_x: Hedef konum x (ateş anındaki düşman pozisyonu).
            target_y: Hedef konum y.
        """
        super().__init__(x, y, damage, Cannonball.SPEED,
                         color=(60, 60, 60), radius=7)
        self.__target_x  = target_x
        self.__target_y  = target_y
        self.__hit_flag  = False   # İsabet gerçekleşti mi?

    def update(self, dt: float, *args, **kwargs) -> None:
        """Sabit hedef konumuna doğru hareket eder."""
        if not self._alive:
            return

        remaining = self._move_toward(self.__target_x, self.__target_y, dt)
        if remaining == 0.0:
            self.__hit_flag = True
            self._alive     = False

    def on_hit(self, enemies: list) -> list[tuple]:
        """
        Varış noktası çevresindeki tüm düşmanlara hasar verir (AOE).

        Args:
            enemies: Sahadaki tüm düşmanlar.

        Returns:
            (düşman, hasar) çiftleri listesi.
        """
        results = []
        for enemy in enemies:
            import math
            dx   = enemy.x - self.__target_x
            dy   = enemy.y - self.__target_y
            dist = math.sqrt(dx * dx + dy * dy)
            if dist <= CANNON_AOE_RADIUS:
                results.append((enemy, self.get_damage()))
        return results

    def get_projectile_type(self) -> str:
        return 'cannonball'

    def draw(self, surface: pygame.Surface) -> None:
        """Top mermisini çizer — patlama efekti eklenir."""
        if not self._alive and not self.__hit_flag:
            return
        super().draw(surface)
        # Metalik vurgu noktası
        pygame.draw.circle(surface, (120, 120, 120),
                           (int(self.x) - 2, int(self.y) - 2), 2)

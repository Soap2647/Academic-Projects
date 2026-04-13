"""
Ok mermisi sınıfı.

OOP Rolü (Somut Uygulama):
    Hedefi takip eden (homing) tek hedefli mermi.
    ArrowTower tarafından oluşturulur.
"""

from __future__ import annotations

import pygame

from settings import PROJECTILE_HIT_DIST
from src.entities.projectiles.projectile import Projectile


class Arrow(Projectile):
    """
    Hedefi takip eden, tek hedefe hasar veren ok mermisi.

    Özellikler:
        Hız    : 400 piksel/saniye
        Hasar  : Tower'dan alınan değer
        Renk   : Turuncu-kahve
    """

    SPEED = 400.0

    def __init__(self, x: float, y: float, damage: float, target) -> None:
        """
        Args:
            x:      Başlangıç x koordinatı.
            y:      Başlangıç y koordinatı.
            damage: Uygulanacak hasar miktarı.
            target: Takip edilecek düşman nesnesi (Enemy alt sınıfı).
        """
        super().__init__(x, y, damage, Arrow.SPEED,
                         color=(220, 140, 60), radius=4)
        self.__target = target

    def update(self, dt: float, *args, **kwargs) -> None:
        """
        Her karede hedefi takip eder.
        Hedef ölmüşse mermi de yok olur.
        """
        if not self._alive:
            return

        if self.__target is None or not self.__target.is_alive():
            self._alive = False
            return

        remaining = self._move_toward(self.__target.x, self.__target.y, dt)
        if remaining == 0.0:
            # Hedefe ulaşıldı
            self._alive = False

    def on_hit(self, enemies: list) -> list[tuple]:
        """
        İsabet anında çağrılır — tek hedefe hasar verir.

        Returns:
            [(hedef_düşman, hasar)] tek elemanlı liste.
        """
        if self.__target and self.__target.is_alive():
            return [(self.__target, self.get_damage())]
        return []

    def get_projectile_type(self) -> str:
        return 'arrow'

    def draw(self, surface: pygame.Surface) -> None:
        """Ok şeklinde çizer."""
        import math
        if not self._alive:
            return

        # Oku hedef yönüne döndür
        if self.__target and self.__target.is_alive():
            dx = self.__target.x - self.x
            dy = self.__target.y - self.y
            angle = math.atan2(-dy, dx)
        else:
            angle = 0.0

        cx, cy = int(self.x), int(self.y)
        length = 10
        ex = cx + int(math.cos(angle) * length)
        ey = cy - int(math.sin(angle) * length)

        pygame.draw.line(surface, (180, 110, 40), (cx, cy), (ex, ey), 2)
        pygame.draw.circle(surface, (220, 160, 60), (cx, cy), 3)

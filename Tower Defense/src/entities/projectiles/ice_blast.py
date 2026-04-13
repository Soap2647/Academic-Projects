"""
Buz patlaması mermisi sınıfı.

OOP Rolü (Somut Uygulama):
    Hedefi takip eder; isabet anında hasar + yavaşlama debuff uygular.
    IceTower tarafından oluşturulur.
"""

from __future__ import annotations

import pygame

from settings import ICE_SLOW_FACTOR, ICE_SLOW_DURATION
from src.entities.projectiles.projectile import Projectile


class IceBlast(Projectile):
    """
    Hedefi yavaşlatan buz patlaması mermisi.

    Özellikler:
        Hız          : 300 piksel/saniye
        Yavaşlama    : Hız * ICE_SLOW_FACTOR (0.4), 2 saniye
        Renk         : Açık mavi
    """

    SPEED = 300.0

    def __init__(self, x: float, y: float, damage: float, target) -> None:
        """
        Args:
            x:      Başlangıç x koordinatı.
            y:      Başlangıç y koordinatı.
            damage: Uygulanacak hasar miktarı.
            target: Takip edilecek düşman nesnesi.
        """
        super().__init__(x, y, damage, IceBlast.SPEED,
                         color=(180, 230, 255), radius=5)
        self.__target = target

    def update(self, dt: float, *args, **kwargs) -> None:
        """Her karede hedefi takip eder."""
        if not self._alive:
            return

        if self.__target is None or not self.__target.is_alive():
            self._alive = False
            return

        remaining = self._move_toward(self.__target.x, self.__target.y, dt)
        if remaining == 0.0:
            self._alive = False

    def on_hit(self, enemies: list) -> list[tuple]:
        """
        İsabet anında hedefe hasar ve yavaşlama debuff uygular.

        Returns:
            [(hedef_düşman, hasar)] tek elemanlı liste.
        """
        if self.__target and self.__target.is_alive():
            self.__target.apply_slow(ICE_SLOW_FACTOR, ICE_SLOW_DURATION)
            return [(self.__target, self.get_damage())]
        return []

    def get_projectile_type(self) -> str:
        return 'ice_blast'

    def draw(self, surface: pygame.Surface) -> None:
        """Buz kristali şeklinde çizer."""
        super().draw(surface)
        # Buz parıltısı efekti
        cx, cy = int(self.x), int(self.y)
        pygame.draw.line(surface, (220, 240, 255), (cx - 4, cy), (cx + 4, cy), 1)
        pygame.draw.line(surface, (220, 240, 255), (cx, cy - 4), (cx, cy + 4), 1)

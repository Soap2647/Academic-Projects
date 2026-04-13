"""
Soyut mermi (projectile) temel sınıfı.

OOP Rolü (Soyutlama + Kalıtım):
    Tüm mermiler bu sınıftan türer.  'on_hit' ve 'get_projectile_type'
    alt sınıfların uygulaması gereken sözleşmeyi tanımlar.  Hareket
    mantığı somut alt sınıflarda implement edilir.
"""

from __future__ import annotations

import math
from abc import abstractmethod

import pygame

from src.entities.game_object import GameObject


class Projectile(GameObject):
    """
    Ok, top mermisi ve buz patlamasının soyut taban sınıfı.

    Kapsülleme: hasar ve hız değerleri '__' ile gizlenmiştir.
    """

    def __init__(
        self,
        x:      float,
        y:      float,
        damage: float,
        speed:  float,
        color:  tuple[int, int, int],
        radius: int = 5,
    ) -> None:
        super().__init__(x, y, radius * 2, radius * 2)
        self.__damage  = damage
        self.__speed   = speed   # piksel/saniye
        self._color    = color
        self._radius   = radius
        self._alive    = True

    # ── Soyut metotlar ─────────────────────────────────────────────────────────

    @abstractmethod
    def on_hit(self, enemies: list) -> list[tuple]:
        """
        İsabet anında çağrılır.

        Args:
            enemies: Sahadaki tüm düşmanlar listesi (AOE için).

        Returns:
            (düşman, hasar_miktarı) çiftlerinin listesi.
        """

    @abstractmethod
    def get_projectile_type(self) -> str:
        """Mermi türünü döner: 'arrow' | 'cannonball' | 'ice_blast'."""

    # ── Soyut hareket metodu ───────────────────────────────────────────────────

    @abstractmethod
    def update(self, dt: float, *args, **kwargs) -> None:
        """Mermiyi hareket ettirir; isabet kontrolü yapar."""

    # ── Somut metotlar ─────────────────────────────────────────────────────────

    def draw(self, surface: pygame.Surface) -> None:
        """Merbiyi küçük bir renkli daire olarak çizer."""
        pygame.draw.circle(surface, self._color,
                           (int(self.x), int(self.y)), self._radius)
        # İç parlaklık
        inner_color = (
            min(255, self._color[0] + 80),
            min(255, self._color[1] + 80),
            min(255, self._color[2] + 80),
        )
        pygame.draw.circle(surface, inner_color,
                           (int(self.x), int(self.y)), max(1, self._radius - 2))

    def is_alive(self) -> bool:
        """Mermi hâlâ uçuştaysa True döner."""
        return self._alive

    # ── Getter (Kapsülleme) ────────────────────────────────────────────────────

    def get_damage(self) -> float:
        return self.__damage

    def get_speed(self) -> float:
        return self.__speed

    # ── Yardımcı hareket yardımcısı ────────────────────────────────────────────

    def _move_toward(self, target_x: float, target_y: float, dt: float) -> float:
        """
        Hedefe doğru hareket eder ve kalan mesafeyi döner.

        Args:
            target_x: Hedef x koordinatı.
            target_y: Hedef y koordinatı.
            dt:       Delta zaman (saniye).

        Returns:
            Hareket sonrası hedefe kalan mesafe.
        """
        dx = target_x - self.x
        dy = target_y - self.y
        dist = math.sqrt(dx * dx + dy * dy)
        if dist < 1e-6:
            return 0.0
        step = self.__speed * dt
        if step >= dist:
            self.x = target_x
            self.y = target_y
            return 0.0
        self.x += (dx / dist) * step
        self.y += (dy / dist) * step
        return dist - step

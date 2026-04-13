"""
Soyut düşman temel sınıfı.

OOP Rolü (Kalıtım + Kapsülleme + Soyutlama):
    Enemy, tüm düşman alt sınıflarının paylaştığı sağlık sistemi, hareket
    motoru ve çizim mantığını barındırır.  Alt sınıflar yalnızca
    'take_damage' ve 'get_type' metotlarını override eder; yol takibi ve
    can çubuğu çizimi bu sınıfta kapsüllenmiştir.
"""

from __future__ import annotations

import math
import random
from abc import abstractmethod

import pygame

from settings import TILE_SIZE, COLORS
from src.entities.game_object import GameObject


class Enemy(GameObject):
    """
    Tüm düşman varlıklarının soyut tabanı.

    Kapsülleme: sağlık, hız ve ödül değerleri '__' ile gizlenmiş;
    dışarıya yalnızca getter/setter arayüzü sunulur.
    """

    def __init__(
        self,
        hp:     int,
        speed:  float,
        reward: int,
        color:  tuple[int, int, int],
        size:   int = 20,
    ) -> None:
        super().__init__(0.0, 0.0, size * 2, size * 2)

        # Özel alanlar — kapsülleme
        self.__health     = hp
        self.__max_health = hp
        self.__speed      = speed   # karo/saniye
        self.__reward     = reward
        self.__color      = color

        # Yol takibi (korumalı — alt sınıflar erişebilir)
        self._path_index  = 1       # bir sonraki hedef ara nokta
        self._reached_end = False
        self._alive       = True
        self._radius      = size

        # Yavaşlama debuff
        self._slow_factor = 1.0
        self._slow_timer  = 0.0

        # Ölüm anında spawn edilecek varlıklar (Boss override eder)
        self._spawn_on_death_flag = False

    # ── Soyut metotlar ─────────────────────────────────────────────────────────

    @abstractmethod
    def take_damage(self, amount: float) -> None:
        """Düşmana hasar uygular."""

    @abstractmethod
    def get_type(self) -> str:
        """Düşman türünü döner: 'goblin' | 'troll' | 'boss'."""

    # ── Somut hareket motoru ───────────────────────────────────────────────────

    def update(self, dt: float, path_pixels: list[tuple[float, float]]) -> None:
        """
        Yavaşlama debuffunu günceller ve düşmanı yol boyunca hareket ettirir.

        Args:
            dt:          Delta zaman (saniye).
            path_pixels: Piksel koordinatlarında yol ara noktaları listesi.
        """
        # Yavaşlama zamanlayıcısı
        if self._slow_timer > 0:
            self._slow_timer -= dt
            if self._slow_timer <= 0:
                self._slow_timer  = 0.0
                self._slow_factor = 1.0

        self.move(path_pixels, dt)

    def move(self, path_pixels: list[tuple[float, float]], dt: float) -> None:
        """
        Waypoint (ara nokta) sistemi kullanarak düşmanı yol üzerinde hareket ettirir.

        Args:
            path_pixels: (x, y) piksel koordinatları listesi.
            dt:          Delta zaman (saniye).
        """
        if self._reached_end or not self._alive:
            return

        if self._path_index >= len(path_pixels):
            self._reached_end = True
            return

        target_x, target_y = path_pixels[self._path_index]
        dx = target_x - self.x
        dy = target_y - self.y
        dist = math.sqrt(dx * dx + dy * dy)

        # Piksel/saniye cinsinden efektif hız
        eff_speed = self.__speed * TILE_SIZE * self._slow_factor * dt

        if dist <= eff_speed:
            # Ara noktaya ulaşıldı
            self.x = target_x
            self.y = target_y
            self._path_index += 1
            if self._path_index >= len(path_pixels):
                self._reached_end = True
        else:
            self.x += (dx / dist) * eff_speed
            self.y += (dy / dist) * eff_speed

    def draw(self, surface: pygame.Surface) -> None:
        """Düşmanı ve sağlık çubuğunu çizer."""
        r = self._radius
        cx, cy = int(self.x), int(self.y)

        # Gölge
        pygame.draw.circle(surface, (0, 0, 0), (cx + 2, cy + 2), r)
        # Gövde
        draw_color = self.__color
        if self._slow_factor < 1.0:
            # Yavaşlamış düşman — buz mavisi karışımı
            draw_color = (
                min(255, draw_color[0] + 60),
                min(255, draw_color[1] + 80),
                min(255, draw_color[2] + 120),
            )
        pygame.draw.circle(surface, draw_color, (cx, cy), r)
        pygame.draw.circle(surface, (0, 0, 0), (cx, cy), r, 2)

        # Sağlık çubuğu
        bar_w  = r * 2
        bar_h  = 4
        bar_x  = cx - r
        bar_y  = cy - r - 8
        ratio  = max(0.0, self.__health / self.__max_health)

        pygame.draw.rect(surface, COLORS['health_bar_bg'],
                         (bar_x, bar_y, bar_w, bar_h))
        if ratio > 0:
            pygame.draw.rect(surface, COLORS['health_bar_fg'],
                             (bar_x, bar_y, int(bar_w * ratio), bar_h))
        pygame.draw.rect(surface, (0, 0, 0), (bar_x, bar_y, bar_w, bar_h), 1)

    def is_alive(self) -> bool:
        """Düşman hayattaysa True döner."""
        return self._alive and not self._reached_end

    # ── Debuff API ─────────────────────────────────────────────────────────────

    def apply_slow(self, factor: float, duration: float) -> None:
        """Yavaşlama debuff uygular (zayıf debuff görmezden gelinir)."""
        if factor < self._slow_factor:
            self._slow_factor = factor
        self._slow_timer = max(self._slow_timer, duration)

    # ── Getter / Setter (Kapsülleme) ───────────────────────────────────────────

    def get_health(self) -> int:
        return self.__health

    def get_max_health(self) -> int:
        return self.__max_health

    def get_speed(self) -> float:
        return self.__speed

    def get_reward(self) -> int:
        return self.__reward

    def get_color(self) -> tuple[int, int, int]:
        return self.__color

    def set_health(self, value: int) -> None:
        self.__health = max(0, value)
        if self.__health == 0:
            self._alive = False

    def has_reached_end(self) -> bool:
        """Düşman yolun sonuna ulaştıysa True döner."""
        return self._reached_end

    def get_path_progress(self) -> float:
        """
        Yol üzerindeki ilerleme skoru — hedefleme için kullanılır.
        Daha yüksek değer = yolun sonuna daha yakın.
        """
        return float(self._path_index)

    def get_spawn_on_death(self) -> list:
        """
        Düşman ölünce spawn edilecek varlıkları döner.
        Boss sınıfı bu metodu override eder.
        """
        return []

    def _apply_base_damage(self, amount: float) -> None:
        """
        Alt sınıfların çağırdığı yardımcı hasar uygulama metodu.
        Doğrudan sağlığı düşürür ve gerekirse düşmanı öldürür.
        """
        self.__health = max(0, self.__health - amount)
        if self.__health <= 0:
            self._alive = False

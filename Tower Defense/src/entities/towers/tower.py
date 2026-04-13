"""
Soyut kule temel sınıfı.

OOP Rolü (Soyutlama + Kapsülleme):
    Tower, tüm kule türlerinin paylaştığı hedefleme, ateş hızı ve
    yükseltme mekaniklerini barındırır.  'shoot' ve 'get_tower_type'
    metotları alt sınıfların uygulaması gereken sözleşmeyi tanımlar.
"""

from __future__ import annotations

import math
from abc import abstractmethod
from typing import Optional

import pygame

from settings import TILE_SIZE, COLORS, UPGRADE_DAMAGE_MULT, UPGRADE_RANGE_MULT, \
                     UPGRADE_COST_RATIO, SELL_RATIO
from src.entities.game_object import GameObject


class Tower(GameObject):
    """
    Tüm kule alt sınıflarının soyut tabanı.

    Kapsülleme: hasar, menzil, ateş hızı ve maliyet '__' ile gizlenmiş;
    upgrade() ve get_sell_value() metotları bu değerlere kontrollü erişim sağlar.
    """

    def __init__(
        self,
        x:         float,
        y:         float,
        damage:    float,
        rng:       float,
        fire_rate: float,
        cost:      int,
        color:     tuple[int, int, int],
    ) -> None:
        super().__init__(x, y, TILE_SIZE - 4, TILE_SIZE - 4)

        # Özel alanlar — kapsülleme
        self.__damage      = damage
        self.__range       = rng
        self.__fire_rate   = fire_rate   # atış/saniye
        self.__cost        = cost
        self.__color       = color

        # İzleme
        self.__upgrade_level   = 0
        self.__total_invested  = cost      # satış değeri hesabı için
        self.__shot_timer      = 0.0       # bir sonraki ateşe kalan süre
        self._target: Optional[object] = None

    # ── Soyut metotlar ─────────────────────────────────────────────────────────

    @abstractmethod
    def shoot(self, enemies: list) -> Optional[object]:
        """
        Hedefe ateş açar ve bir Projectile nesnesi döner.
        Uygun hedef yoksa None döner.
        """

    @abstractmethod
    def get_tower_type(self) -> str:
        """Kule türünü döner: 'arrow' | 'cannon' | 'ice'."""

    # ── Somut metotlar ─────────────────────────────────────────────────────────

    def update(self, dt: float, enemies: list) -> Optional[object]:
        """
        Ateş zamanlayıcısını günceller ve hazırsa ateş açar.

        Args:
            dt:      Delta zaman (saniye).
            enemies: Sahadaki düşmanlar listesi.

        Returns:
            Yeni Projectile nesnesi veya None.
        """
        self.__shot_timer += dt
        interval = 1.0 / self.__fire_rate
        if self.__shot_timer >= interval:
            self.__shot_timer -= interval
            return self.shoot(enemies)
        return None

    def find_target(self, enemies: list) -> Optional[object]:
        """
        Menzil içindeki düşmanlar arasından hedef seçer.
        Alt sınıf bu metodu override edebilir (farklı hedefleme stratejisi).

        Returns:
            Seçilen düşman veya None.
        """
        in_range = [
            e for e in enemies
            if e.is_alive() and self.distance_to(e) <= self.__range
        ]
        if not in_range:
            return None
        # Varsayılan: yolda en önde olan düşman
        return max(in_range, key=lambda e: e.get_path_progress())

    def is_alive(self) -> bool:
        """Kuleler haritadan silinmedikçe her zaman aktiftir."""
        return True

    def can_fire(self) -> bool:
        """Ateş zamanlayıcısı dolmuşsa True döner."""
        return self.__shot_timer >= 1.0 / self.__fire_rate

    def upgrade(self) -> None:
        """
        Kuleyi yükseltir:
            - Hasar  : +25%
            - Menzil : +10%
        Yükseltme maliyeti toplam yatırıma eklenir (satış değeri için).
        """
        upgrade_cost = int(self.__cost * UPGRADE_COST_RATIO)
        self.__damage   = self.__damage * UPGRADE_DAMAGE_MULT
        self.__range    = self.__range  * UPGRADE_RANGE_MULT
        self.__upgrade_level += 1
        self.__total_invested += upgrade_cost

    def get_sell_value(self) -> int:
        """Toplam yatırımın %50'sini döner."""
        return int(self.__total_invested * SELL_RATIO)

    def get_upgrade_cost(self) -> int:
        """Bir sonraki yükseltmenin maliyetini döner."""
        return int(self.__cost * UPGRADE_COST_RATIO)

    def get_upgrade_level(self) -> int:
        return self.__upgrade_level

    def draw(self, surface: pygame.Surface) -> None:
        """Kuleyi renkli dikdörtgen + karo sembolü olarak çizer."""
        cx, cy = int(self.x), int(self.y)
        hs = TILE_SIZE // 2 - 4

        # Gölge
        pygame.draw.rect(surface, (0, 0, 0),
                         (cx - hs + 2, cy - hs + 2, hs * 2, hs * 2))
        # Ana gövde
        pygame.draw.rect(surface, self.__color,
                         (cx - hs, cy - hs, hs * 2, hs * 2))
        # Kenar çizgisi
        border_c = tuple(min(255, c + 60) for c in self.__color)
        pygame.draw.rect(surface, border_c,
                         (cx - hs, cy - hs, hs * 2, hs * 2), 2)

        # Yükseltme seviyesi noktaları
        for i in range(self.__upgrade_level):
            dot_x = cx - hs + 5 + i * 8
            dot_y = cy + hs - 5
            pygame.draw.circle(surface, (255, 215, 0), (dot_x, dot_y), 3)

    def draw_range(self, surface: pygame.Surface) -> None:
        """Kulların menzil dairesini yarı saydam olarak çizer."""
        rng_surf = pygame.Surface(
            (int(self.__range * 2 + 2), int(self.__range * 2 + 2)),
            pygame.SRCALPHA
        )
        r = int(self.__range)
        pygame.draw.circle(rng_surf, (255, 255, 255, 35),
                           (r + 1, r + 1), r)
        pygame.draw.circle(rng_surf, (255, 255, 255, 80),
                           (r + 1, r + 1), r, 1)
        surface.blit(rng_surf,
                     (int(self.x) - r - 1, int(self.y) - r - 1))

    # ── Getter (Kapsülleme) ────────────────────────────────────────────────────

    def get_damage(self) -> float:
        return self.__damage

    def get_range(self) -> float:
        return self.__range

    def get_fire_rate(self) -> float:
        return self.__fire_rate

    def get_cost(self) -> int:
        return self.__cost

    def get_color(self) -> tuple:
        return self.__color

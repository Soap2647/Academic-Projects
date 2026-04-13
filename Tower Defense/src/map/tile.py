"""
Harita karesi (Tile) sınıfı.

OOP Rolü (Kapsülleme):
    Her karo kendi türünü, çizilebilirlik durumunu ve varsa üzerindeki
    kuleyi kapsüller.  GameMap bu nesneleri 2D dizi içinde yönetir.
"""

from __future__ import annotations

from enum import Enum, auto

import pygame

from settings import TILE_SIZE, COLORS


class TileType(Enum):
    """Karo türleri."""
    BUILDABLE = auto()   # Kule kurulabilir, yeşil
    PATH      = auto()   # Düşman yolu, kahverengi
    BLOCKED   = auto()   # Kullanılamaz (gelecekte genişletilebilir)


class Tile:
    """
    Harita ızgarasındaki tek bir karo.

    Kapsülleme: tile_type yalnızca kurucu tarafından atanır ve is_buildable /
    is_path property'leri aracılığıyla okunur.
    """

    def __init__(
        self,
        col:       int,
        row:       int,
        tile_type: TileType = TileType.BUILDABLE,
    ) -> None:
        """
        Args:
            col:       Izgara sütun indeksi.
            row:       Izgara satır indeksi.
            tile_type: Karonun başlangıç türü.
        """
        self.__col       = col
        self.__row       = row
        self.__tile_type = tile_type
        self.has_tower   = False    # Kule yerleştirildi mi?
        self.tower       = None     # Yerleştirilmiş kule referansı

    # ── Property'ler ───────────────────────────────────────────────────────────

    @property
    def is_buildable(self) -> bool:
        """Kule kurulabilirse True döner."""
        return self.__tile_type == TileType.BUILDABLE

    @property
    def is_path(self) -> bool:
        """Yol karosuysa True döner."""
        return self.__tile_type == TileType.PATH

    @property
    def col(self) -> int:
        return self.__col

    @property
    def row(self) -> int:
        return self.__row

    # ── Çizim ──────────────────────────────────────────────────────────────────

    def draw(self, surface: pygame.Surface, hover: bool = False) -> None:
        """
        Karoyu harita yüzeyine çizer.

        Args:
            surface: Çizim yüzeyi.
            hover:   Farenin üzerindeyse True — vurgu rengi uygular.
        """
        pixel_x = self.__col * TILE_SIZE
        pixel_y = self.__row * TILE_SIZE
        rect    = pygame.Rect(pixel_x, pixel_y, TILE_SIZE, TILE_SIZE)

        if self.__tile_type == TileType.PATH:
            # Yol — iki tonlu kahverengi desenle
            pygame.draw.rect(surface, COLORS['path'], rect)
            # Orta çizgi detayı
            if self.__col % 2 == 0:
                inner = pygame.Rect(pixel_x + 8, pixel_y + 8,
                                    TILE_SIZE - 16, TILE_SIZE - 16)
                pygame.draw.rect(surface, COLORS['path_dark'], inner)
        else:
            # Kule alanı
            base_color = COLORS['buildable_hover'] if hover else COLORS['buildable']
            pygame.draw.rect(surface, base_color, rect)
            # Izgara çizgileri
            pygame.draw.rect(surface, (20, 60, 20), rect, 1)

    def get_pixel_center(self) -> tuple[float, float]:
        """Karonun piksel merkez koordinatlarını döner."""
        return (
            self.__col * TILE_SIZE + TILE_SIZE / 2,
            self.__row * TILE_SIZE + TILE_SIZE / 2,
        )

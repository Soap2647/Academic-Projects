"""
Oyun haritası sınıfı.

OOP Rolü (Kapsülleme):
    GameMap, karo ızgarasını ve yol verilerini içsel olarak yönetir.
    Dışarıya yalnızca get_tile(), path_pixels ve is_buildable_at()
    arayüzleri sunulur.
"""

from __future__ import annotations

import pygame

from settings import (
    GRID_COLS, GRID_ROWS, TILE_SIZE,
    PATH_WAYPOINTS, COLORS, MAP_WIDTH, MAP_HEIGHT,
)
from src.map.tile import Tile, TileType


class GameMap:
    """
    20×12 karo ızgarası ve düşman yolunu yöneten harita sınıfı.

    Kapsülleme: __grid ve __path_tiles dışarıdan değiştirilemez;
    erişim yalnızca public metotlar aracılığıyla gerçekleşir.
    """

    def __init__(self) -> None:
        self.__grid: list[list[Tile]] = []
        self.__path_tiles: set[tuple[int, int]] = set()
        self.__build_path_tiles()
        self.__build_grid()
        # Piksel koordinatlarında yol ara noktaları (hareket motoru için)
        self.path_pixels: list[tuple[float, float]] = [
            (col * TILE_SIZE + TILE_SIZE / 2, row * TILE_SIZE + TILE_SIZE / 2)
            for col, row in PATH_WAYPOINTS
        ]

    # ── Başlangıç ──────────────────────────────────────────────────────────────

    def __build_path_tiles(self) -> None:
        """
        PATH_WAYPOINTS listesindeki ara noktalar arasındaki tüm yol
        karolarını hesaplar ve __path_tiles kümesine ekler.
        """
        waypoints = PATH_WAYPOINTS
        for i in range(len(waypoints) - 1):
            c0, r0 = waypoints[i]
            c1, r1 = waypoints[i + 1]
            # Yatay segment
            if r0 == r1:
                for c in range(min(c0, c1), max(c0, c1) + 1):
                    self.__path_tiles.add((c, r0))
            # Dikey segment
            elif c0 == c1:
                for r in range(min(r0, r1), max(r0, r1) + 1):
                    self.__path_tiles.add((c0, r))

    def __build_grid(self) -> None:
        """2D karo ızgarasını oluşturur."""
        self.__grid = []
        for row in range(GRID_ROWS):
            grid_row: list[Tile] = []
            for col in range(GRID_COLS):
                if (col, row) in self.__path_tiles:
                    tile = Tile(col, row, TileType.PATH)
                else:
                    tile = Tile(col, row, TileType.BUILDABLE)
                grid_row.append(tile)
            self.__grid.append(grid_row)

    # ── Public API ─────────────────────────────────────────────────────────────

    def get_tile(self, col: int, row: int) -> Tile | None:
        """
        Belirtilen koordinattaki karo nesnesini döner.

        Args:
            col: Sütun indeksi.
            row: Satır indeksi.

        Returns:
            Tile nesnesi veya koordinat ızgara dışındaysa None.
        """
        if 0 <= col < GRID_COLS and 0 <= row < GRID_ROWS:
            return self.__grid[row][col]
        return None

    def is_buildable_at(self, col: int, row: int) -> bool:
        """Belirtilen konuma kule kurulabilirse True döner."""
        tile = self.get_tile(col, row)
        if tile is None:
            return False
        return tile.is_buildable and not tile.has_tower

    def pixel_to_grid(self, px: float, py: float) -> tuple[int, int]:
        """Piksel koordinatlarını ızgara koordinatlarına çevirir."""
        return int(px // TILE_SIZE), int(py // TILE_SIZE)

    # ── Çizim ──────────────────────────────────────────────────────────────────

    def draw(
        self,
        surface:     pygame.Surface,
        hover_tile:  tuple[int, int] | None = None,
    ) -> None:
        """
        Tüm haritayı çizer.

        Args:
            surface:    Çizim yüzeyi.
            hover_tile: Farenin üzerinde olduğu (col, row) — vurgu için.
        """
        for row in range(GRID_ROWS):
            for col in range(GRID_COLS):
                tile  = self.__grid[row][col]
                is_hv = (hover_tile == (col, row)) and tile.is_buildable
                tile.draw(surface, hover=is_hv)

        # Giriş ve çıkış işaretleri
        self.__draw_entry_exit(surface)

    def __draw_entry_exit(self, surface: pygame.Surface) -> None:
        """Yolun giriş ve çıkış noktalarını işaretler."""
        font = pygame.font.SysFont(None, 18)

        # Giriş — ilk waypoint
        entry_col, entry_row = PATH_WAYPOINTS[0]
        ex = entry_col * TILE_SIZE
        ey = entry_row * TILE_SIZE + TILE_SIZE // 2 - 8
        label = font.render('GİRİŞ', True, COLORS['entry_marker'])
        surface.blit(label, (ex + 2, ey))

        # Çıkış — son waypoint
        exit_col, exit_row = PATH_WAYPOINTS[-1]
        xx = exit_col * TILE_SIZE - 2
        xy = exit_row * TILE_SIZE + TILE_SIZE // 2 - 8
        label2 = font.render('ÇIKIŞ', True, COLORS['exit_marker'])
        surface.blit(label2, (xx - label2.get_width() + TILE_SIZE, xy))

    def draw_path_arrows(self, surface: pygame.Surface) -> None:
        """Yol yönünü gösteren okları çizer (debug yardımcısı)."""
        import math
        pts = self.path_pixels
        for i in range(len(pts) - 1):
            x0, y0 = pts[i]
            x1, y1 = pts[i + 1]
            mx, my  = (x0 + x1) / 2, (y0 + y1) / 2
            angle   = math.atan2(y1 - y0, x1 - x0)
            size    = 8
            tip_x   = mx + math.cos(angle) * size
            tip_y   = my + math.sin(angle) * size
            l_x     = mx + math.cos(angle + 2.4) * (size * 0.6)
            l_y     = my + math.sin(angle + 2.4) * (size * 0.6)
            r_x     = mx + math.cos(angle - 2.4) * (size * 0.6)
            r_y     = my + math.sin(angle - 2.4) * (size * 0.6)
            pygame.draw.polygon(surface, (255, 255, 100, 120), [
                (tip_x, tip_y), (l_x, l_y), (r_x, r_y)
            ])

"""
Oyun yöneticisi — merkezi oyun döngüsü koordinatörü.

OOP Rolü (Kompozisyon + Polimorfizm):
    GameManager, tüm alt sistemleri (harita, oyuncu, dalgalar, UI)
    kompozisyon yoluyla içerir ve polimorfik entity.update() /
    entity.draw() çağrıları aracılığıyla tüm varlıkları tek tip
    yönetir.
"""

from __future__ import annotations

import math
import random
from enum import Enum, auto

import pygame

from settings import (
    TILE_SIZE, COLORS, MAP_WIDTH, WINDOW_HEIGHT, PROJECTILE_HIT_DIST,
    TOTAL_WAVES,
)
from src.map.game_map         import GameMap
from src.player.player        import Player
from src.managers.wave_manager import WaveManager
from src.managers.ui_manager  import UIManager


# ── Oyun durumu ────────────────────────────────────────────────────────────────

class GameState(Enum):
    """Oyunun ana durum makinesi."""
    WAVE_TRANSITION = auto()   # Dalga bekleniyor
    PLAYING         = auto()   # Aktif oynanış
    GAME_OVER       = auto()   # Oyuncu yenildi
    VICTORY         = auto()   # Tüm dalgalar geçildi


# ── Parçacık efekti ────────────────────────────────────────────────────────────

class Particle:
    """Ölüm animasyonu için basit parçacık efekti."""

    def __init__(
        self,
        x:       float,
        y:       float,
        vx:      float,
        vy:      float,
        color:   tuple[int, int, int],
        size:    int   = 5,
        life:    float = 0.6,
    ) -> None:
        self.x     = x
        self.y     = y
        self.vx    = vx
        self.vy    = vy
        self.color = color
        self.size  = size
        self.life  = life
        self.age   = 0.0

    def update(self, dt: float) -> None:
        self.x   += self.vx * dt
        self.y   += self.vy * dt
        self.vy  += 200 * dt   # Yerçekimi
        self.age += dt

    def is_alive(self) -> bool:
        return self.age < self.life

    def draw(self, surface: pygame.Surface) -> None:
        ratio = 1.0 - self.age / self.life
        r     = max(1, int(self.size * ratio))
        alpha = int(255 * ratio)
        s     = pygame.Surface((r * 2, r * 2), pygame.SRCALPHA)
        pygame.draw.circle(s, (*self.color, alpha), (r, r), r)
        surface.blit(s, (int(self.x) - r, int(self.y) - r))


# ── Ana yönetici sınıfı ────────────────────────────────────────────────────────

class GameManager:
    """
    Tüm oyun mantığını koordine eden merkezi yönetici sınıfı.

    Polimorfizm: enemies, towers ve projectiles listeleri farklı
    alt tipleri barındırır; tüm update/draw çağrıları uniform arayüzle yapılır.
    """

    def __init__(self, screen: pygame.Surface) -> None:
        self.screen = screen

        # Alt sistemler (Kompozisyon)
        self.game_map     = GameMap()
        self.player       = Player()
        self.wave_manager = WaveManager()
        self.ui_manager   = UIManager(self)

        # Varlık listeleri (polimorfik)
        self.enemies:     list = []
        self.towers:      list = []
        self.projectiles: list = []
        self.particles:   list[Particle] = []

        # Oyun durumu
        self.game_state   = GameState.WAVE_TRANSITION
        self.game_speed   = 1

        # Seçim durumu
        self.selected_tile:  tuple[int, int] | None = None
        self.selected_tower: object | None           = None
        self.__hovered_tile: tuple[int, int] | None  = None
        self.__hovered_tower: object | None           = None

    # ── Olay işleme ────────────────────────────────────────────────────────────

    def handle_events(self, events: list) -> None:
        """Pygame olaylarını işler."""
        for event in events:
            if event.type == pygame.MOUSEBUTTONDOWN:
                self.__handle_click(event.pos, event.button)
            elif event.type == pygame.MOUSEMOTION:
                self.__handle_motion(event.pos)
            elif event.type == pygame.KEYDOWN:
                self.__handle_key(event.key)

    def __handle_click(self, pos: tuple[int, int], button: int) -> None:
        """Fare tıklamasını işler."""
        # UI manager ilk önce değerlendirir
        if self.ui_manager.handle_click(pos, button):
            return

        if self.game_state in (GameState.GAME_OVER, GameState.VICTORY):
            return

        px, py = pos
        # Harita alanı dışındaki tıklamalar atlanır
        if px >= MAP_WIDTH or py < 50:  # 50 = üst çubuk yüksekliği
            return

        if button == 1:   # Sol tık
            # Mevcut popup/seçimi kapat
            clicked_tower = self.__get_tower_at(px, py)
            if clicked_tower:
                self.selected_tower = clicked_tower
                self.selected_tile  = None
                return

            col, row = px // TILE_SIZE, py // TILE_SIZE
            if self.game_map.is_buildable_at(col, row):
                self.selected_tile  = (col, row)
                self.selected_tower = None
            else:
                # Boş alana tıklama — seçimleri temizle
                self.selected_tile  = None
                self.selected_tower = None

        elif button == 3:   # Sağ tık
            clicked_tower = self.__get_tower_at(px, py)
            if clicked_tower:
                self.selected_tower = clicked_tower
                self.selected_tile  = None

    def __handle_motion(self, pos: tuple[int, int]) -> None:
        """Fare hareketi — hover durumlarını günceller."""
        px, py = pos
        if px < MAP_WIDTH and py >= 50:
            col, row = px // TILE_SIZE, py // TILE_SIZE
            self.__hovered_tile = (col, row)
            self.__hovered_tower = self.__get_tower_at(px, py)
        else:
            self.__hovered_tile  = None
            self.__hovered_tower = None

    def __handle_key(self, key: int) -> None:
        """Klavye tuşlarını işler."""
        if key == pygame.K_ESCAPE:
            self.selected_tile  = None
            self.selected_tower = None
        elif key == pygame.K_SPACE:
            if self.game_state == GameState.WAVE_TRANSITION:
                self.start_wave()
        elif key == pygame.K_F1:
            self.toggle_speed()

    # ── Dalga kontrolü ─────────────────────────────────────────────────────────

    def start_wave(self) -> None:
        """Bir sonraki dalgayı başlatır."""
        if self.wave_manager.start_next_wave():
            self.game_state = GameState.PLAYING
            self.player.set_wave_reached(self.wave_manager.get_current_wave())

    def toggle_speed(self) -> None:
        """Oyun hızını 1x ↔ 2x arasında değiştirir."""
        self.game_speed = 2 if self.game_speed == 1 else 1

    # ── Güncelleme döngüsü ─────────────────────────────────────────────────────

    def update(self, dt: float) -> None:
        """Tüm oyun varlıklarını ve mantığını günceller."""
        self.ui_manager.update(dt)

        if self.game_state != GameState.PLAYING:
            return

        eff_dt = dt * self.game_speed

        # Dalga yöneticisi — yeni düşmanlar doğur
        new_enemies = self.wave_manager.update(eff_dt)
        for e in new_enemies:
            self.enemies.append(e)

        # Düşmanları güncelle
        self.__update_enemies(eff_dt)

        # Kuleleri güncelle — ateş açar ve mermi döner
        self.__update_towers(eff_dt)

        # Mermileri güncelle — hareket + isabet kontrolü
        self.__update_projectiles(eff_dt)

        # Parçacıkları güncelle
        for p in self.particles[:]:
            p.update(eff_dt)
            if not p.is_alive():
                self.particles.remove(p)

        # Oyun sonu koşulları
        self.__check_end_conditions()

    def __update_enemies(self, dt: float) -> None:
        """Düşmanları hareket ettirir ve ölü/biten düşmanları temizler."""
        path = self.game_map.path_pixels
        for enemy in self.enemies[:]:
            enemy.update(dt, path)

            if enemy.has_reached_end():
                self.player.lose_life()
                self.enemies.remove(enemy)

            elif not enemy.is_alive():
                self.__on_enemy_death(enemy)
                self.enemies.remove(enemy)

    def __update_towers(self, dt: float) -> None:
        """Kuleleri günceller; yeni mermileri listeye ekler."""
        for tower in self.towers:
            # Polimorfik çağrı — tower.update() doğru shoot() metodunu çağırır
            new_proj = tower.update(dt, self.enemies)
            if new_proj is not None:
                self.projectiles.append(new_proj)

    def __update_projectiles(self, dt: float) -> None:
        """Mermileri günceller ve isabet kontrolü yapar."""
        for proj in self.projectiles[:]:
            proj.update(dt)

            if not proj.is_alive():
                # Mermi hedefe ulaştı veya kayboldu — on_hit() çağır
                hits = proj.on_hit(self.enemies)
                for enemy, dmg in hits:
                    if enemy.is_alive():
                        enemy.take_damage(dmg)
                        self.__spawn_damage_text(dmg, enemy.x, enemy.y)
                        if not enemy.is_alive():
                            self.__on_enemy_death(enemy)
                            if enemy in self.enemies:
                                self.enemies.remove(enemy)

                self.projectiles.remove(proj)

    # ── Olaylar ────────────────────────────────────────────────────────────────

    def __on_enemy_death(self, enemy) -> None:
        """Düşman ölümünü işler: altın, parçacıklar, boss spawn."""
        self.player.earn_gold(enemy.get_reward())
        self.player.update_score()

        # Altın kazanım metni
        self.ui_manager.add_floating_text(
            f'+{enemy.get_reward()}💰',
            enemy.x, enemy.y - 20,
            COLORS['gold_text'],
        )

        # Ölüm parçacıkları (8 adet)
        for _ in range(8):
            angle = random.uniform(0, 2 * math.pi)
            speed = random.uniform(60, 160)
            self.particles.append(Particle(
                x     = enemy.x,
                y     = enemy.y,
                vx    = math.cos(angle) * speed,
                vy    = math.sin(angle) * speed - 40,
                color = enemy.get_color(),
                size  = random.randint(3, 7),
                life  = random.uniform(0.4, 0.8),
            ))

        # Boss ölüm spawnu
        spawn_list = enemy.get_spawn_on_death()
        for new_e in spawn_list:
            self.enemies.append(new_e)

    def __spawn_damage_text(self, dmg: float, x: float, y: float) -> None:
        """Hasar miktarını yüzen metin olarak gösterir."""
        self.ui_manager.add_floating_text(
            f'-{int(dmg)}',
            x + random.randint(-10, 10),
            y - 10,
            COLORS['damage_text'],
        )

    def __check_end_conditions(self) -> None:
        """Oyun sonu koşullarını kontrol eder."""
        if self.player.is_defeated():
            self.game_state = GameState.GAME_OVER
            return

        # Tüm düşmanlar yok edildi mi?
        wave_done = (
            self.wave_manager.current_wave_spawning_done() and
            len(self.enemies) == 0
        )
        if wave_done:
            if self.wave_manager.all_waves_complete() or \
               self.wave_manager.get_current_wave() >= TOTAL_WAVES:
                self.game_state = GameState.VICTORY
            else:
                self.game_state = GameState.WAVE_TRANSITION

    # ── Kule yönetimi ──────────────────────────────────────────────────────────

    def place_tower(self, tower_type: str) -> None:
        """
        Seçili karoye belirtilen türde kule yerleştirir.

        Fabrika deseni: tower_type string değerinden somut Tower sınıfı üretilir.
        """
        if self.selected_tile is None:
            return
        from settings import TOWER_DATA
        cost = TOWER_DATA[tower_type]['cost']
        if not self.player.spend_gold(cost):
            return

        col, row = self.selected_tile
        tile     = self.game_map.get_tile(col, row)
        if tile is None or not tile.is_buildable or tile.has_tower:
            self.player.earn_gold(cost)   # iade
            return

        tower = self.__create_tower(tower_type, col, row)
        self.towers.append(tower)
        tile.has_tower = True
        tile.tower     = tower
        self.selected_tile  = None

    def upgrade_tower(self, tower) -> None:
        """Seçili kuleyi yükseltir."""
        cost = tower.get_upgrade_cost()
        if self.player.spend_gold(cost):
            tower.upgrade()
            self.selected_tower = None

    def sell_tower(self, tower) -> None:
        """Kuleyi satar ve değerini oyuncuya öder."""
        sell_val = tower.get_sell_value()
        self.player.earn_gold(sell_val)

        # Haritadaki karo durumunu sıfırla
        col = int(tower.x // TILE_SIZE)
        row = int(tower.y // TILE_SIZE)
        tile = self.game_map.get_tile(col, row)
        if tile:
            tile.has_tower = False
            tile.tower     = None

        if tower in self.towers:
            self.towers.remove(tower)
        self.selected_tower = None

    def __create_tower(self, tower_type: str, col: int, row: int):
        """Kule fabrika metodu."""
        from src.entities.towers.arrow_tower  import ArrowTower
        from src.entities.towers.cannon_tower import CannonTower
        from src.entities.towers.ice_tower    import IceTower

        x = col * TILE_SIZE + TILE_SIZE / 2
        y = row * TILE_SIZE + TILE_SIZE / 2

        factory = {
            'arrow':  ArrowTower,
            'cannon': CannonTower,
            'ice':    IceTower,
        }
        cls = factory.get(tower_type)
        if cls is None:
            raise ValueError(f"Bilinmeyen kule tipi: {tower_type!r}")
        return cls(x, y)

    # ── Çizim döngüsü ──────────────────────────────────────────────────────────

    def draw(self, surface: pygame.Surface) -> None:
        """Tüm oyun varlıklarını ve UI'ı çizer."""
        surface.fill(COLORS['background'])

        # Harita
        hover = self.__hovered_tile if self.selected_tile is None else None
        self.game_map.draw(surface, hover_tile=hover)

        # Kulelerin menzil daireleri (hover üzerindekiler)
        if self.__hovered_tower:
            self.__hovered_tower.draw_range(surface)
        if self.selected_tower:
            self.selected_tower.draw_range(surface)

        # Kuleler (polimorfik draw)
        for tower in self.towers:
            tower.draw(surface)

        # Seçili karonun vurgusu
        if self.selected_tile:
            col, row = self.selected_tile
            hl = pygame.Surface((TILE_SIZE, TILE_SIZE), pygame.SRCALPHA)
            hl.fill((255, 255, 100, 80))
            surface.blit(hl, (col * TILE_SIZE, row * TILE_SIZE))

        # Düşmanlar (polimorfik draw)
        for enemy in self.enemies:
            enemy.draw(surface)

        # Mermiler (polimorfik draw)
        for proj in self.projectiles:
            proj.draw(surface)

        # Parçacıklar
        for particle in self.particles:
            particle.draw(surface)

        # UI
        self.ui_manager.draw(surface)

        # Overlay ekranları
        if self.game_state == GameState.GAME_OVER:
            self.ui_manager.draw_game_over(surface)
        elif self.game_state == GameState.VICTORY:
            self.ui_manager.draw_victory(surface)

    # ── Yardımcılar ────────────────────────────────────────────────────────────

    def __get_tower_at(self, px: float, py: float) -> object | None:
        """Verilen piksel koordinatında kule varsa döner."""
        for tower in self.towers:
            if tower.distance_to_point(px, py) <= TILE_SIZE / 2:
                return tower
        return None

    def restart(self) -> None:
        """Oyunu tamamen sıfırlar."""
        self.enemies.clear()
        self.towers.clear()
        self.projectiles.clear()
        self.particles.clear()
        self.ui_manager.floating_texts.clear()

        self.game_map     = GameMap()
        self.player.reset()
        self.wave_manager = WaveManager()

        self.game_state   = GameState.WAVE_TRANSITION
        self.game_speed   = 1
        self.selected_tile  = None
        self.selected_tower = None

"""
Kullanıcı arayüzü yöneticisi.

OOP Rolü (Yönetici / Manager):
    UIManager, oyun HUD'unu, kule seçim popup'larını, yükseltme/satış
    panelini ve tüm UI etkileşimlerini kapsüller.  GameManager referansı
    aracılığıyla oyun durumuna erişir.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

import pygame

from settings import (
    WINDOW_WIDTH, WINDOW_HEIGHT, MAP_WIDTH, UI_PANEL_WIDTH,
    TILE_SIZE, TOWER_DATA, COLORS, TOTAL_WAVES,
)

if TYPE_CHECKING:
    from src.managers.game_manager import GameManager


# ── Yardımcı yardımcı veri sınıfı ─────────────────────────────────────────────

class FloatingText:
    """Hasar veya altın kazanımını gösteren yüzer metin."""

    def __init__(
        self,
        text:     str,
        x:        float,
        y:        float,
        color:    tuple[int, int, int],
        lifetime: float = 1.2,
    ) -> None:
        self.text     = text
        self.x        = x
        self.y        = y
        self.color    = color
        self.lifetime = lifetime
        self.age      = 0.0

    def update(self, dt: float) -> None:
        self.age += dt
        self.y   -= 30 * dt   # Yukarı doğru hareket

    def is_alive(self) -> bool:
        return self.age < self.lifetime

    def draw(self, surface: pygame.Surface, font: pygame.font.Font) -> None:
        alpha  = max(0, int(255 * (1.0 - self.age / self.lifetime)))
        txt    = font.render(self.text, True, self.color)
        txt.set_alpha(alpha)
        surface.blit(txt, (int(self.x) - txt.get_width() // 2, int(self.y)))


# ── Ana UI yöneticisi ──────────────────────────────────────────────────────────

class UIManager:
    """
    Tüm UI katmanlarını yöneten sınıf.

    Kapsülleme: popup durumu, seçili karo/kule ve UI Rect'leri
    dışarıdan doğrudan değiştirilemez.
    """

    def __init__(self, game_manager: "GameManager") -> None:
        self.__gm = game_manager
        pygame.font.init()
        self.__font_large  = pygame.font.SysFont(None, 36)
        self.__font_medium = pygame.font.SysFont(None, 26)
        self.__font_small  = pygame.font.SysFont(None, 20)
        self.__font_tiny   = pygame.font.SysFont(None, 16)

        # Sağ panel başlangıç x koordinatı
        self.__panel_x = MAP_WIDTH

        # Kule buton dikdörtgenleri (sağ panelde)
        self.__tower_btns: dict[str, pygame.Rect] = {}
        self.__build_panel_rects()

        # Dalga başlat butonu
        self.__wave_btn_rect = pygame.Rect(
            self.__panel_x + 10, WINDOW_HEIGHT - 70, UI_PANEL_WIDTH - 20, 50
        )
        # Hız butonu
        self.__speed_btn_rect = pygame.Rect(
            self.__panel_x + 10, WINDOW_HEIGHT - 130, UI_PANEL_WIDTH - 20, 45
        )

        # Yüzen metinler
        self.floating_texts: list[FloatingText] = []

    # ── Rect kurulumu ──────────────────────────────────────────────────────────

    def __build_panel_rects(self) -> None:
        """Sağ paneldeki kule satın alma düğmelerinin dikdörtgenlerini oluşturur."""
        btn_h  = 80
        btn_gap = 8
        start_y = 220
        types   = ['arrow', 'cannon', 'ice']
        for i, ttype in enumerate(types):
            y = start_y + i * (btn_h + btn_gap)
            self.__tower_btns[ttype] = pygame.Rect(
                self.__panel_x + 8, y, UI_PANEL_WIDTH - 16, btn_h
            )

    # ── Güncelleme ─────────────────────────────────────────────────────────────

    def update(self, dt: float) -> None:
        """Yüzen metinleri günceller."""
        for ft in self.floating_texts[:]:
            ft.update(dt)
            if not ft.is_alive():
                self.floating_texts.remove(ft)

    # ── Ana çizim ──────────────────────────────────────────────────────────────

    def draw(self, surface: pygame.Surface) -> None:
        """Tüm UI katmanlarını çizer."""
        self.__draw_panel_bg(surface)
        self.__draw_top_bar(surface)
        self.__draw_tower_shop(surface)
        self.__draw_speed_btn(surface)
        self.__draw_wave_btn(surface)
        self.__draw_selected_tower_panel(surface)
        self.__draw_tower_placement_popup(surface)
        self.__draw_floating_texts(surface)

    # ── UI katmanları ──────────────────────────────────────────────────────────

    def __draw_panel_bg(self, surface: pygame.Surface) -> None:
        """Sağ UI panelinin arka planını çizer."""
        panel_rect = pygame.Rect(self.__panel_x, 0, UI_PANEL_WIDTH, WINDOW_HEIGHT)
        pygame.draw.rect(surface, COLORS['ui_bg'], panel_rect)
        pygame.draw.line(surface, COLORS['ui_border'],
                         (self.__panel_x, 0), (self.__panel_x, WINDOW_HEIGHT), 2)

    def __draw_top_bar(self, surface: pygame.Surface) -> None:
        """Üst bilgi çubuğunu (can, altın, dalga, skor) çizer."""
        gm      = self.__gm
        player  = gm.player
        bar_h   = 50

        pygame.draw.rect(surface, (15, 15, 35), (0, 0, MAP_WIDTH, bar_h))
        pygame.draw.line(surface, COLORS['ui_border'],
                         (0, bar_h), (MAP_WIDTH, bar_h), 1)

        # Can
        lives_txt = self.__font_medium.render(
            f'❤ {player.get_lives()}', True, COLORS['lives']
        )
        surface.blit(lives_txt, (10, 14))

        # Altın
        gold_txt = self.__font_medium.render(
            f'💰 {player.get_gold()}', True, COLORS['gold']
        )
        surface.blit(gold_txt, (140, 14))

        # Dalga
        wave_txt = self.__font_medium.render(
            f'Dalga {gm.wave_manager.get_current_wave()}/{TOTAL_WAVES}',
            True, COLORS['text']
        )
        surface.blit(wave_txt, (MAP_WIDTH // 2 - wave_txt.get_width() // 2, 14))

        # Skor
        score_txt = self.__font_medium.render(
            f'Skor: {player.get_score()}', True, (200, 200, 255)
        )
        surface.blit(score_txt, (MAP_WIDTH - score_txt.get_width() - 10, 14))

    def __draw_tower_shop(self, surface: pygame.Surface) -> None:
        """Sağ panelde kule mağazasını çizer."""
        title = self.__font_medium.render('KULE MAĞAZASI', True, COLORS['text'])
        surface.blit(title, (self.__panel_x + UI_PANEL_WIDTH // 2 - title.get_width() // 2, 10))
        pygame.draw.line(surface, COLORS['ui_border'],
                         (self.__panel_x + 8, 40), (WINDOW_WIDTH - 8, 40), 1)

        # Alt başlık
        hint = self.__font_tiny.render('Yeşil karoya tıkla → kule kur', True, (160, 160, 160))
        surface.blit(hint, (self.__panel_x + 8, 50))

        player = self.__gm.player
        for ttype, rect in self.__tower_btns.items():
            data       = TOWER_DATA[ttype]
            can_afford = player.can_afford(data['cost'])
            btn_color  = (40, 40, 70) if can_afford else (50, 30, 30)
            border_c   = data['color'] if can_afford else (100, 60, 60)

            pygame.draw.rect(surface, btn_color, rect, border_radius=6)
            pygame.draw.rect(surface, border_c,  rect, 2, border_radius=6)

            # Kule simgesi
            icon_x = rect.x + 16
            icon_y = rect.centery
            pygame.draw.rect(surface, data['color'], (icon_x, icon_y - 12, 24, 24))
            pygame.draw.rect(surface, (0, 0, 0), (icon_x, icon_y - 12, 24, 24), 1)

            # Kule adı
            name_txt = self.__font_small.render(data['name'], True, COLORS['text'])
            surface.blit(name_txt, (rect.x + 48, rect.y + 8))

            # Maliyet
            cost_c   = COLORS['gold'] if can_afford else (180, 120, 120)
            cost_txt = self.__font_small.render(f"{data['cost']} 💰", True, cost_c)
            surface.blit(cost_txt, (rect.x + 48, rect.y + 28))

            # Açıklama (ilk satır)
            desc_line = data['desc'].split('\n')[0]
            desc_txt  = self.__font_tiny.render(desc_line, True, (180, 180, 180))
            surface.blit(desc_txt, (rect.x + 48, rect.y + 50))

    def __draw_speed_btn(self, surface: pygame.Surface) -> None:
        """Oyun hızı değiştirme düğmesini çizer."""
        speed     = self.__gm.game_speed
        label     = f'Hız: {speed}x  →  {"2x" if speed == 1 else "1x"}'
        btn_color = (60, 60, 100) if speed == 1 else (100, 60, 60)
        pygame.draw.rect(surface, btn_color, self.__speed_btn_rect, border_radius=6)
        pygame.draw.rect(surface, COLORS['ui_border'], self.__speed_btn_rect, 1, border_radius=6)
        txt = self.__font_small.render(label, True, COLORS['text'])
        surface.blit(txt, (
            self.__speed_btn_rect.centerx - txt.get_width() // 2,
            self.__speed_btn_rect.centery - txt.get_height() // 2,
        ))

    def __draw_wave_btn(self, surface: pygame.Surface) -> None:
        """Dalga başlat düğmesini çizer."""
        from src.managers.game_manager import GameState
        gm = self.__gm
        if gm.game_state != GameState.WAVE_TRANSITION:
            return

        wave_num = gm.wave_manager.get_current_wave() + 1
        if wave_num > TOTAL_WAVES:
            return

        mouse_pos = pygame.mouse.get_pos()
        hovering  = self.__wave_btn_rect.collidepoint(mouse_pos)
        btn_color = COLORS['wave_btn_hover'] if hovering else COLORS['wave_btn']

        pygame.draw.rect(surface, btn_color, self.__wave_btn_rect, border_radius=8)
        pygame.draw.rect(surface, (0, 150, 0), self.__wave_btn_rect, 2, border_radius=8)

        label = f'► Dalga {wave_num} Başlat'
        txt   = self.__font_medium.render(label, True, (255, 255, 255))
        surface.blit(txt, (
            self.__wave_btn_rect.centerx - txt.get_width() // 2,
            self.__wave_btn_rect.centery - txt.get_height() // 2,
        ))

    def __draw_selected_tower_panel(self, surface: pygame.Surface) -> None:
        """Seçili kulenin yükseltme/satış panelini çizer."""
        tower = self.__gm.selected_tower
        if tower is None:
            return

        panel = pygame.Rect(self.__panel_x + 8, 460, UI_PANEL_WIDTH - 16, 180)
        pygame.draw.rect(surface, COLORS['popup_bg'], panel, border_radius=8)
        pygame.draw.rect(surface, COLORS['popup_border'], panel, 2, border_radius=8)

        # Başlık
        tdata = TOWER_DATA.get(tower.get_tower_type(), {})
        title = self.__font_medium.render(tdata.get('name', '?'), True, COLORS['text'])
        surface.blit(title, (panel.x + 8, panel.y + 8))

        # Seviye
        lvl_txt = self.__font_small.render(
            f'Seviye: {tower.get_upgrade_level() + 1}', True, (200, 200, 255)
        )
        surface.blit(lvl_txt, (panel.x + 8, panel.y + 36))

        # Stat satırları
        stats = [
            f'Hasar: {tower.get_damage():.0f}',
            f'Menzil: {tower.get_range():.0f}',
            f'Ateş hızı: {tower.get_fire_rate():.1f}/sn',
        ]
        for i, stat in enumerate(stats):
            s_txt = self.__font_tiny.render(stat, True, (180, 200, 180))
            surface.blit(s_txt, (panel.x + 8, panel.y + 58 + i * 16))

        # Yükselt butonu
        up_cost  = tower.get_upgrade_cost()
        can_up   = self.__gm.player.can_afford(up_cost)
        up_rect  = pygame.Rect(panel.x + 8, panel.y + 115, (panel.width - 20) // 2, 32)
        up_color = COLORS['upgrade_btn'] if can_up else (40, 40, 80)
        pygame.draw.rect(surface, up_color, up_rect, border_radius=5)
        up_txt   = self.__font_tiny.render(f'↑ {up_cost}💰', True, COLORS['text'])
        surface.blit(up_txt, (up_rect.centerx - up_txt.get_width() // 2,
                               up_rect.centery - up_txt.get_height() // 2))

        # Sat butonu
        sell_val  = tower.get_sell_value()
        sell_rect = pygame.Rect(
            panel.x + 12 + (panel.width - 20) // 2, panel.y + 115,
            (panel.width - 20) // 2, 32
        )
        pygame.draw.rect(surface, COLORS['sell_btn'], sell_rect, border_radius=5)
        sell_txt  = self.__font_tiny.render(f'Sat +{sell_val}💰', True, COLORS['text'])
        surface.blit(sell_txt, (sell_rect.centerx - sell_txt.get_width() // 2,
                                  sell_rect.centery - sell_txt.get_height() // 2))

        # Rect'leri sakla (tıklama kontrolü için)
        self.__upgrade_rect = up_rect
        self.__sell_rect    = sell_rect

    def __draw_tower_placement_popup(self, surface: pygame.Surface) -> None:
        """Seçili karo üzerinde kule seçim popup'ını çizer."""
        if self.__gm.selected_tile is None:
            return
        col, row = self.__gm.selected_tile
        px = col * TILE_SIZE
        py = row * TILE_SIZE

        popup_w = 180
        popup_h = 160
        # Popup konumunu harita içinde tut
        pop_x = min(px + TILE_SIZE, MAP_WIDTH - popup_w - 2)
        pop_y = max(0, min(py - popup_h // 2, WINDOW_HEIGHT - popup_h - 2))

        popup_rect = pygame.Rect(pop_x, pop_y, popup_w, popup_h)
        pygame.draw.rect(surface, COLORS['popup_bg'], popup_rect, border_radius=8)
        pygame.draw.rect(surface, COLORS['popup_border'], popup_rect, 2, border_radius=8)

        title = self.__font_small.render('Kule Seç:', True, COLORS['text'])
        surface.blit(title, (pop_x + 8, pop_y + 6))

        self.__placement_btns: dict[str, pygame.Rect] = {}
        player = self.__gm.player
        for i, ttype in enumerate(['arrow', 'cannon', 'ice']):
            data      = TOWER_DATA[ttype]
            btn_rect  = pygame.Rect(pop_x + 6, pop_y + 28 + i * 42, popup_w - 12, 36)
            can_aff   = player.can_afford(data['cost'])
            b_color   = (50, 70, 50) if can_aff else (60, 40, 40)
            border_c  = data['color'] if can_aff else (100, 70, 70)

            pygame.draw.rect(surface, b_color,   btn_rect, border_radius=5)
            pygame.draw.rect(surface, border_c,  btn_rect, 1, border_radius=5)

            # Simge
            pygame.draw.rect(surface, data['color'],
                             (btn_rect.x + 5, btn_rect.centery - 10, 20, 20))

            # İsim + maliyet
            lbl = self.__font_tiny.render(
                f"{data['name']}  {data['cost']}💰", True,
                COLORS['text'] if can_aff else (150, 130, 130)
            )
            surface.blit(lbl, (btn_rect.x + 32, btn_rect.centery - lbl.get_height() // 2))
            self.__placement_btns[ttype] = btn_rect

    def __draw_floating_texts(self, surface: pygame.Surface) -> None:
        """Yüzen hasar/altın metinlerini çizer."""
        for ft in self.floating_texts:
            ft.draw(surface, self.__font_small)

    # ── Overlay ekranları ──────────────────────────────────────────────────────

    def draw_game_over(self, surface: pygame.Surface) -> None:
        """Oyun bitti ekranını çizer."""
        self.__draw_overlay(surface, (180, 20, 20, 180))
        cx, cy = MAP_WIDTH // 2, WINDOW_HEIGHT // 2

        title = self.__font_large.render('OYUN BİTTİ', True, (255, 80, 80))
        self.__shadow_blit(surface, title, cx - title.get_width() // 2, cy - 80)

        score = self.__gm.player.get_score()
        s_txt = self.__font_medium.render(f'Final Skor: {score}', True, COLORS['gold'])
        self.__shadow_blit(surface, s_txt, cx - s_txt.get_width() // 2, cy - 30)

        self.__restart_btn_rect = pygame.Rect(cx - 100, cy + 20, 200, 50)
        pygame.draw.rect(surface, (200, 50, 50), self.__restart_btn_rect, border_radius=10)
        r_txt = self.__font_medium.render('Yeniden Başla', True, COLORS['text'])
        surface.blit(r_txt, (cx - r_txt.get_width() // 2, cy + 35))

    def draw_victory(self, surface: pygame.Surface) -> None:
        """Zafer ekranını çizer."""
        self.__draw_overlay(surface, (20, 100, 20, 180))
        cx, cy = MAP_WIDTH // 2, WINDOW_HEIGHT // 2

        title = self.__font_large.render('ZAFERSİN!', True, (100, 255, 100))
        self.__shadow_blit(surface, title, cx - title.get_width() // 2, cy - 80)

        score = self.__gm.player.get_score()
        s_txt = self.__font_medium.render(f'Final Skor: {score}', True, COLORS['gold'])
        self.__shadow_blit(surface, s_txt, cx - s_txt.get_width() // 2, cy - 30)

        self.__restart_btn_rect = pygame.Rect(cx - 100, cy + 20, 200, 50)
        pygame.draw.rect(surface, (50, 150, 50), self.__restart_btn_rect, border_radius=10)
        r_txt = self.__font_medium.render('Tekrar Oyna', True, COLORS['text'])
        surface.blit(r_txt, (cx - r_txt.get_width() // 2, cy + 35))

    def draw_wave_countdown(self, surface: pygame.Surface, seconds_left: float) -> None:
        """Dalga başlamadan önce geri sayım sayısını haritanın ortasına çizer."""
        cx, cy = MAP_WIDTH // 2, WINDOW_HEIGHT // 2
        txt = self.__font_large.render(f'Hazır ol! {int(seconds_left) + 1}', True, (255, 220, 50))
        surface.blit(txt, (cx - txt.get_width() // 2, cy - txt.get_height() // 2))

    # ── Tıklama işleme ─────────────────────────────────────────────────────────

    def handle_click(self, pos: tuple[int, int], button: int) -> bool:
        """
        UI üzerindeki tıklamaları işler.

        Returns:
            Tıklama UI tarafından tüketildiyse True.
        """
        from src.managers.game_manager import GameState

        # Overlay düğmesi — Game Over / Victory
        if self.__gm.game_state in (GameState.GAME_OVER, GameState.VICTORY):
            if hasattr(self, '_UIManager__restart_btn_rect') and \
               self.__restart_btn_rect.collidepoint(pos):
                self.__gm.restart()
                return True
            return False

        # Sağ panelin dışındaysa — harita tıklaması
        if pos[0] < MAP_WIDTH:
            return self.__handle_map_click(pos, button)

        # Sağ paneli içindeyse
        return self.__handle_panel_click(pos, button)

    def __handle_map_click(self, pos: tuple[int, int], button: int) -> bool:
        """Harita üzerindeki tıklamaları işler."""
        # Kule yerleştirme popup butonu tıklandı mı?
        if button == 1 and self.__gm.selected_tile is not None:
            if hasattr(self, '_UIManager__placement_btns'):
                for ttype, rect in self.__placement_btns.items():
                    if rect.collidepoint(pos):
                        self.__gm.place_tower(ttype)
                        return True

        if button == 1:
            # Seçili kule paneli butonları
            if self.__gm.selected_tower is not None:
                if hasattr(self, '_UIManager__upgrade_rect') and \
                   self.__upgrade_rect.collidepoint(pos):
                    self.__gm.upgrade_tower(self.__gm.selected_tower)
                    return True
                if hasattr(self, '_UIManager__sell_rect') and \
                   self.__sell_rect.collidepoint(pos):
                    self.__gm.sell_tower(self.__gm.selected_tower)
                    return True

        return False

    def __handle_panel_click(self, pos: tuple[int, int], button: int) -> bool:
        """Sağ panel tıklamalarını işler."""
        from src.managers.game_manager import GameState

        if button == 1:
            # Dalga başlat
            if self.__wave_btn_rect.collidepoint(pos):
                if self.__gm.game_state == GameState.WAVE_TRANSITION:
                    self.__gm.start_wave()
                return True

            # Hız değiştir
            if self.__speed_btn_rect.collidepoint(pos):
                self.__gm.toggle_speed()
                return True

            # Seçili kule paneli butonları (sağ panel üzerinde gösterildiğinde)
            if self.__gm.selected_tower is not None:
                if hasattr(self, '_UIManager__upgrade_rect') and \
                   self.__upgrade_rect.collidepoint(pos):
                    self.__gm.upgrade_tower(self.__gm.selected_tower)
                    return True
                if hasattr(self, '_UIManager__sell_rect') and \
                   self.__sell_rect.collidepoint(pos):
                    self.__gm.sell_tower(self.__gm.selected_tower)
                    return True

        return False

    # ── Yardımcılar ────────────────────────────────────────────────────────────

    def __draw_overlay(
        self,
        surface: pygame.Surface,
        color:   tuple[int, int, int, int],
    ) -> None:
        """Harita üzerine yarı saydam overlay çizer."""
        overlay = pygame.Surface((MAP_WIDTH, WINDOW_HEIGHT), pygame.SRCALPHA)
        overlay.fill(color)
        surface.blit(overlay, (0, 0))

    def __shadow_blit(
        self,
        surface: pygame.Surface,
        txt_surf: pygame.Surface,
        x: int,
        y: int,
    ) -> None:
        """Gölgeli metin çizer."""
        shadow = self.__font_large.render(
            txt_surf.get_rect().size.__str__(), True, (0, 0, 0)
        )
        # Gölge ofset
        shadow_surf = pygame.Surface(txt_surf.get_size(), pygame.SRCALPHA)
        shadow_surf.fill((0, 0, 0, 150))
        surface.blit(shadow_surf, (x + 2, y + 2))
        surface.blit(txt_surf, (x, y))

    def add_floating_text(
        self,
        text:  str,
        x:     float,
        y:     float,
        color: tuple[int, int, int],
    ) -> None:
        """Yeni yüzen metin nesnesi ekler."""
        self.floating_texts.append(FloatingText(text, x, y, color))

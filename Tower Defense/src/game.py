"""
Ana oyun sarmalayıcı sınıfı.

OOP Rolü (Kapsülleme):
    Game sınıfı, pygame başlatma, ana döngü ve ekran yönetimini
    GameManager'dan ayırarak tek sorumluluk prensibini uygular.
"""

from __future__ import annotations

import pygame

from settings import WINDOW_WIDTH, WINDOW_HEIGHT, FPS
from src.managers.game_manager import GameManager


class Game:
    """
    Pygame başlatma ve ana oyun döngüsünü yöneten sarmalayıcı sınıf.

    Kapsülleme: pygame ekran, saat ve çalışma bayrağı dışarıdan
    erişilemez; yalnızca run() metodu aracılığıyla oyun başlatılır.
    """

    def __init__(self) -> None:
        pygame.init()
        pygame.display.set_caption('Tower Defense — v1.0')

        self.__screen  = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
        self.__clock   = pygame.time.Clock()
        self.__running = True
        self.__gm      = GameManager(self.__screen)

    def run(self) -> None:
        """Ana oyun döngüsünü başlatır ve oyun kapanana dek çalıştırır."""
        while self.__running:
            dt     = self.__clock.tick(FPS) / 1000.0   # Saniye cinsinden delta zaman
            events = pygame.event.get()

            for event in events:
                if event.type == pygame.QUIT:
                    self.__running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_F4 and \
                       pygame.key.get_mods() & pygame.KMOD_ALT:
                        self.__running = False

            self.__gm.handle_events(events)
            self.__gm.update(dt)
            self.__gm.draw(self.__screen)
            pygame.display.flip()

        pygame.quit()

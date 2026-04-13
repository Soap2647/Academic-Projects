"""
Tower Defense — Giriş Noktası

Kullanım:
    python main.py

Gereksinimler:
    pip install pygame

OOP Mimari Özeti:
    GameObject (ABC)
    ├── Enemy (ABC)  →  Goblin, Troll, Boss
    ├── Tower (ABC)  →  ArrowTower, CannonTower, IceTower
    └── Projectile (ABC)  →  Arrow, Cannonball, IceBlast

    Yöneticiler: GameManager, WaveManager, UIManager
    Destek: GameMap, Player
"""

import sys
import os

# Proje kökünü Python yoluna ekle — her çalışma dizininden çalışabilirlik
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from src.game import Game


def main() -> None:
    """Oyunu başlatır."""
    game = Game()
    game.run()


if __name__ == '__main__':
    main()

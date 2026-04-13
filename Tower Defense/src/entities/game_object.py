"""
Soyut temel sınıf — tüm oyun varlıkları bu sınıftan türer.

OOP Rolü (Soyutlama / Abstraction):
    GameObject, 'update', 'draw' ve 'is_alive' davranışlarını zorunlu kılan
    soyut bir kontrat tanımlar. Alt sınıflar bu metotları uygulamak zorundadır;
    böylece GameManager tüm varlıkları tek tip arayüzle yönetebilir.
"""

from abc import ABC, abstractmethod
import pygame


class GameObject(ABC):
    """
    Tüm oyun varlıklarının (düşman, kule, mermi) soyut temel sınıfı.

    Kapsülleme: x, y, width, height alanları doğrudan erişilebilir tutulur
    çünkü bunlar salt geometri verisidir; iş mantığı içermez.
    """

    def __init__(self, x: float, y: float, width: int, height: int) -> None:
        """
        Args:
            x:      Varlığın merkez x koordinatı (piksel).
            y:      Varlığın merkez y koordinatı (piksel).
            width:  Çizim genişliği (piksel).
            height: Çizim yüksekliği (piksel).
        """
        self.x      = x
        self.y      = y
        self.width  = width
        self.height = height

    # ── Soyut metotlar (alt sınıflar uygulamak zorunda) ───────────────────────

    @abstractmethod
    def update(self, dt: float, *args, **kwargs) -> None:
        """
        Varlığın oyun mantığını günceller.

        Args:
            dt: Son kareden bu yana geçen süre (saniye).
        """

    @abstractmethod
    def draw(self, surface: pygame.Surface) -> None:
        """
        Varlığı verilen yüzeye çizer.

        Args:
            surface: Çizim yapılacak pygame yüzeyi.
        """

    @abstractmethod
    def is_alive(self) -> bool:
        """Varlık hâlâ aktifse True döner."""

    # ── Yardımcı somut metot ──────────────────────────────────────────────────

    def get_rect(self) -> pygame.Rect:
        """Varlığın çevresini saran dikdörtgeni döner."""
        return pygame.Rect(
            self.x - self.width  // 2,
            self.y - self.height // 2,
            self.width,
            self.height,
        )

    def distance_to(self, other: "GameObject") -> float:
        """İki varlık arasındaki Öklid mesafesini hesaplar."""
        import math
        dx = self.x - other.x
        dy = self.y - other.y
        return math.sqrt(dx * dx + dy * dy)

    def distance_to_point(self, px: float, py: float) -> float:
        """Bir noktaya olan Öklid mesafesini hesaplar."""
        import math
        dx = self.x - px
        dy = self.y - py
        return math.sqrt(dx * dx + dy * dy)

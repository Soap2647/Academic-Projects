"""
Oyuncu sınıfı.

OOP Rolü (Kapsülleme):
    Player, altın ve can değerlerini özel alanlarda saklar.
    spend_gold(), earn_gold() ve lose_life() metotları doğrulama
    içeren kontrollü erişim noktaları sağlar.
"""

from __future__ import annotations

from settings import STARTING_GOLD, STARTING_LIVES


class Player:
    """
    Oyuncunun ekonomi ve can durumunu yöneten sınıf.

    Kapsülleme: __gold ve __lives doğrudan değiştirilemez;
    yalnızca doğrulama içeren public metotlar aracılığıyla güncellenir.
    """

    def __init__(self) -> None:
        self.__gold:        int = STARTING_GOLD
        self.__lives:       int = STARTING_LIVES
        self.__score:       int = 0
        self.__total_earned: int = 0   # toplam kazanılan altın (skor hesabı)
        self.__wave_reached: int = 1

    # ── Altın işlemleri ────────────────────────────────────────────────────────

    def spend_gold(self, amount: int) -> bool:
        """
        Belirtilen miktarda altını harcar.

        Args:
            amount: Harcanacak altın miktarı (>= 0).

        Returns:
            Yeterli altın varsa True ve altın düşülür; yoksa False döner.
        """
        if amount < 0:
            raise ValueError(f"Harcama miktarı negatif olamaz: {amount}")
        if self.__gold >= amount:
            self.__gold -= amount
            return True
        return False

    def earn_gold(self, amount: int) -> None:
        """
        Oyuncuya altın ekler.

        Args:
            amount: Eklenecek altın miktarı (>= 0).
        """
        if amount < 0:
            raise ValueError(f"Kazanç miktarı negatif olamaz: {amount}")
        self.__gold        += amount
        self.__total_earned += amount

    # ── Can işlemleri ──────────────────────────────────────────────────────────

    def lose_life(self) -> None:
        """Bir can düşürür; can sıfırın altına inmez."""
        self.__lives = max(0, self.__lives - 1)

    def is_defeated(self) -> bool:
        """Tüm canlar bitmişse True döner."""
        return self.__lives <= 0

    # ── Skor ───────────────────────────────────────────────────────────────────

    def update_score(self) -> None:
        """Mevcut dalga numarasını kullanarak skoru hesaplar."""
        self.__score = self.__total_earned * self.__wave_reached

    def set_wave_reached(self, wave: int) -> None:
        """Ulaşılan dalga numarasını kaydeder (skor hesabı için)."""
        self.__wave_reached = wave
        self.update_score()

    # ── Getter'lar (Kapsülleme) ────────────────────────────────────────────────

    def get_gold(self) -> int:
        return self.__gold

    def get_lives(self) -> int:
        return self.__lives

    def get_score(self) -> int:
        return self.__score

    def get_wave_reached(self) -> int:
        return self.__wave_reached

    def can_afford(self, amount: int) -> bool:
        """Belirtilen miktarı karşılayabiliyorsa True döner."""
        return self.__gold >= amount

    def reset(self) -> None:
        """Oyuncuyu başlangıç durumuna sıfırlar."""
        self.__gold         = STARTING_GOLD
        self.__lives        = STARTING_LIVES
        self.__score        = 0
        self.__total_earned = 0
        self.__wave_reached = 1

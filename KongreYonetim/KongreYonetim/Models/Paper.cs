using System.ComponentModel.DataAnnotations;

namespace KongreYonetim.Models
{
    public class Paper
    {
        public int Id { get; set; }

        [Required(ErrorMessage = "Başlık zorunludur.")]
        [Display(Name = "Bildiri Başlığı")]
        public string Title { get; set; }

        [Required(ErrorMessage = "Özet alanı zorunludur.")]
        [Display(Name = "Bildiri Özeti")]
        public string Abstract { get; set; }

        public string? FileName { get; set; } // Dosya Adı
        public string? FilePath { get; set; } // Dosya Yolu

        public string? AuthorId { get; set; } // Yazar ID

        // Varsayılan durum: Bekliyor
        public PaperStatus Status { get; set; } = PaperStatus.Pending;

        public virtual ICollection<Review>? Reviews { get; set; }
    }

    public enum PaperStatus
    {
        [Display(Name = "Değerlendirme Bekliyor")]
        Pending,
        [Display(Name = "İnceleniyor")]
        UnderReview,
        [Display(Name = "Kabul Edildi")]
        Accepted,
        [Display(Name = "Reddedildi")]
        Rejected
    }
}
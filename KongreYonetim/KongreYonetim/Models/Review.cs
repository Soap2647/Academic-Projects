using System.ComponentModel.DataAnnotations;

namespace KongreYonetim.Models
{
    public class Review
    {
        public int Id { get; set; }

        public int PaperId { get; set; }
        public virtual Paper? Paper { get; set; }

        public string? ReviewerId { get; set; }

        [Range(1, 100, ErrorMessage = "Puan 1-100 arasında olmalıdır.")]
        public int Score { get; set; }

        [Display(Name = "Hakem Yorumu")]
        public string? Comments { get; set; }
    }
}
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace KongreYonetim.Data
{
    public class ApplicationDbContext(DbContextOptions<ApplicationDbContext> options) : IdentityDbContext(options)
    {
        public DbSet<KongreYonetim.Models.Paper> Papers { get; set; }
        public DbSet<KongreYonetim.Models.Review> Reviews { get; set; }
    }
}

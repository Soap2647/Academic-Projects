using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Claims;
using System.Threading.Tasks;
using KongreYonetim.Data;
using KongreYonetim.Models;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Rendering;
using Microsoft.EntityFrameworkCore;

namespace KongreYonetim.Controllers
{
    [Authorize]
    public class PapersController : Controller
    {
        private readonly ApplicationDbContext _context;

        public PapersController(ApplicationDbContext context)
        {
            _context = context;
        }

        // GET: Papers
        public async Task<IActionResult> Index(string searchString)
        {
            // 1. Tüm bildirileri seç
            var papers = from p in _context.Papers
                         select p;

            // 2. Eğer arama kutusu doluysa filtrele
            if (!String.IsNullOrEmpty(searchString))
            {
                // Başlıkta VEYA Özette aranan kelime geçiyor mu?
                papers = papers.Where(s => s.Title.Contains(searchString) || s.Abstract.Contains(searchString));
            }

            // 3. Sonuçları listele
            return View(await papers.ToListAsync());
        }

        // GET: Papers/Details/5
        public async Task<IActionResult> Details(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var paper = await _context.Papers
                .FirstOrDefaultAsync(m => m.Id == id);
            if (paper == null)
            {
                return NotFound();
            }

            return View(paper);
        }

        // GET: Papers/Create
        public IActionResult Create()
        {
            return View();
        }

        // POST: Papers/Create
        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Create([Bind("Id,Title,Abstract")] Paper paper, IFormFile? upload)
        {
            // 1. Yazar bilgisini (Giriş yapmış kullanıcıyı) al
            var userId = User.FindFirstValue(ClaimTypes.NameIdentifier);
            paper.AuthorId = userId;
            paper.Status = PaperStatus.Pending; // Varsayılan durum: Bekliyor

            // 2. Dosya Yükleme İşlemi
            if (upload != null && upload.Length > 0)
            {
                // Dosya uzantısını kontrol et (Sadece PDF olsun)
                var extension = Path.GetExtension(upload.FileName).ToLower();
                if (extension != ".pdf")
                {
                    ModelState.AddModelError("", "Sadece PDF dosyaları kabul edilir.");
                    return View(paper);
                }

                // Rastgele dosya ismi üret (Çakışmayı önlemek için)
                var randomFileName = Guid.NewGuid().ToString() + extension;

                // Kayıt yolu: wwwroot/uploads
                var path = Path.Combine(Directory.GetCurrentDirectory(), "wwwroot/uploads");

                // Klasör yoksa oluştur
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                }

                // Dosyayı kaydet
                var filePath = Path.Combine(path, randomFileName);
                using (var stream = new FileStream(filePath, FileMode.Create))
                {
                    await upload.CopyToAsync(stream);
                }

                // Veritabanı için bilgileri doldur
                paper.FileName = upload.FileName; // Kullanıcının gördüğü isim
                paper.FilePath = randomFileName;  // Sunucudaki isim
            }
            else
            {
                // Dosya yüklenmediyse hata ver
                ModelState.AddModelError("upload", "Lütfen bir PDF dosyası seçiniz.");
                return View(paper);
            }

            // 3. Veritabanına Kaydet
            if (ModelState.IsValid)
            {
                _context.Add(paper);
                await _context.SaveChangesAsync();
                return RedirectToAction(nameof(Index));
            }
            return View(paper);
        }

        // GET: Papers/Edit/5
        public async Task<IActionResult> Edit(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var paper = await _context.Papers.FindAsync(id);
            if (paper == null)
            {
                return NotFound();
            }
            return View(paper);
        }

        // POST: Papers/Edit/5
        // To protect from overposting attacks, enable the specific properties you want to bind to.
        // For more details, see http://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Edit(int id, [Bind("Id,Title,Abstract,FileName,FilePath,AuthorId,Status")] Paper paper)
        {
            if (id != paper.Id)
            {
                return NotFound();
            }

            if (ModelState.IsValid)
            {
                try
                {
                    _context.Update(paper);
                    await _context.SaveChangesAsync();
                }
                catch (DbUpdateConcurrencyException)
                {
                    if (!PaperExists(paper.Id))
                    {
                        return NotFound();
                    }
                    else
                    {
                        throw;
                    }
                }
                return RedirectToAction(nameof(Index));
            }
            return View(paper);
        }

        // GET: Papers/Delete/5
        public async Task<IActionResult> Delete(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var paper = await _context.Papers
                .FirstOrDefaultAsync(m => m.Id == id);
            if (paper == null)
            {
                return NotFound();
            }

            return View(paper);
        }

        // POST: Papers/Delete/5
        [HttpPost, ActionName("Delete")]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> DeleteConfirmed(int id)
        {
            var paper = await _context.Papers.FindAsync(id);
            if (paper != null)
            {
                _context.Papers.Remove(paper);
            }

            await _context.SaveChangesAsync();
            return RedirectToAction(nameof(Index));
        }

        private bool PaperExists(int id)
        {
            return _context.Papers.Any(e => e.Id == id);
        }
    }
}

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
    public class ReviewsController : Controller
    {
        private readonly ApplicationDbContext _context;

        public ReviewsController(ApplicationDbContext context)
        {
            _context = context;
        }

        // GET: Reviews
        public async Task<IActionResult> Index()
        {
            var applicationDbContext = _context.Reviews.Include(r => r.Paper);
            return View(await applicationDbContext.ToListAsync());
        }

        // GET: Reviews/Details/5
        public async Task<IActionResult> Details(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var review = await _context.Reviews
                .Include(r => r.Paper)
                .FirstOrDefaultAsync(m => m.Id == id);
            if (review == null)
            {
                return NotFound();
            }

            return View(review);
        }

        /// GET: Reviews/Create
        [Authorize]
        public async Task<IActionResult> Create(int? paperId)
        {
            if (paperId == null) return NotFound();

            var paper = await _context.Papers.FindAsync(paperId);
            if (paper == null) return NotFound();

            // ... (Kendi kendini puanlama engeli kodu burada kalacak) ...

            ViewData["PaperId"] = new SelectList(_context.Papers, "Id", "Title", paperId);

            // --- BURAYI GÜNCELLİYORUZ ---
            ViewData["PaperTitle"] = paper.Title;       // Başlığı gönder
            ViewData["PaperAbstract"] = paper.Abstract; // Özeti de gönder (YENİ)
                                                        // ----------------------------

            return View();
        }

        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Create([Bind("Id,PaperId,Score,Comments")] Review review)
        {
            // 1. Hakem kimliğini al
            review.ReviewerId = User.FindFirstValue(ClaimTypes.NameIdentifier);

            if (ModelState.IsValid)
            {
                // 2. Önce Puanı (Review) Kaydet
                _context.Add(review);

                // 3. --- YENİ EKLENEN KISIM: Bildirinin Durumunu Güncelle ---
                var paper = await _context.Papers.FindAsync(review.PaperId);
                if (paper != null)
                {
                    // Basit Okul Mantığı: Puan 50 ve üzeriyse KABUL, altıysa RET
                    if (review.Score >= 50)
                    {
                        paper.Status = PaperStatus.Accepted; // Kabul Edildi yap
                    }
                    else
                    {
                        paper.Status = PaperStatus.Rejected; // Reddedildi yap
                    }

                    // Bildiri tablosunu da güncellediğimizi belirtiyoruz
                    _context.Update(paper);
                }
                // -----------------------------------------------------------

                // 4. Her iki değişikliği (Review Ekleme + Paper Güncelleme) veritabanına yaz
                await _context.SaveChangesAsync();

                return RedirectToAction("Index", "Papers");
            }

            ViewData["PaperId"] = new SelectList(_context.Papers, "Id", "Title", review.PaperId);
            return View(review);
        }

        // GET: Reviews/Edit/5
        public async Task<IActionResult> Edit(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var review = await _context.Reviews.FindAsync(id);
            if (review == null)
            {
                return NotFound();
            }
            ViewData["PaperId"] = new SelectList(_context.Papers, "Id", "Abstract", review.PaperId);
            return View(review);
        }

        // POST: Reviews/Edit/5
        // To protect from overposting attacks, enable the specific properties you want to bind to.
        // For more details, see http://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> Edit(int id, [Bind("Id,PaperId,ReviewerId,Score,Comments")] Review review)
        {
            if (id != review.Id)
            {
                return NotFound();
            }

            if (ModelState.IsValid)
            {
                try
                {
                    _context.Update(review);
                    await _context.SaveChangesAsync();
                }
                catch (DbUpdateConcurrencyException)
                {
                    if (!ReviewExists(review.Id))
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
            ViewData["PaperId"] = new SelectList(_context.Papers, "Id", "Abstract", review.PaperId);
            return View(review);
        }

        // GET: Reviews/Delete/5
        public async Task<IActionResult> Delete(int? id)
        {
            if (id == null)
            {
                return NotFound();
            }

            var review = await _context.Reviews
                .Include(r => r.Paper)
                .FirstOrDefaultAsync(m => m.Id == id);
            if (review == null)
            {
                return NotFound();
            }

            return View(review);
        }

        // POST: Reviews/Delete/5
        [HttpPost, ActionName("Delete")]
        [ValidateAntiForgeryToken]
        public async Task<IActionResult> DeleteConfirmed(int id)
        {
            var review = await _context.Reviews.FindAsync(id);
            if (review != null)
            {
                _context.Reviews.Remove(review);
            }

            await _context.SaveChangesAsync();
            return RedirectToAction(nameof(Index));
        }

        private bool ReviewExists(int id)
        {
            return _context.Reviews.Any(e => e.Id == id);
        }
    }
}

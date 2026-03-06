# 🖥️ Mert Studio Code - Türkçe Kod Editörü

![Version](https://img.shields.io/badge/version-3.5.0-blue)
![License](https://img.shields.io/badge/license-Mert_Studio-green)
![Electron](https://img.shields.io/badge/Electron-40.4.0-informational)
![React](https://img.shields.io/badge/React-19.2.0-61dafb)

Mert Studio Code, geliştiriciler için modern, hızlı ve özellikle Türkçe dil desteği ile güçlendirilmiş bir kod editörü aracıdır. Electron tabanlı yapısı sayesinde masaüstü deneyimini en üst seviyeye taşır.

---

## ✨ Özellikler

- **🇹🇷 Türkçe Kod Desteği:** Kod yazımı sırasında Türkçe karakter ve özel sözdizimi desteği.
- **🎨 Özelleştirilebilir Temalar:** `ThemeSelector` ile geniş tema yelpazesi arasından seçim yapabilme.
- **⚙️ Gelişmiş Ayarlar:** `SettingsModal` üzerinden kişiselleştirilebilir düzenleme seçenekleri.
- **⚡ Hızlı ve Hafif:** Vite ve React 19 ile optimize edilmiş performans.
- **📱 Masaüstü Uygulaması:** Electron ile Windows, macOS ve Linux desteği.

---

## 🛠️ Teknoloji Yığını

- **Frontend:** [React 19](https://reactjs.org/)
- **Masaüstü:** [Electron](https://www.electronjs.org/)
- **Derleme Aracı:** [Vite](https://vitejs.dev/)
- **Stil:** [TailwindCSS 4](https://tailwindcss.com/)
- **İkonlar:** [Lucide-React](https://lucide.dev/)

---

## 🚀 Başlangıç

Projeyi yerel makinenizde çalıştırmak için aşağıdaki adımları izleyin:

### Gereksinimler

- [Node.js](https://nodejs.org/) (Sürüm 18 veya üzeri önerilir)
- npm veya yarn

### Kurulum

1. Depoyu klonlayın veya indirin.
2. Klasöre gidin:
   ```bash
   cd Mert-Studio-Code
   ```
3. Bağımlılıkları yükleyin:
   ```bash
   npm install
   ```

### Çalıştırma

Geliştirme modunda (Tarayıcı):
```bash
npm run dev
```

Electron (Masaüstü Uygulaması) olarak çalıştırma:
```bash
npm run electron:dev
```

---

## 📦 Dağıtım ve Build

Uygulamayı paketlemek (Portable veya Installer):

```bash
npm run electron:dist
```

*Çıktılar `release` klasöründe oluşturulacaktır.*



<?php
$baglan = mysqli_connect("localhost", "root", "", "okul");
mysqli_set_charset($baglan, "utf8");

if (!$baglan) {
    die("Bağlantı hatası: " . mysqli_connect_error());
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $adsoyad = $_POST['adsoyad'];
    $bolum = $_POST['bolum'];

    $sql = "INSERT INTO ogrenci (adsoyad, bolum) VALUES ('$adsoyad', '$bolum')";
    $ekle = mysqli_query($baglan, $sql);

    if ($ekle) {
        echo "✅ Kayıt eklendi.<br><br>";
    } else {
        echo "❌ Kayıt başarısız: " . mysqli_error($baglan) . "<br><br>";
    }

    echo "<b>Eklenen Bilgiler:</b><br>";
    echo "Ad Soyad: $adsoyad <br>";
    echo "Bölüm: $bolum <br><br>";
    echo "<a href='index.php'>Geri Dön</a><br><br>";
}

mysqli_close($baglan);
?>

<html>
<head><title>Veritabanı Kayıt Ekleme</title></head>
<body>
<h2>Yeni Öğrenci Ekle</h2>
<form action="index.php" method="POST">
    Ad Soyad: <input type="text" name="adsoyad" required><br><br>
    Bölüm: <input type="text" name="bolum" required><br><br>
    <input type="submit" value="Kaydet">
</form>
<br>
<a href="kayitlar.php">Kayıtları Gör</a>
</body>
</html>

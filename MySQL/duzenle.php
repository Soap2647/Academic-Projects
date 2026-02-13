<?php
$baglan = mysqli_connect("localhost", "root", "", "okul");
mysqli_set_charset($baglan, "utf8");

if (!$baglan) {
    die("Bağlantı hatası: " . mysqli_connect_error());
}

if (isset($_GET['id'])) {
    $id = $_GET['id'];
    $sorgu = mysqli_query($baglan, "SELECT * FROM ogrenci WHERE numara='$id'");
    $bilgi = mysqli_fetch_assoc($sorgu);
}

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $id = $_POST['id'];
    $adsoyad = $_POST['adsoyad'];
    $bolum = $_POST['bolum'];

    $guncelle = mysqli_query($baglan, "UPDATE ogrenci SET adsoyad='$adsoyad', bolum='$bolum' WHERE numara='$id'");

    if ($guncelle) {
        echo "✅ Güncelleme başarılı.";
        header("refresh:2; url=kayitlar.php");
    } else {
        echo "❌ Güncelleme hatası: " . mysqli_error($baglan);
    }
}
?>

<html>
<head><title>Kayıt Güncelle</title></head>
<body>
<h2>Kayıt Güncelle</h2>
<form action="duzenle.php" method="POST">
    <input type="hidden" name="id" value="<?php echo $bilgi['numara']; ?>">
    Ad Soyad: <input type="text" name="adsoyad" value="<?php echo $bilgi['adsoyad']; ?>"><br><br>
    Bölüm: <input type="text" name="bolum" value="<?php echo $bilgi['bolum']; ?>"><br><br>
    <input type="submit" value="Güncelle">
</form>
<br>
<a href="kayitlar.php">Geri Dön</a>
</body>
</html>
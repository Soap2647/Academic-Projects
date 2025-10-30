<?php
$baglan = mysqli_connect("localhost", "root", "", "okul");
mysqli_set_charset($baglan, "utf8");

if (!$baglan) {
    die("Bağlantı hatası: " . mysqli_connect_error());
}

$id = $_GET['id'];

$sil = mysqli_query($baglan, "DELETE FROM ogrenci WHERE numara='$id'");

if ($sil) {
    echo "✅ Silme işlemi başarılı.";
    header("refresh:1; url=kayitlar.php");
} else {
    echo "❌ Silme hatası: " . mysqli_error($baglan);
}

mysqli_close($baglan);
?>
<html>
<head><title>Kayıtlar</title></head>
<body>
<h2>Öğrenci Listesi</h2>
<table border="1" cellpadding="5">
<tr style="background-color: #ddd;">
    <td>Ad Soyad</td>
    <td>Bölüm</td>
    <td>Düzenle</td>
    <td>Sil</td>
</tr>

<?php
$baglan = mysqli_connect("localhost", "root", "", "okul");
mysqli_set_charset($baglan, "utf8");

if (!$baglan) {
    die("Bağlantı Hatası: " . mysqli_connect_error());
}

$bilgiler = mysqli_query($baglan, "SELECT * FROM ogrenci");

while ($goster = mysqli_fetch_assoc($bilgiler)) {
    $id = $goster['numara'];
    $adsoyad = $goster['adsoyad'];
    $bolum = $goster['bolum'];

    echo "<tr>";
    echo "<td>$adsoyad</td>";
    echo "<td>$bolum</td>";
    echo "<td><a href='duzenle.php?id=$id'>Düzenle</a></td>";
    echo "<td><a href='sil.php?id=$id' onclick=\"return confirm('Silmek istediğinize emin misiniz?')\">Sil</a></td>";
    echo "</tr>";
}

mysqli_close($baglan);
?>
</table>
<br>
<a href="index.php">Yeni Kayıt Ekle</a>
</body>
</html>

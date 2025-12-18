# Veri İletişimi Hata Tespit Projesi
Bu proje, veri iletişiminde kullanılan hata tespit yöntemlerini gösteren socket programlama uygulamasıdır. Proje, veri aktarımı sırasında oluşabilecek hataların nasıl tespit edildiğini göstermektedir.

# Sistem Mimarisi
Proje, C dilinde yazılmış 3 bağımsız program ile çalışır.

1. client1.c : 
    - Kullanıcıdan gönderilecek metni alır
    - Kullanıcının seçtiği hata tespit yöntemine işlemi ayarlar
    - Veri + Yöntem + Kontrol bilgisini içeren **paket** oluşturur
    - Paketi TCP bağlantısı üzerinden Server'a gönderir
    
2. server.c :
    - Client1'den gelen paketi alır
    - Paketteki veriyi kasıtlı olarak bozar 
    - Bozulmuş paketi Client2'ye iletir
    - Hangi hatanın enjekte edildiğini terminalde gösterir
   
3. client2.c :
    - Server'dan bozulmuş paketi alır
    - Paketi parse eder (veri, yöntem, gelen kontrol bilgisi)
    - Aynı yöntemi kullanarak kendi kontrol bilgisini hesaplar
    - İki kontrol bilgisini karşılaştırır
    - Sonucu raporlar: DATA CORRECT veya DATA CORRUPTED
   
## Kullanım

Program 3 farklı terminal açılarak proje dosyaları herbiri manuel olarak başlatılır.
Programı çalıştırdığınızda:

1. Göndermek istediğiniz metni yazın
2. Hata tespit yöntemini seçin (1-5 arası)
3. Sistem otomatik olarak kontrol bilgisini üretir ve gönderir


### Hata Tespit Yöntemleri

1. **Parity Bit**: Basit parite kontrolü
2. **2D Parity**: Matris tabanlı parite kontrolü
3. **CRC**: Döngüsel artıklık kontrolü
4. **Hamming Code**: Hamming kod ile hata tespiti
5. **Internet Checksum**: IP checksum algoritması

#### Örnek Çıktı
```
=== CLIENT 2 - VERI ALICI VE KONTROL ===

Server'a baglaniliyor...
Server'a baglandi.
Paket bekleniyor...

Paket alindi: helXlo|2D_PARITY|0162

========================================
Received Data        : helXlo
Method               : 2D_PARITY
Sent Check Bits      : 0162
Computed Check Bits  : 003A
----------------------------------------
Status               : DATA CORRUPTED
========================================

Client2 kapatiliyor...
```




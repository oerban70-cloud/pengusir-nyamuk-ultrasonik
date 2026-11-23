# Pengusir Nyamuk Ultrasonik — ESP32 + Buzzer Piezo

Nyamuk ganggu tidurmu tiap malam? Alat kecil ini membunyikan suara
ultrasonik yang tidak terdengar telinga manusia. ESP32 membangkitkan
gelombang PWM 20.000–40.000 Hz, transistor 2N2222 menguatkannya, dan
buzzer piezo pasif mengubahnya menjadi suara ultrasonik. Putar
potensiometer untuk mengubah frekuensi, tekan tombol untuk menyalakan
atau mematikan alat.

Video penjelasan (60 detik):
https://youtube.com/shorts/2p8xPMx1I9M

> Catatan jujur: efektivitas pengusir nyamuk ultrasonik masih
> diperdebatkan secara ilmiah. Alat ini membunyikan frekuensi yang
> diklaim mengganggu nyamuk, tetapi tidak ada jaminan nyamuk pasti
> pergi — coba sendiri semalam dan lihat hasilnya.

## Komponen

| Bagian | Keterangan |
|---|---|
| ESP32 DevKit | Otak alat, membangkitkan PWM (LEDC) |
| Buzzer piezo pasif | Mengubah gelombang PWM menjadi suara ultrasonik |
| Transistor NPN 2N2222 | Saklar penguat buzzer (basis via resistor 1k) |
| Resistor 1k | Seri basis transistor |
| Tombol push | Toggle on/off, ke GND (INPUT_PULLUP) |
| Potensiometer 10k | Mengatur frekuensi 20.000–40.000 Hz |
| LED hijau | Indikator, menyala saat alat ON |
| Resistor 220 Ω | Seri dengan LED hijau |
| Adaptor 5V 2A | Catu ESP32 (VIN) + buzzer (+) |

## Wiring

| Dari | Ke |
|---|---|
| Buzzer piezo (+) | 5V (adaptor) |
| Buzzer piezo (-) | Kolektor 2N2222 |
| 2N2222 basis | ESP32 GPIO25 (via resistor 1k) |
| 2N2222 emitor | GND |
| Tombol push | ESP32 GPIO4 ke GND (tekan = LOW) |
| Potensiometer wiper | ESP32 GPIO34 (ADC) |
| Potensiometer ujung | 3V3 dan GND |
| LED hijau | ESP32 GPIO2 (seri 220 Ω ke GND) |
| Adaptor 5V | ESP32 VIN + buzzer (+) |
| Semua GND | Digabung |

Skema lengkap: [schematic.png](schematic.png)

## Cara kerja

1. ESP32 membangkitkan PWM di GPIO25 dengan API LEDC core 4.x
   (`ledcAttach`, `ledcWrite`, `ledcChangeFrequency`), duty 50%.
2. Sinyal PWM masuk ke basis transistor 2N2222 lewat resistor 1k;
   transistor menghidup-matikan arus buzzer yang dicatu 5V.
3. Buzzer piezo pasif bergetar mengikuti frekuensi PWM dan
   menghasilkan bunyi ultrasonik di atas 20.000 Hz.
4. Potensiometer dibaca ADC GPIO34 (0–4095) lalu dipetakan ke
   frekuensi 20.000–40.000 Hz; frekuensi baru diterapkan bila
   bergeser minimal 200 Hz supaya tidak bergetar karena noise ADC.
5. Tombol di GPIO4 (INPUT_PULLUP) dengan debounce 50 ms membalik
   status alat; LED hijau di GPIO2 menyala saat alat aktif.
6. Saat alat dimatikan, PWM ditulis 0 sehingga buzzer diam.

## Konfigurasi (di awal sketch)

- `PIN_BUZZER` (25) — PWM ke basis transistor 2N2222.
- `PIN_TOMBOL` (4) — tombol push ke GND, INPUT_PULLUP.
- `PIN_POTENSIOMETER` (34) — wiper potensiometer 10k (ADC).
- `PIN_LED` (2) — LED indikator.
- `FREKUENSI_MIN_HZ` (20000) / `FREKUENSI_MAKS_HZ` (40000) — rentang frekuensi.
- `FREKUENSI_AWAL_HZ` (25000) — frekuensi default saat dinyalakan.
- `LANGKAH_FREKUENSI_HZ` (200) — perubahan minimum untuk update frekuensi.
- `RESOLUSI_PWM_BIT` (8) / `DUTY_PWM` (128) — PWM 8-bit, duty 50%.
- `WAKTU_DEBOUNCE_MS` (50) — jeda anti-goyang tombol.

## Library yang dibutuhkan

Tidak ada — sketch ini hanya memakai core ESP32 (LEDC + ADC),
jadi tidak perlu library eksternal.

## Compile

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 PengusirNyamukUltrasonik
```

Terverifikasi 23 Nov 2026: sketch 309.579 byte (23% flash),
RAM 23.508 byte (7%), ESP32 core 4.0.0.

## Lisensi

MIT — lihat [LICENSE](LICENSE).

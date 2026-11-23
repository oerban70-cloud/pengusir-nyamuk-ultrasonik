// PengusirNyamukUltrasonik.ino
// Pengusir nyamuk ultrasonik: ESP32 membangkitkan gelombang PWM di atas
// 20.000 Hz, buzzer piezo pasif mengubahnya menjadi suara ultrasonik.
// Tombol menghidupkan/mematikan alat, potensiometer mengatur frekuensi
// 20.000-40.000 Hz, LED menyala saat alat aktif.
//
// CATATAN KEJUJURAN: efektivitas pengusir nyamuk ultrasonik masih
// diperdebatkan secara ilmiah. Alat ini membunyikan frekuensi yang
// diklaim mengganggu nyamuk, tetapi tidak ada jaminan nyamuk pasti
// pergi — coba sendiri semalam dan lihat hasilnya.
//
// Rangkaian:
//   Buzzer piezo pasif -> kolektor transistor NPN 2N2222
//   Transistor 2N2222  : basis -> GPIO25 via resistor 1k,
//                        emitor -> GND, kolektor -> buzzer (-)
//   Buzzer (+)         -> 5V (adaptor)
//   Tombol push        -> GPIO4 ke GND (INPUT_PULLUP, tekan = LOW)
//   Potensiometer 10k  : ujung -> 3V3 dan GND, wiper -> GPIO34 (ADC)
//   LED hijau          -> GPIO2 + resistor 220 ohm ke GND
//   Catu               -> adaptor 5V ke VIN ESP32
//
// API LEDC ESP32 core 4.x: ledcAttach(pin, freq, resolusi),
// ledcWrite(pin, duty), ledcChangeFrequency(pin, freq, resolusi).

// ---------- konfigurasi pin ----------
const uint8_t PIN_BUZZER = 25;  // PWM ke basis transistor 2N2222 (via 1k)
const uint8_t PIN_TOMBOL = 4;   // tombol push ke GND, INPUT_PULLUP
const uint8_t PIN_POTENSIOMETER = 34;  // wiper potensiometer 10k (ADC)
const uint8_t PIN_LED = 2;      // LED indikator (anoda via resistor 220 ohm)

// ---------- konfigurasi frekuensi ----------
const uint32_t FREKUENSI_MIN_HZ = 20000;  // batas bawah ultrasonik
const uint32_t FREKUENSI_MAKS_HZ = 40000;  // batas atas yang diatur potensio
const uint32_t FREKUENSI_AWAL_HZ = 25000;  // frekuensi default saat dinyalakan
const uint32_t LANGKAH_FREKUENSI_HZ = 200;  // perubahan minimum agar frekuensi di-update
const uint8_t RESOLUSI_PWM_BIT = 8;  // resolusi LEDC, duty 0-255
const uint8_t DUTY_PWM = 128;  // duty 50% (gelombang kotak)

// ---------- konfigurasi ADC dan debounce ----------
const int NILAI_ADC_MAKS = 4095;  // ADC ESP32 12-bit
const unsigned long WAKTU_DEBOUNCE_MS = 50;  // jeda anti-goyang tombol

// ---------- status ----------
bool alatAktif = false;  // alat mati saat pertama dinyalakan
uint32_t frekuensiSaatIni = FREKUENSI_AWAL_HZ;
bool tombolSebelumnya = HIGH;  // INPUT_PULLUP: HIGH = tombol tidak ditekan
unsigned long waktuTombolTerakhir = 0;  // millis() tekanan tombol terakhir yang sah

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TOMBOL, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_POTENSIOMETER, INPUT);
  analogReadResolution(12);

  // Siapkan kanal PWM di GPIO25 pada frekuensi awal, duty 0 (diam).
  ledcAttach(PIN_BUZZER, FREKUENSI_AWAL_HZ, RESOLUSI_PWM_BIT);
  ledcWrite(PIN_BUZZER, 0);
  digitalWrite(PIN_LED, LOW);

  Serial.println("Pengusir nyamuk ultrasonik siap.");
  Serial.println("Tekan tombol untuk menyalakan, putar potensiometer untuk mengubah frekuensi.");
}

void loop() {
  bacaTombol();
  if (alatAktif) {
    aturFrekuensiDariPotensiometer();
  }
}

// Membaca tombol (tekan = LOW) dengan debounce, lalu toggle alat.
void bacaTombol() {
  bool tombol = digitalRead(PIN_TOMBOL);
  if (tombol == LOW && tombolSebelumnya == HIGH &&
      millis() - waktuTombolTerakhir >= WAKTU_DEBOUNCE_MS) {
    waktuTombolTerakhir = millis();
    alatAktif = !alatAktif;
    terapkanStatusAlat();
  }
  tombolSebelumnya = tombol;
}

// Menyalakan atau mematikan buzzer dan LED sesuai status alat.
void terapkanStatusAlat() {
  if (alatAktif) {
    frekuensiSaatIni = FREKUENSI_AWAL_HZ;
    ledcChangeFrequency(PIN_BUZZER, frekuensiSaatIni, RESOLUSI_PWM_BIT);
    ledcWrite(PIN_BUZZER, DUTY_PWM);
    digitalWrite(PIN_LED, HIGH);
    Serial.print("Alat MENYALA pada ");
    Serial.print(frekuensiSaatIni);
    Serial.println(" Hz.");
  } else {
    ledcWrite(PIN_BUZZER, 0);
    digitalWrite(PIN_LED, LOW);
    Serial.println("Alat MATI.");
  }
}

// Membaca potensiometer dan mengubah frekuensi bila bergeser cukup jauh.
void aturFrekuensiDariPotensiometer() {
  int nilaiAdc = analogRead(PIN_POTENSIOMETER);
  uint32_t frekuensiBaru = map(nilaiAdc, 0, NILAI_ADC_MAKS,
                               FREKUENSI_MIN_HZ, FREKUENSI_MAKS_HZ);
  if (frekuensiBaru < FREKUENSI_MIN_HZ) {
    frekuensiBaru = FREKUENSI_MIN_HZ;
  }
  if (frekuensiBaru > FREKUENSI_MAKS_HZ) {
    frekuensiBaru = FREKUENSI_MAKS_HZ;
  }
  long selisih = (long)frekuensiBaru - (long)frekuensiSaatIni;
  if (selisih < 0) {
    selisih = -selisih;
  }
  if ((uint32_t)selisih >= LANGKAH_FREKUENSI_HZ) {
    frekuensiSaatIni = frekuensiBaru;
    ledcChangeFrequency(PIN_BUZZER, frekuensiSaatIni, RESOLUSI_PWM_BIT);
    ledcWrite(PIN_BUZZER, DUTY_PWM);
    Serial.print("Frekuensi: ");
    Serial.print(frekuensiSaatIni);
    Serial.println(" Hz.");
  }
}

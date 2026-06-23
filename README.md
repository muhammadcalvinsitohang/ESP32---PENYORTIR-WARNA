ESP32 Color Sorter using TCS3200 and k-NN Classification

Proyek ini merupakan sistem penyortir warna otomatis berbasis ESP32 menggunakan sensor warna TCS3200 dan algoritma k-Nearest Neighbor (k-NN). Sistem mampu mendeteksi objek berwarna merah, hijau, dan biru, kemudian mengarahkan objek ke jalur yang sesuai menggunakan motor servo sebagai pemisah.

Selain proses klasifikasi warna, sistem dilengkapi dengan:

1. Sensor TCS3200 sebagai pembaca nilai RGB.
2. Circular Buffer dan Moving Average untuk mereduksi noise pembacaan sensor.
3. Algoritma Euclidean Distance dan k-Nearest Neighbor (k-NN) sebagai metode klasifikasi warna.
4. Sensor IR untuk mendeteksi keberadaan objek.
5. Servo 180° sebagai mekanisme penyortiran.
6. LED indikator untuk menunjukkan hasil klasifikasi.
7. Monitoring real-time menggunakan aplikasi Blynk IoT.
8. Counter jumlah objek berdasarkan warna.

Proyek ini dibuat sebagai implementasi sistem embedded dan Internet of Things (IoT) pada alat penyortir warna otomatis berbasis ESP32.

Perangkat Keras

1. ESP32 DevKit V1
2. Sensor warna TCS3200
3. Sensor Infrared (IR)
4. Motor Servo SG90/MG90S 180°
5. LED Merah
6. LED Hijau
7. LED Biru
8. Kabel jumper
9. Catu daya 5 V

Library yang Digunakan

```cpp
WiFi.h
BlynkSimpleEsp32.h
ESP32Servo.h
math.h
```

Cara Menjalankan Program

 1. Install Arduino IDE

Pastikan Arduino IDE telah terpasang pada komputer.

 2. Install Board ESP32

Tambahkan URL berikut pada Additional Board Manager URLs:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Kemudian install board ESP32 melalui Board Manager.

3. Install Library

Install library berikut melalui Library Manager:

1. Blynk
2. ESP32Servo

 4. Konfigurasi WiFi dan Blynk

Ubah parameter berikut sesuai jaringan yang digunakan:

```cpp
char ssid[] = "Nama_WiFi";
char pass[] = "Password_WiFi";

#define BLYNK_TEMPLATE_ID "xxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Color Sorter"
#define BLYNK_AUTH_TOKEN "xxxxxxxx"
```

5. Upload Program

1. Pilih board ESP32 Dev Module.
2. Pilih port COM yang sesuai.
3. Upload file program ke ESP32.

6. Jalankan Sistem

1. Sensor TCS3200 membaca warna objek.
2. Data RGB difilter menggunakan Circular Buffer dan Moving Average.
3. Nilai RGB diklasifikasikan menggunakan Euclidean Distance dan algoritma k-NN.
4. Sensor IR mendeteksi keberadaan objek.
5. Servo mengarahkan objek sesuai warna:

   * Merah → kiri.
   * Hijau → lurus.
   * Biru → kanan.
6. Jumlah objek dan nilai sensor ditampilkan secara real-time melalui aplikasi Blynk.

 Struktur Repository

```
ESP32-Color-Sorter/
│
├── ESP32_Color_Sorter.ino
├── README.md
├── LICENSE
├── Flowchart.png
├── Wiring_Diagram.png
├── Dataset_RGB.xlsx
└── images/
```

 Metode yang Digunakan

1. Circular Buffer
2. Moving Average
3. Euclidean Distance
4. Selection Sort
5. k-Nearest Neighbor (k-NN)

 Penulis

[M. Calvin Sitohang]

Teknik Elektro

Universitas Lampung

2026

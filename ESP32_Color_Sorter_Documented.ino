//=====================================================
// PROYEK : PENYORTIR WARNA OTOMATIS BERBASIS ESP32
// SENSOR : TCS3200
// METODE : Circular Buffer + Moving Average +
//           Euclidean Distance + Selection Sort + k-NN
// AKTUATOR : Servo 180°
// MONITORING : Blynk IoT
//=====================================================


#define BLYNK_TEMPLATE_ID   "TMPL6HSg78t_E"
#define BLYNK_TEMPLATE_NAME "Penyortir Warna"
#define BLYNK_AUTH_TOKEN    "LpistW7a2A48oohl62R_QoWS1dTYtP4o"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <ESP32Servo.h>
#include <math.h>

char ssid[] = "hospot";
char pass[] = "33333333";

#define V_WARNA        V0
#define V_STATUS       V1
#define V_R            V2
#define V_G            V3
#define V_B            V4
#define V_JARAK        V5
#define V_COUNT_MERAH  V6
#define V_COUNT_HIJAU  V7
#define V_COUNT_BIRU   V8
#define V_COUNT_TOTAL  V9
#define V_RESET        V10

#define PIN_S0 5
#define PIN_S1 18
#define PIN_S2 19
#define PIN_S3 21
#define PIN_OUT 22

#define PIN_IR 23

#define PIN_SERVO 14

#define LED_MERAH 4
#define LED_HIJAU 2
#define LED_BIRU 15

#define SUDUT_LURUS        90    // posisi netral/lurus (HIJAU lewat tengah)

#define SUDUT_DORONG_MERAH 140   // dorong ke KIRI (lebih besar dari 90)
#define SUDUT_DORONG_BIRU  160    // dorong ke KANAN (lebih kecil dari 90)

#define DELAY_GERAK_AWAL   400   // waktu servo bergerak menuju posisi dorong (ms)

#define DELAY_TAHAN_MERAH  1300  // <<< DIPERBESAR untuk MERAH >>>
#define DELAY_TAHAN_BIRU   900   // waktu tahan untuk BIRU (silakan sesuaikan)

#define DELAY_KEMBALI      400   // waktu servo bergerak balik ke posisi lurus (ms)

#define BUFFER_SIZE 5
#define JUMLAH_DATA 20
#define K 5
float dataLatih[JUMLAH_DATA][3] =
{
  {74,170,146},
  {50,151,123},
  {67,173,145},
  {58,160,134},
  {37,131,104},

  {97,69,97},
  {90,64,90},
  {103,78,104},
  {109,88,110},
  {116,96,118},

  {144,123,71},
  {148,126,73},
  {160,149,93},
  {170,165,111},
  {163,162,109},

  {114,126,118},
  {94,104,97},
  {95,105,98},
  {98,108,101},
  {102,112,104}
};

int label[JUMLAH_DATA] =
{
  1,1,1,1,1,
  2,2,2,2,2,
  3,3,3,3,3,
  0,0,0,0,0
};
Servo servoPemisah;

BlynkTimer timer;

int bufferR[BUFFER_SIZE];
int bufferG[BUFFER_SIZE];
int bufferB[BUFFER_SIZE];

int indeksBuffer = 0;

String warnaTersimpan = "UNKNOWN";

bool warnaSudahTerbaca = false;
bool prosesAktif = false;

float gAvgR = 0, gAvgG = 0, gAvgB = 0, gJarakTerdekat = 999;
unsigned long jumlahMerah = 0, jumlahHijau = 0, jumlahBiru = 0;


//=====================================================
// Fungsi membaca frekuensi warna dari sensor TCS3200
//=====================================================
int bacaKanal(bool s2, bool s3)
{
  digitalWrite(PIN_S2, s2);
  digitalWrite(PIN_S3, s3);

  delay(5);

  long total = 0;

  for(int i=0;i<10;i++)
  {
    total += pulseIn(PIN_OUT, LOW);
    delay(2);
  }

  return total/10;
}

//=====================================================
// Fungsi memasukkan data RGB ke circular buffer
//=====================================================
void updateBuffer(int R,int G,int B)
{
  bufferR[indeksBuffer]=R;
  bufferG[indeksBuffer]=G;
  bufferB[indeksBuffer]=B;

  indeksBuffer++;

  if(indeksBuffer>=BUFFER_SIZE)
    indeksBuffer=0;
}

//=====================================================
// Fungsi menghitung rata-rata data pada buffer
//=====================================================
float rataRata(int buffer[])
{
  long total=0;

  for(int i=0;i<BUFFER_SIZE;i++)
    total+=buffer[i];

  return total/(float)BUFFER_SIZE;
}

//=====================================================
// Fungsi menghitung jarak Euclidean terhadap data latih
//=====================================================
float euclidean(float R,float G,float B,int i)
{
  float dR=R-dataLatih[i][0];
  float dG=G-dataLatih[i][1];
  float dB=B-dataLatih[i][2];

  return sqrt(dR*dR+dG*dG+dB*dB);
}

//=====================================================
// Fungsi mengurutkan data menggunakan Selection Sort
//=====================================================
void selectionSort(float d[], int idx[], int n)
{
  for(int i=0;i<n-1;i++)
  {
    int minIdx=i;

    for(int j=i+1;j<n;j++)
    {
      if(d[j]<d[minIdx])
        minIdx=j;
    }

    float tempD=d[i];
    d[i]=d[minIdx];
    d[minIdx]=tempD;

    int tempI=idx[i];
    idx[i]=idx[minIdx];
    idx[minIdx]=tempI;
  }
}

//=====================================================
// Fungsi klasifikasi warna menggunakan algoritma k-NN
//=====================================================
String klasifikasiKNN(float R,float G,float B)
{
  float distance[JUMLAH_DATA];
  int idx[JUMLAH_DATA];

  for(int i=0;i<JUMLAH_DATA;i++)
  {
    distance[i]=euclidean(R,G,B,i);
    idx[i]=label[i];
  }

  selectionSort(distance,idx,JUMLAH_DATA);

  if(distance[0]>60)
    return "UNKNOWN";

  int merah=0;
  int hijau=0;
  int biru=0;
  int tidak=0;

  for(int i=0;i<K;i++)
  {
    if(idx[i]==1) merah++;
    else if(idx[i]==2) hijau++;
    else if(idx[i]==3) biru++;
    else tidak++;
  }

  if(merah>=3) return "MERAH";
  if(hijau>=3) return "HIJAU";
  if(biru>=3) return "BIRU";
  if(tidak>=3) return "TIDAK ADA OBJEK";

  return "UNKNOWN";
}


//=====================================================
// Fungsi mencari jarak terdekat terhadap data latih
//=====================================================
float jarakTerdekatKNN(float R, float G, float B)
{
  float terkecil = euclidean(R, G, B, 0);

  for (int i = 1; i < JUMLAH_DATA; i++)
  {
    float d = euclidean(R, G, B, i);
    if (d < terkecil) terkecil = d;
  }

  return terkecil;
}


//=====================================================
// Fungsi mematikan seluruh LED indikator
//=====================================================
void matikanLED()
{
  digitalWrite(LED_MERAH,LOW);
  digitalWrite(LED_HIJAU,LOW);
  digitalWrite(LED_BIRU,LOW);
}


//=====================================================
// Fungsi menyalakan LED sesuai warna hasil klasifikasi
//=====================================================
void tampilkanLED(String warna)
{
  matikanLED();

  if(warna=="MERAH")
    digitalWrite(LED_MERAH,HIGH);

  else if(warna=="HIJAU")
    digitalWrite(LED_HIJAU,HIGH);

  else if(warna=="BIRU")
    digitalWrite(LED_BIRU,HIGH);
}


//=====================================================
// Fungsi menggerakkan servo pemisah berdasarkan warna
//=====================================================
void gerakServo(String warna)
{
  int sudutDorong = SUDUT_LURUS;
  int delayTahan = 0;
  bool perluDorong = false;

  if(warna=="MERAH")
  {
    sudutDorong = SUDUT_DORONG_MERAH;
    delayTahan  = DELAY_TAHAN_MERAH;
    perluDorong = true;
  }
  else if(warna=="BIRU")
  {
    sudutDorong = SUDUT_DORONG_BIRU;
    delayTahan  = DELAY_TAHAN_BIRU;
    perluDorong = true;
  }

  if(perluDorong)
  {
    servoPemisah.write(sudutDorong);
    delay(DELAY_GERAK_AWAL);

    delay(delayTahan);

    servoPemisah.write(SUDUT_LURUS);
    delay(DELAY_KEMBALI);
  }
  else
  {
    servoPemisah.write(SUDUT_LURUS);
    delay(DELAY_GERAK_AWAL + DELAY_TAHAN_MERAH + DELAY_KEMBALI);
  }
}


//=====================================================
// Fungsi mengirim status sistem ke aplikasi Blynk
//=====================================================
void kirimStatusBlynk(String s)
{
  Blynk.virtualWrite(V_STATUS, s);
}


//=====================================================
// Fungsi memperbarui jumlah objek pada Blynk
//=====================================================
void perbaruiCounterBlynk()
{
  unsigned long total = jumlahMerah + jumlahHijau + jumlahBiru;

  Blynk.virtualWrite(V_COUNT_MERAH, jumlahMerah);
  Blynk.virtualWrite(V_COUNT_HIJAU, jumlahHijau);
  Blynk.virtualWrite(V_COUNT_BIRU, jumlahBiru);
  Blynk.virtualWrite(V_COUNT_TOTAL, total);
}


//=====================================================
// Fungsi mengirim nilai RGB dan jarak secara realtime
//=====================================================
void kirimRealtimeKeBlynk()
{
  Blynk.virtualWrite(V_R, gAvgR);
  Blynk.virtualWrite(V_G, gAvgG);
  Blynk.virtualWrite(V_B, gAvgB);
  Blynk.virtualWrite(V_JARAK, gJarakTerdekat);
}

BLYNK_WRITE(V_RESET)
{
  int ditekan = param.asInt();

  if (ditekan)
  {
    jumlahMerah = 0;
    jumlahHijau = 0;
    jumlahBiru = 0;

    perbaruiCounterBlynk();
    Serial.println("COUNTER DI-RESET DARI APP BLYNK");
  }
}

BLYNK_CONNECTED()
{
  Blynk.virtualWrite(V_WARNA, "-");
  Blynk.virtualWrite(V_STATUS, "SIAP");
  perbaruiCounterBlynk();
}


//=====================================================
// Fungsi inisialisasi seluruh perangkat
//=====================================================
void setup()
{
  Serial.begin(115200);

  pinMode(PIN_S0,OUTPUT);
  pinMode(PIN_S1,OUTPUT);
  pinMode(PIN_S2,OUTPUT);
  pinMode(PIN_S3,OUTPUT);
  pinMode(PIN_OUT,INPUT);

  digitalWrite(PIN_S0,HIGH);
  digitalWrite(PIN_S1,LOW);

  pinMode(PIN_IR,INPUT);

  pinMode(LED_MERAH,OUTPUT);
  pinMode(LED_HIJAU,OUTPUT);
  pinMode(LED_BIRU,OUTPUT);

  servoPemisah.attach(PIN_SERVO);
  servoPemisah.write(SUDUT_LURUS);

  matikanLED();

  for(int i=0;i<BUFFER_SIZE;i++)
  {
    bufferR[i]=0;
    bufferG[i]=0;
    bufferB[i]=0;
  }

  Serial.println("Menghubungkan ke WiFi & Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, kirimRealtimeKeBlynk);

  Serial.println("SYSTEM READY");
}

//=====================================================
// Program utama yang berjalan terus-menerus
//=====================================================
void loop()
{
  Blynk.run();
  timer.run();

  int R=bacaKanal(LOW,LOW);
  int B=bacaKanal(LOW,HIGH);
  int G=bacaKanal(HIGH,HIGH);

  updateBuffer(R,G,B);

  float avgR=rataRata(bufferR);
  float avgG=rataRata(bufferG);
  float avgB=rataRata(bufferB);

  gAvgR = avgR;
  gAvgG = avgG;
  gAvgB = avgB;

  String hasil=klasifikasiKNN(avgR,avgG,avgB);

  gJarakTerdekat = jarakTerdekatKNN(avgR, avgG, avgB);

  if(!warnaSudahTerbaca)
  {
    if(hasil=="MERAH" || hasil=="HIJAU" || hasil=="BIRU")
    {
      warnaTersimpan=hasil;
      warnaSudahTerbaca=true;

      tampilkanLED(warnaTersimpan);

      Serial.print("WARNA : ");
      Serial.println(warnaTersimpan);

      Blynk.virtualWrite(V_WARNA, warnaTersimpan);
      kirimStatusBlynk("MEMBACA - TUNGGU IR");
    }
  }

  int ir=digitalRead(PIN_IR);

  if(ir==LOW && warnaSudahTerbaca && !prosesAktif)
  {
    prosesAktif=true;

    Serial.println("OBJECT DETECTED");

    kirimStatusBlynk("SORTIR JALAN");

    gerakServo(warnaTersimpan);

    if(warnaTersimpan=="MERAH") jumlahMerah++;
    else if(warnaTersimpan=="HIJAU") jumlahHijau++;
    else if(warnaTersimpan=="BIRU") jumlahBiru++;

    perbaruiCounterBlynk();

    warnaSudahTerbaca=false;
    warnaTersimpan="UNKNOWN";

    matikanLED();

    prosesAktif=false;

    kirimStatusBlynk("SIAP");
  }

  delay(100);
}

#include <stdlib.h>
// library dht11
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h> //memasukan library HX711

//konfigurasi hx771
#define DOUT  43 //mendefinisikan pin arduino yang terhubung dengan pin DT module HX711
#define CLK  45 //mendefinisikan pin arduino yang terhubung dengan pin SCK module HX711
HX711 scale(DOUT, CLK);
float calibration_factor = 483;

//konfigurasi ultrasonik
#define trigPin1 37 //Set Trigger HCSR04 di Pin digital 6
#define echoPin1 39 //Set Echo HCSR04 di Pin digital 5
#define MAX_DISTANCE 200 //Set jarak maksimal
NewPing ultrasonic1(trigPin1, echoPin1, MAX_DISTANCE);

//konfigurasi eps8266
#define SSID "empty" //nama wifinya
#define PASS "fajri1002" //password wifi routernya
#define Baud_Rate 115200
#define Delay_Time 5000
#define WRITE_APIKEY "MKD9ZQD99A9T26H8"//channel read apikey

//konfig dht11
DHT dht(4, DHT11); //Pin, Jenis DHT
//konfigurasi lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);


bool kirimdata; //cek status kirim data
int kipas = 25;
int lampu = 27;
int pinLdr = A1;
int suhu = 0;
int kelembaban = 0;
int bacaLdr = 0;
float unit = 0;
int ja = 31;
int tot = 0;
void setup(){
  pinMode(kipas, OUTPUT);
  pinMode(lampu, OUTPUT);
  pinMode(pinLdr, INPUT);
  pinMode(trigPin1, OUTPUT); //Set pin Trigger sebagai output
  pinMode(echoPin1, INPUT); //Set pin Echo sebagai input

  lcd.begin();
  lcd.backlight();
  // Konfigurasi Load Cell
  Serial.println("Memulai program kalibrasi pada sensor berat");
  Serial.println("Pastikan tidak ada beban diatas sensor");
  delay(1000);
  scale.set_scale();
  scale.tare(); // auto zero / mengenolkan pembacaan berat
  
  long zero_factor = scale.read_average(); //membaca nilai output sensor saat tidak ada beban
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);  

  // konfiguarasi esp8266  
  Serial.begin(Baud_Rate);
  Serial.println("AT");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AT");
  dht.begin();
  delay(1000);

  //sambungkan ke wifi
  Serial.println("AT+CWMODE=1");//mode wifi client
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("AT+CWMODE=1");
  delay(1000);

  String cmd = "AT+CWJAP=\"";//at command konek ke wifi router
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";//simpelnya ya gini AT+CWJAP="SSID","PASS";
  Serial.println(cmd);
  delay(1000);

  if(Serial.find("OK")){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("OK");
  }else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Gagal");
  }
  //end sambungan ke wifi

  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Selamat Datang");
  lcd.setCursor(0,1);
  lcd.print("Jamur Tiram");
  delay(1000);
}
void loop(){
  scale.set_scale(calibration_factor);
  unit = scale.get_units();
  if(unit < 0){
    unit = 0;
  }
  Serial.println(unit, 1);
  delay(500);
  
  // pembacaan suhu  
  kelembaban = dht.readHumidity();
  suhu = dht.readTemperature();
  
  if(suhu > 33){
    digitalWrite(kipas, LOW);
  }else{
    digitalWrite(kipas, HIGH);
  }  

  // pembacaan Ldr
  bacaLdr = analogRead(pinLdr);
  if(bacaLdr > 650){
    digitalWrite(lampu, LOW);
  }else{
    digitalWrite(lampu, HIGH);
  }  

  // pembacaan ultrasonik
  int US1 = ultrasonic1.ping_cm();
  tot = ja - US1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("S="); 
  lcd.print(suhu); 
  lcd.print(", LDR="); 
  lcd.print(bacaLdr);
  lcd.setCursor(0,1);
  lcd.print("T="); 
  lcd.print(tot);
  lcd.print(", L="); 
  lcd.print(unit);
  
    
  //inisialisasi alamat server
  String cmd = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80";//ip api.thingspeak.com
  Serial.println(cmd);
  delay(500);

  //coba kirim data
  String getStr = "GET /update?api_key=";
  getStr += WRITE_APIKEY;
  getStr += "&field1=";
  getStr += suhu;//silahkan ganti nilai sensor
  getStr += "&field2=";
  getStr += bacaLdr;//silahkan ganti nilai sensor
  getStr += "&field3=";
  getStr += tot;//silahkan ganti nilai sensor
  getStr += "&field4=";
  getStr += unit;//silahkan ganti nilai sensor
  getStr += "\r\n\r\n";
  
  //Serial.print("AT+CIPSEND=");
  //Serial.println(cmd.length());
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());

  Serial.println(cmd);
  
  if(Serial.find(">")){
    Serial.print(getStr);
  }else{
    Serial.println("AT+CIPCLOSE");
  }

//  if(Serial.find("OK")){
//    for(int a = 0; a < 5; a++){
//      digitalWrite(4,HIGH);
//      digitalWrite(3,LOW);
//      delay(200);
//      digitalWrite(4,LOW);
//      digitalWrite(3,HIGH);
//      delay(200);
//    }
//  }else{
//    for(int a = 0; a < 5; a++){
//      digitalWrite(4,HIGH);
//      digitalWrite(3,HIGH);
//      delay(200);
//      digitalWrite(4,LOW);
//      digitalWrite(3,LOW);
//      delay(200);
//    }
//  }
  
  //end coba kirim data
  delay(500);
}

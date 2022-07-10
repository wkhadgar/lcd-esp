#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h>                 
#include <LiquidCrystal.h>  
#define RS D6
#define EN D8
#define lcD4 D1
#define lcD5 D2
#define lcD6 D3
#define lcD7 D4
#define LEFT D0
#define SELECT D7
#define RIGHT D5

LiquidCrystal lcd(RS, EN, lcD4, lcD5, lcD6, lcD7);

char ssid[] = "Paulo 2G";  //wifi ssid
char password[] = "10324966";   //wifi password

WiFiUDP ntpUDP;


NTPClient timeClient(ntpUDP, "br.pool.ntp.org", -10800, 1000);

char Time[] = "HORA: 00:00:00";
char Date[] = "DATA: 00/00/2000";
byte last_second, sec, minutes, hh, dd, mm;
int yyyy;



void setup() {

  pinMode(LEFT, INPUT_PULLDOWN_16);
  pinMode(SELECT, INPUT);
  pinMode(RIGHT, INPUT);

  char l1_fixed[] = "";
  Serial.begin(115200);
  lcd.begin(16, 2);                 // Initialize 16x2 LCD Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(Time);
  lcd.setCursor(0, 1);
  lcd.print(Date);

  delay(800);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(l1_fixed);
  lcd.print(ssid);
  Serial.println();
  Serial.print("Conectando: ");
  Serial.println(ssid);
  lcd.setCursor(0, 1);
  Serial.print("Status: ");
  lcd.print("Status:         ");
  WiFi.begin(ssid, password);

  byte full_block[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
  lcd.createChar(0, full_block);
  int bar, name_offset, c = 0;
  int ssid_size = sizeof(ssid)/sizeof(char);
  int l1_fixed_size = sizeof(l1_fixed)/sizeof(char);

  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print(".");

    //Serial.println(name_offset);
    lcd.setCursor(l1_fixed_size-1, 0); //ssid name start
    name_offset %= ssid_size-(16-l1_fixed_size)-1;
    if (++c%2) lcd.print(ssid+name_offset++);
    
    bar %= 10;
    lcd.setCursor(6+bar, 1);
    if (!bar++) lcd.print(":          ");
    else lcd.write(byte(0));
    
    delay(400);
  }
  lcd.setCursor(7, 1);
  lcd.print(" OK!     ");
  Serial.println(" conectou!");
  timeClient.begin();
  delay(800);
  lcd.clear();
}
 

void loop() {

  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server

  sec = second(unix_epoch);
  if (last_second != sec) {
 

    minutes = minute(unix_epoch);
    hh   = hour(unix_epoch);
    dd    = day(unix_epoch);
    mm  = month(unix_epoch);
    yyyy   = year(unix_epoch);

    Time[13] = sec % 10 + 48;
    Time[12] = sec / 10 + 48;
    Time[10]  = minutes % 10 + 48;
    Time[9]  = minutes / 10 + 48;
    Time[7]  = hh % 10 + 48;
    Time[6]  = hh / 10 + 48;

 

    Date[6]  = dd / 10 + 48;
    Date[7]  = dd % 10 + 48;
    Date[9]  = mm / 10 + 48;
    Date[10]  = mm  % 10 + 48;
    Date[14] = (yyyy / 10) % 10 + 48;
    Date[15] = yyyy % 10 % 10 + 48;

    //Serial.println(Time);
    //Serial.println(Date);

    lcd.setCursor(0, 0);
    lcd.print(Time);
    lcd.setCursor(0, 1);
    lcd.print(Date);
    last_second = sec;

  }
  if (digitalRead(LEFT)) Serial.println(-1);
  else if (digitalRead(SELECT)) {
    Serial.println(0);
    lcd.setCursor(0, 1);
    lcd.print("!!!!SELECTED!!!!");
  }
  else if (digitalRead(RIGHT)) Serial.println(1);
  else Serial.println(0.5);
  delay(10);
}
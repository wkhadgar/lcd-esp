#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h>                 
#include <LiquidCrystal.h>  
LiquidCrystal lcd(D6, D5, D1, D2, D3, D4); 


char ssid[] = "adalovelace";  //wifi ssid
char password[] = "geladotrincando";   //wifi password

WiFiUDP ntpUDP;


NTPClient timeClient(ntpUDP, "br.pool.ntp.org", -10800, 1000);

char Time[] = "HORA: 00:00:00";
char Date[] = "DATA: 00/00/2000";
byte last_second, sec, minutes, hh, dd, mm;
int yyyy;



void setup() {
  int bar = 0;
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
  lcd.print("Conectando:");
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
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print(".");
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

    Serial.println(Time);
    Serial.println(Date);

    lcd.setCursor(0, 0);
    lcd.print(Time);
    lcd.setCursor(0, 1);
    lcd.print(Date);
    last_second = sec;

  }
  delay(500);
}
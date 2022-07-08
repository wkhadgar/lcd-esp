#include <LiquidCrystal.h>

const int RS = D6, EN = D5, lcD4 = D1, lcD5 = D2, lcD6 = D3, lcD7 = D4;   
LiquidCrystal lcd(RS, EN, lcD4, lcD5, lcD6, lcD7);
int t;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(115200); 
  delay(500); //rst delay
  Serial.print("\nTeste"); 

  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(500);
  }

  lcd.print("time:");
  Serial.println(" ok!");
}

void loop() {
  
  lcd.setCursor(0, 1);
  t = (millis() / 1000)-5;
  lcd.print(t);
  delay(1000);
  Serial.println(t);
}
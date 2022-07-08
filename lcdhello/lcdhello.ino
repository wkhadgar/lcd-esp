#include <LiquidCrystal.h>

const int RS = D6, EN = D5, lcD4 = D1, lcD5 = D2, lcD6 = D3, lcD7 = D4;   
LiquidCrystal lcd(RS, EN, lcD4, lcD5, lcD6, lcD7);
int t=0;

char easteregg[] = "LINHA 1";
char easteregg1[] = "1 ";
char easteregg2[] = "2 ";
char easteregg3[] = "3";
char* cycle[3] = { easteregg1, easteregg2, easteregg3 };

void setup() {
  lcd.begin(16, 2);
  Serial.begin(115200); 
  delay(500); //rst delay
  Serial.print("\nTeste"); 

  lcd.print(easteregg);
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    lcd.print(".");
    delay(500);
  }
  Serial.println(" ok!");
}

void loop() {
  
  lcd.setCursor(0, 1);
  for(t = 0; t<3; t++)  {
    lcd.print(cycle[t]);
    Serial.println(cycle[t]);
    delay(1000);
  }
  lcd.setCursor(0, 1);
  lcd.print("               ");
  delay(500);
}
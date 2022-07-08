#include <LiquidCrystal.h>
const int RS = D6, EN = D5, lcD4 = D1, lcD5 = D2, lcD6 = D3, lcD7 = D4;   
LiquidCrystal lcd(RS, EN, lcD4, lcD5, lcD6, lcD7);

void setup() {
                         // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(115200); 
  Serial.print("Teste"); 
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(500);
  }
                      // Print a message to the LCD.
  lcd.print("hello, world!");
  Serial.println(" ok!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}
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

byte arrow[8] = {B00000, B00000, B00100, B00110, B11111, B00110, B00100, B00000};
byte full_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte dynamic_block[8];

lcd.createChar(0, arrow);

byte shift_bits(byte* source, int shift) {
    byte result[8];
    for (int i = 0; i < 8; i++) {
        result[i] = source[i] << shift; 
    }
    return result;
}

void setup()    {
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(SELECT, INPUT);

    
}

int pos = 0;
void loop()     {
    lcd.print("Teste:");
    pos %= 16;
    lcd.setCursor(pos, 1);
    
    for (int i = 0; i < 5; i++) {
        dynamic_block = shift_bits(full_block, i);
        lcd.createChar(1, dynamic_block);
        lcd.write(byte(1)); 
        delay(200);   
    }
    pos++;
};
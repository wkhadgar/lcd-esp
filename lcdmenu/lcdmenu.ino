#include <LiquidCrystal.h>
#define lcdRS D6
#define lcdEN D8
#define lcD4 D1
#define lcD5 D2
#define lcD6 D3
#define lcD7 D4
#define LEFT D0
#define SELECT D7
#define RIGHT D5

LiquidCrystal lcd(lcdRS, lcdEN, lcD4, lcD5, lcD6, lcD7);

byte arrow[8] = {B00000, B10000, B11000, B11100, B11000, B10000, B00000, B00000};
byte full_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111};
byte dynamic_block[8];

const short int menu_size = 5;
//menu options (limited to given size)
char option_1[] = "OPTION 1";
char option_2[] = "Option 2";
char option_3[] = "OPTION 3";
char option_4[] = "Option 4";
char option_5[] = "OPTION 5";
char* menu[menu_size] = {option_1, option_2, option_3, option_4, option_5};

bool arrow_low = 0;
short int menu_pos = 0;
short int current_selection = 0;
short int cursor_pos = 0;

void setup()    {
    lcd.begin(16, 2);      
    lcd.clear();
    lcd.createChar(0, arrow);

    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(SELECT, INPUT);

    lcd.setCursor(0, 0);
    lcd.print(" Iniciando Menu");

    //aesthetic smooth fake load 
    for (int p = 0; p < 16; p++)    {
            for (int i = 0; i < 5; i++) {
            shift_bits(full_block, dynamic_block, 5-i);
            lcd.createChar(1, dynamic_block);
            lcd.setCursor(p, 1);
            lcd.write(byte(1)); 
            delay(19);   //1.5s = 1500ms/(16*5) = 19ms
        }
        lcd.setCursor(p, 1);
        lcd.write(byte(255));
    }
    lcd.clear();
}

void loop()     {
    //menu movement
    if (digitalRead(LEFT))  {
        if (arrow_low) arrow_low = !arrow_low;
        else menu_pos--;
        while (digitalRead(LEFT)); //button realease wait
    }
    else if (digitalRead(RIGHT)) {
        if (!arrow_low) arrow_low = !arrow_low;
        else menu_pos++;
        while (digitalRead(RIGHT)); //button realease wait
    }

    //menu movement limits
    if (!arrow_low && menu_pos < 0) {
        menu_pos = 0;
    }
    else if (arrow_low && menu_pos > menu_size-2) {
        menu_pos = menu_size-2;
    }

    current_selection = menu_pos+arrow_low;
    cursor_pos = 0;
    
    if (!arrow_low) {lcd.setCursor(cursor_pos, 1); lcd.print(" "); lcd.setCursor(cursor_pos++, 0); lcd.write(byte(0));} //arrow print
    else {lcd.setCursor(cursor_pos, 0); lcd.print(" "); lcd.setCursor(cursor_pos++, 1); lcd.write(byte(0));}

    lcd.setCursor(cursor_pos, 0); lcd.print(menu[menu_pos]);
    lcd.setCursor(cursor_pos, 1); lcd.print(menu[menu_pos + 1]);

};

void shift_bits(byte* source, byte* dest, int shift) {
    for (int i = 0; i < 8; i++) {
        dest[i] = source[i] << shift; 
    }
    return;
}
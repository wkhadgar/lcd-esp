/*
Versão 2
Usando um rotary encoder no lugar dos 3 botões, ainda em pulldown
*/

#include <LiquidCrystal.h>
#define lcdRS D6
#define lcdEN D8
#define lcD4 D1
#define lcD5 D2
#define lcD6 D3
#define lcD7 D4
#define lcdBrightness_pin 3 //RX PWM
#define LEFT D7
#define SELECT D5
#define RIGHT D0

LiquidCrystal lcd(lcdRS, lcdEN, lcD4, lcD5, lcD6, lcD7);

byte full_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111}; //referencia pro load
byte arrow[8] = {B00000, B10000, B11000, B11100, B11000, B10000, B00000, B00000}; //setinha
byte done[8] = {B00000, B00000, B00001, B00011, B10110, B11100, B01000, B00000}; //ok
byte dynamic_block[8];

//menu options (limited to given size)
const unsigned char menu_size = 5;
char option_0[] = "OPTION 1"; 
char option_1[] = "Option 2";
char option_2[] = "OPTION 3";
char option_3[] = "Option 4";
char option_4[] = "Brilho da tela";
short int menu_op_value[menu_size] = {0};
short int value_preview = 0;
char* menu[menu_size] = {option_0, option_1, option_2, option_3, option_4};

bool on_menu = 1;
bool is_arrow_down = 0;
unsigned char menu_pos = 0;
unsigned char current_selection = 0;
unsigned char cursor_pos = 0;
bool act_state, last_state = false;

void setup()    {
    lcd.begin(16, 2);      
    lcd.clear();
    lcd.createChar(0, arrow);
    lcd.createChar(2, done);

    pinMode(lcdBrightness_pin, FUNCTION_3);
    pinMode(lcdBrightness_pin, OUTPUT);
    menu_op_value[4] = 5;
    analogWrite(lcdBrightness_pin, menu_op_value[4]);
    //entrdas do rotary encoder
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(SELECT, INPUT);

    lcd.setCursor(0, 0);
    lcd.print(" Iniciando Menu");

    //fake load suave estetico
    for (unsigned char p = 1; p < 15; p++)    {
            for (unsigned char i = 0; i < 5; i++) {
            shift_bits(full_block, dynamic_block, 5-i, 0);
            lcd.createChar(1, dynamic_block);
            lcd.setCursor(p, 1);
            lcd.write(byte(1)); 
            delay(19);   //1.5s = 1500ms/(16*5) = 19ms
        }
        lcd.setCursor(p, 1);
        lcd.write(byte(255));
    }
    lcd.clear();
    last_state = digitalRead(LEFT);
}

void loop()     {
    act_state = digitalRead(LEFT);
    //menu movement
    if (on_menu)    {
        if ((act_state!=last_state) & act_state)  {
            if (!digitalRead(RIGHT)) { //anti horario
                if (is_arrow_down) is_arrow_down = !is_arrow_down;
                else !menu_pos?:menu_pos--; //guard pra signed assignment e limite superior do menu
            }
            else { //horario
                if (!is_arrow_down) is_arrow_down = !is_arrow_down;
                else (menu_pos>(menu_size-3))?:menu_pos++;
            }
        }

        last_state = act_state; //remember the change (if there was one)
    }
    //value changing
    else    {
        if ((act_state!=last_state) & act_state)    {
            if (!digitalRead(RIGHT))   {
                value_preview--;
            }
            else {
                value_preview+=5;
            }
        }

        last_state = act_state;
    }

    current_selection = menu_pos+is_arrow_down; //qual option ta selecionada agora
    cursor_pos = 0;
    
    if (on_menu)    {
        if (!is_arrow_down) {lcd.setCursor(cursor_pos, 1); lcd.print(" "); lcd.setCursor(cursor_pos++, 0); lcd.write(byte(0));} //arrow print
        else {lcd.setCursor(cursor_pos, 0); lcd.print(" "); lcd.setCursor(cursor_pos++, 1); lcd.write(byte(0));}

        lcd.setCursor(cursor_pos, 0); lcd.print(menu[menu_pos]); lcd.print(" : "); lcd.print(menu_op_value[menu_pos]); lcd.print("        "); //linha 1 print
        lcd.setCursor(cursor_pos, 1); lcd.print(menu[menu_pos + 1]); lcd.print(" : "); lcd.print(menu_op_value[menu_pos + 1]); lcd.print("        "); //linha 2 print

        if (digitalRead(SELECT))    { //selecionou alguma op do menu principal
            shift_bits(arrow, dynamic_block, 0, 0); //salva o arrow original no bloco dinamico
            shift_bits(dynamic_block, arrow, 2, 1); //seta anda pra direita 2 bit
            lcd.createChar(0, arrow); //seta na memoria
            
            while (digitalRead(SELECT)) delay(100); //espera soltar
            shift_bits(dynamic_block, arrow, 0, 0); //volta ao normal
            lcd.createChar(0, arrow); //set

            on_menu = !on_menu;
            value_preview = menu_op_value[current_selection];
        }
    }
    else { //sub-menu de modificar valores
        lcd.setCursor(cursor_pos, 0); lcd.print(" "); lcd.print(menu[menu_pos+is_arrow_down]); lcd.print("       ");//linha 1 print
        lcd.setCursor(cursor_pos, 1); lcd.write(byte(0)); lcd.print("      "); lcd.print(value_preview); lcd.print("        "); //linha 2 print

        if (current_selection == 4) {
            (value_preview>100)?value_preview=100:(value_preview<0)?value_preview=0:value_preview;
            menu_op_value[current_selection] = value_preview;
            analogWrite(lcdBrightness_pin, ((menu_op_value[4]*255)/100));
            lcd.setCursor(cursor_pos, 1); lcd.write(byte(0)); lcd.print("      "); lcd.print(value_preview); lcd.print("%        "); //linha 2 print
        }

        if (digitalRead(SELECT))    {
            menu_op_value[current_selection] = value_preview; //salva a config
            lcd.setCursor(cursor_pos, 1); lcd.write(byte(2)); //da o simbolo de ok;
            while (digitalRead(SELECT)) delay(500);
            on_menu = !on_menu; //sai do sub-menu
        }
    }

};

void shift_bits(byte* source, byte* dest, unsigned char shift, bool right) {
    if (!right) {
        for (unsigned char i = 0; i < 8; i++) {
           dest[i] = source[i] << shift; 
        }
    }
    else {
        for (unsigned char i = 0; i < 8; i++) {
           dest[i] = source[i] >> shift; 
        }
    }
    return;
}
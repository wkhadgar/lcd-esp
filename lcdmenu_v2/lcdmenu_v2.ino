/*
Versão 2
Usando um rotary encoder no lugar dos 3 botões, ainda em pulldown
*/

#include <LiquidCrystal.h>
#include <EEPROM.h>
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
#define DEBOUNCE_ROTARY_MS 10
#define LCD_ROWS 2
#define LCD_COLS 16

LiquidCrystal lcd(lcdRS, lcdEN, lcD4, lcD5, lcD6, lcD7);

byte full_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111}; //referencia pro load
byte arrow[8] = {B00000, B10000, B11000, B11100, B11000, B10000, B00000, B00000}; //setinha
byte saving[8] = {B00000, B11110, B11111, B10001, B10001, B11111, B11111, B00000};
byte done[8] = {B00000, B00000, B00001, B00011, B10110, B11100, B01000, B00000}; //ok
byte dynamic_block[8];

//menu options (limited to given size)
const unsigned char menu_size = 6;
char option_0[] = "OPTION 1"; 
char option_1[] = "Option 2";
char option_2[] = "OPTION 3";
char option_3[] = "Option 4";
char option_4[] = "Brilho da tela";
char option_5[] = "Salvar configs";
unsigned short int menu_op_value[menu_size] = {0};
unsigned short int value_preview = 0;
char* menu[menu_size] = {option_0, option_1, option_2, option_3, option_4, option_5};

bool on_menu = 1;
unsigned char arrow_pos = 0;
unsigned char menu_pos = 0;
unsigned char current_selection = 0;
unsigned char cursor_pos = 0;

void IRAM_ATTR rotary_IRQ();
void shift_bits(byte* source, byte* dest, unsigned char shift, bool right);

void setup()    {
    delay(200); //estabilizar a tensão
    
    lcd.begin(LCD_COLS, LCD_ROWS);      
    lcd.clear();
    lcd.createChar(0, arrow); //1 is the dynamic character
    lcd.createChar(2, done);
    lcd.createChar(3, saving);
    lcd.setCursor(0, 0);
    lcd.print(" Iniciando Menu");
    
    EEPROM.begin(menu_size*2);
    int stored_brightness = EEPROM.read(0);
    EEPROM.end();
    //lcd.print(stored_brightness);

    //screen led pwm
    pinMode(lcdBrightness_pin, FUNCTION_3);
    pinMode(lcdBrightness_pin, OUTPUT);
    menu_op_value[4] = stored_brightness; analogWrite(lcdBrightness_pin, (menu_op_value[4]*255)/100);
    //entradas do rotary encoder
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(SELECT, INPUT);
    attachInterrupt(digitalPinToInterrupt(LEFT), rotary_IRQ, RISING);
    
    //fake load suave estetico
    for (unsigned char p = 1; p < LCD_COLS-1; p++)    {
            for (unsigned char i = 0; i < 5; i++) {
            shift_bits(full_block, dynamic_block, 5-i, 0);
            lcd.createChar(1, dynamic_block);
            lcd.setCursor(p, 1);
            lcd.write(byte(1)); 
            delay(1500/(LCD_COLS*5));   //1.5s = 1500ms/(LCD_COLS*5)
        }
        lcd.setCursor(p, 1);
        lcd.write(byte(255));
    }
    lcd.clear();
}

void loop()     {
    //menu movement
    current_selection = menu_pos+arrow_pos; //qual option ta selecionada agora
    
    if (on_menu)    {
        for (int row = 0; row < LCD_ROWS; row++)  {
            lcd.setCursor(0, row)
            if (arrow_pos == row) lcd.write(byte(0));
            else lcd.print(" ");
            lcd.print(menu[menu_pos+row]); lcd.print(" : "); lcd.print(menu_op_value[menu_pos+row]); lcd.print("        "); //linha print
        }

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
        cursor_pos = 0;
        for (int row = 0; row < LCD_ROWS; row++)  {
            lcd.setCursor(cursor_pos, row);
            if (row == 0)   {
                lcd.print(" ");lcd.print(menu[current_selection]);
            }
            else if (row == 1) {
                lcd.write(byte(0)); 
                lcd.print("      "); lcd.print(value_preview); lcd.print("        ");
            }
            else lcd.print("                   ");
        }

        //especific configs
        if (current_selection == 4) { //brightness
            (value_preview>100)?value_preview=100:(value_preview<0)?value_preview=0:value_preview;
            menu_op_value[current_selection] = value_preview;
            analogWrite(lcdBrightness_pin, ((menu_op_value[4]*255)/100));
            lcd.setCursor(cursor_pos, 1); lcd.print("%"); lcd.print("      "); lcd.print(value_preview); lcd.print("        "); //linha 2 print
        }

        if (current_selection == 5) { //save 
            lcd.setCursor(cursor_pos, 1); lcd.write(byte(3)); lcd.print("  Salvando..."); lcd.print("         "); //linha 2 print
            uint8_t eeprom_value;
            EEPROM.begin(menu_size*2);
            eeprom_value = EEPROM.read(0);
            uint8_t brightness = menu_op_value[4];
            if (eeprom_value != menu_op_value[4])   {
                EEPROM.write(0, brightness);
                delay(1500); //simular espera do save
            }

            EEPROM.end(); delay(500);
            lcd.setCursor(cursor_pos, 1); lcd.write(byte(2)); //ok
            delay(1000);
            on_menu = !on_menu; //sai automaticamente da tela de salvamento
        }
        //só entra aqui se não for save config
        else if (digitalRead(SELECT))    { 
            menu_op_value[current_selection] = value_preview; //salva o setting
            lcd.setCursor(cursor_pos, 1); lcd.write(byte(2)); //da o simbolo de ok;
            while (digitalRead(SELECT)) delay(1000);
            on_menu = !on_menu; //sai do sub-menu
        }
    }

};

void shift_bits(byte* source, byte* dest, unsigned char shift, bool right)  {
    if (!right) {
        for (unsigned char i = 0; i < 8; i++)   {
           dest[i] = source[i] << shift; 
        }
    }
    else    {
        for (unsigned char i = 0; i < 8; i++)   {
           dest[i] = source[i] >> shift; 
        }
    }
    return;
}

void IRAM_ATTR rotary_IRQ()   {
    static unsigned long last_Interrupt = 0;
    unsigned long interrupt_now = millis();
    if (on_menu)    {
        if ((interrupt_now - last_Interrupt) > DEBOUNCE_ROTARY_MS)  { //debounce sem delay()
            if (!digitalRead(RIGHT))    { //anti horario
                if (arrow_pos) arrow_pos--;
                else !menu_pos?:menu_pos--; //guard pra signed assignment e limite superior do menu
            }
            else    { //horario
                if (arrow_pos<LCD_ROWS-1) arrow_pos++;
                else (menu_pos>(menu_size-3))?:menu_pos++;
            }
        }

        last_Interrupt = interrupt_now; //remember the change (if there was one)
    }
    else    {
        if ((interrupt_now - last_Interrupt) > DEBOUNCE_ROTARY_MS)    { //debounce sem delay()
            if (!digitalRead(RIGHT))   {
                (value_preview<1)?:value_preview--; //guard pra signed assignment
            }
            else {
                value_preview++;
            }
        }

        last_Interrupt = interrupt_now;
    }
    while(digitalRead(LEFT)); //espara cair
}
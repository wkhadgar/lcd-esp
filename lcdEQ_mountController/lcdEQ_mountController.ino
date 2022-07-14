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

#define RATE_ROTATION_MS 1006 //mudar
#define DEBOUNCE_ROTARY_MS 10

#define LCD_COLS 16
#define LCD_ROWS 2

LiquidCrystal lcd(lcdRS, lcdEN, lcD4, lcD5, lcD6, lcD7);

byte full_block[8] = {B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111}; //referencia pro load
byte arrow[8] = {B00000, B10000, B11000, B11100, B11000, B10000, B00000, B00000}; //setinha
byte saving[8] = {B00000, B11110, B11111, B10001, B10001, B11111, B11111, B00000}; //card
byte lock[8] = {B01110, B10001, B10001, B11111, B11011, B11011, B11111, B00000}; //cadeado
byte done[8] = {B00000, B00000, B00001, B00011, B10110, B11100, B01000, B00000}; //ok
byte dynamic_block[8];

//menu options (limited to given size)
const unsigned char menu_size = 6;
char option_0[] = "DEC "; 
char option_1[] = "R.A.";
char option_2[] = "Hemisferio";
char option_3[] = "Modo automatico";
char option_4[] = "Brilho da tela";
char option_5[] = "Salvar configs"; //precisa ser a ultima
unsigned short int menu_op_value[menu_size] = {0};
unsigned short int value_preview = 0;
unsigned short int lock_value;
char* menu[menu_size] = {option_0, option_1, option_2, option_3, option_4, option_5};

bool on_menu = 1;
bool auto_mode = 0;
unsigned char arrow_row = 0;
unsigned char menu_top_row = 0;
unsigned char current_selection = 0;
unsigned char current_text_row;

unsigned long int time_last = 0;
unsigned long int time_now = 0;

void IRAM_ATTR rotary_IRQ();
void shift_bits(byte* source, byte* dest, unsigned char shift, bool right);

void setup()    {
    delay(200); //estabilizar a tensão
    
    lcd.begin(LCD_COLS, LCD_ROWS);      
    lcd.clear();
    lcd.createChar(1, arrow); //0 is the dynamic character
    lcd.createChar(2, done);
    lcd.createChar(3, saving);
    lcd.createChar(4, lock);
    lcd.setCursor(0, 0);
    lcd.print(" Iniciando Menu");
    
    EEPROM.begin(menu_size*2);
    int stored_brightness = EEPROM.read(0);
    int stored_hemisphere = EEPROM.read(1);
    EEPROM.end();

    //screen led pwm
    pinMode(lcdBrightness_pin, FUNCTION_3);
    pinMode(lcdBrightness_pin, OUTPUT);
    menu_op_value[2] = stored_hemisphere;
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
            lcd.createChar(0, dynamic_block);
            lcd.setCursor(p, 1); //draw over
            lcd.write(byte(0)); 
            delay(1500/(LCD_COLS*5));   //1.5s = 1500ms/(LCD_COLS*5)
        }
        lcd.setCursor(p, 1);
        lcd.write(byte(255));
    }
    lcd.clear();

    time_last = millis();
}

void loop()     {
    //automode settings
    time_now = millis();
    if ((time_now - time_last > RATE_ROTATION_MS) && auto_mode)    {
        menu_op_value[1]++; //R.A. increment over time
        time_last = time_now;
    }
    
    //menu movement
    current_selection = menu_top_row+arrow_row; //qual option ta selecionada agora

    if (on_menu)    {
        
        for (int row = 0; row < LCD_ROWS; row++)  { //main-menu print
            current_text_row = menu_top_row+row;
            lcd.setCursor(0, row);
            if (arrow_row == row) lcd.write(byte(1));
            else lcd.print(" ");
            switch (current_text_row)   {
                case 0: { //dec
                    lcd.print(menu[current_text_row]); lcd.print(" : "); lcd.print(menu_op_value[current_text_row]/60); lcd.write(byte(223)); lcd.print(menu_op_value[current_text_row]%60); lcd.print("'  ");
                    if (auto_mode) {lcd.setCursor(LCD_COLS-1, row); lcd.write(byte(4));}
                    else lcd.print("    ");
                } break;
                case 1: { //ra
                    lcd.print(menu[current_text_row]); lcd.print(" : "); lcd.print(menu_op_value[current_text_row]/60); lcd.print("h"); lcd.print(menu_op_value[current_text_row]%60); lcd.print("m  ");
                    if (auto_mode) {lcd.setCursor(LCD_COLS-1, row); lcd.write(byte(4));}
                    else lcd.print("    ");
                } break;
                case 2: { //hemisphere
                    lcd.print(menu[current_text_row]); lcd.print(": "); menu_op_value[current_text_row]?lcd.print("N    "):lcd.print("S    ");
                } break;
                default:    {
                    lcd.print(menu[current_text_row]); lcd.print(" : "); lcd.print(menu_op_value[current_text_row]); lcd.print("        "); //linha print
                }
            }
        }

        if (digitalRead(SELECT))    { //selecionou alguma op do menu principal
            shift_bits(arrow, dynamic_block, 0, 0); //salva o arrow original no bloco dinamico
            shift_bits(dynamic_block, arrow, 2, 1); //seta anda pra direita 2 bit
            lcd.createChar(1, arrow); //seta na memoria
            while (digitalRead(SELECT)) delay(150); //espera soltar e dá o tempo minimo da animação
            shift_bits(dynamic_block, arrow, 0, 0); //volta ao normal
            lcd.createChar(1, arrow); //set

            on_menu = !on_menu;
            value_preview = menu_op_value[current_selection];
            lock_value = value_preview;
        }
    }

    else { //sub-menu de modificar valores
        lcd.setCursor(0, 0);
        lcd.print(" "); lcd.print(menu[current_selection]); lcd.print("           "); //titulo submenu

        //especific configs
        switch (current_selection) {
            case 0: { //dec
                if (auto_mode)  (value_preview==lock_value)?:value_preview=lock_value; //lock
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.write(byte(223)); lcd.print(value_preview%60); lcd.print("'    ");
                lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
                if (auto_mode) lcd.write(byte(4)); 
                else lcd.print("    ");
            } break;
            case 1: { //right ascendence
                if (auto_mode)  (value_preview==menu_op_value[1])?:value_preview=menu_op_value[1]; //lock on auto ra
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.print("h"); lcd.print(value_preview%60); lcd.print("m    ");
                lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
                if (auto_mode) lcd.write(byte(4)); 
                else lcd.print("    ");
            } break;
            case 2: { //hemisphere
                (value_preview>0)?value_preview=1:value_preview; //boolean
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); value_preview?lcd.print("Norte         "):lcd.print(" Sul        ");
            } break;
            case 3: { //auto-mode
                (value_preview>0)?value_preview=1:value_preview; //boolean
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); value_preview?lcd.print("ON        "):lcd.print("OFF       ");
            } break;
            case 4: { //brightness
                (value_preview>100)?value_preview=100:(value_preview<0)?value_preview=0:value_preview;
                analogWrite(lcdBrightness_pin, ((value_preview*255)/100));
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("%        "); //linha 2 print
            } break;    
            case (menu_size-1):     { //save (ultima op)
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(3)); lcd.print("  Salvando..."); lcd.print("         "); //linha 2 print
                uint8_t eeprom_brightness, eeprom_H;
                EEPROM.begin(menu_size*2);
                eeprom_brightness = EEPROM.read(0);
                eeprom_H = EEPROM.read(1);
                uint8_t brightness = menu_op_value[4];
                uint8_t hemisphere = menu_op_value[2];
                if (eeprom_brightness != brightness)   {
                    EEPROM.write(0, brightness);
                }
                if (eeprom_H != hemisphere) {
                    EEPROM.write(1, hemisphere);
                }
                delay(2000); //simular espera do save
                EEPROM.end();
                lcd.setCursor(0, 1); lcd.write(byte(2)); //ok
                delay(1000);
                on_menu = !on_menu; //sai automaticamente da tela de salvamento
            } break;  
            default:    { //submenu generico
                for (int row = 1; row < LCD_ROWS; row++)  {
                    lcd.setCursor(0, row);
                    if (row == LCD_ROWS-1){ 
                        lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("        ");
                    }
                    else lcd.print("                  ");
                }
            }
        }
        
        if (digitalRead(SELECT) && (menu_size-current_selection-1))    { //press sem ser no save
            time_last = millis();
            menu_op_value[current_selection] = value_preview; //salva o setting
            lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(2)); //da o simbolo de ok;
            while (digitalRead(SELECT)) delay(800);
            on_menu = !on_menu; //sai do sub-menu
            if (menu_op_value[3])   {auto_mode = true; time_last = millis();} else auto_mode = false; //inicia o modo automatico
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
                if (arrow_row) arrow_row--;
                else !menu_top_row?:menu_top_row--; //guard pra signed assignment e limite superior do menu
            }
            else    { //horario
                if (arrow_row<LCD_ROWS-1) arrow_row++;
                else (menu_top_row>(menu_size-3))?:menu_top_row++;
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
                value_preview+=5;
            }
        }

        last_Interrupt = interrupt_now;
    }
    while(digitalRead(LEFT)); //espara cair
}
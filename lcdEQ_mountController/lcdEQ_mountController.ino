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

#define ROT_A D7
#define SELECT D5
#define ROT_B D0

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
enum menu_option{
    DEC_           = 0,
    RA             = 1,
    Hemisphere     = 2,
    automatic_mode = 3,
    brilho_tela    = 4,
    tempo_tela     = 5,
    save_configs   = 6
};
const char* menu_str[] = {
    "DEC",
    "R.A",
    "Hemisferio", 
    "Modo automatico", 
    "Brilho da tela", 
    "Luz da tela",
    "Salvar configs"
};
const uint8_t menu_size = 7;
unsigned short int menu_op_value[menu_size] = {0};

unsigned short int value_preview = 0;
unsigned short int lock_value;

bool on_menu = 1;
bool auto_mode = 0;
uint8_t arrow_row = 0;
uint8_t menu_top_row = 0;
uint8_t current_selection;
uint8_t current_text_row;

unsigned long int action_last_time = 0;
unsigned long int ra_last_time = 0;
unsigned long int time_now = 0;

void IRAM_ATTR rotary_IRQ();
void shift_bits(byte* source, byte* dest, uint8_t shift, bool right);

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
    
    //data recover
    EEPROM.begin(menu_size);
    int stored_brightness = EEPROM.read(0);
    int stored_light_time = EEPROM.read(1);
    int stored_hemisphere = EEPROM.read(2);
    EEPROM.end();
    menu_op_value[brilho_tela] = stored_brightness; 
    menu_op_value[Hemisphere]  = stored_hemisphere;
    menu_op_value[tempo_tela]  = stored_light_time;

    //screen led pwm
    pinMode(lcdBrightness_pin, FUNCTION_3);
    pinMode(lcdBrightness_pin, OUTPUT);
    analogWrite(lcdBrightness_pin, (menu_op_value[brilho_tela]*255)/100);
    
    //rotary encoder
    pinMode(ROT_A, INPUT);
    pinMode(ROT_B, INPUT);
    pinMode(SELECT, INPUT);
    attachInterrupt(digitalPinToInterrupt(ROT_A), rotary_IRQ, RISING);
    

    //fake load suave estetico
    for (uint8_t p = 1; p < LCD_COLS-1; p++)    {
        for (uint8_t i = 0; i < 5; i++) {
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

    ra_last_time = millis();
    action_last_time = millis();
}

bool sleeping = false;
bool wake     = false;
void loop()     {
    time_now = millis();
    
    //wait screen 
    if (wake)  {
        analogWrite(lcdBrightness_pin, (menu_op_value[brilho_tela]*255)/100);
        action_last_time = time_now;
        wake     = false;
        sleeping = false;
    }
    else if (!sleeping && (time_now - action_last_time) >= (menu_op_value[tempo_tela]*1000))  { //sec to ms
        analogWrite(lcdBrightness_pin, 0);
        sleeping = true;
    }

    //automode settings
    if ((time_now - ra_last_time >= RATE_ROTATION_MS) && auto_mode)    {
        menu_op_value[RA]++; //R.A. increment over time
        ra_last_time = time_now;
    }
    
    //menu movement
    current_selection = menu_top_row+arrow_row; //qual option ta selecionada agora
    if (on_menu)    {
        
        for (uint8_t row = 0; row < LCD_ROWS; row++)  { //main-menu print
            current_text_row = menu_top_row+row;
            lcd.setCursor(0, row);
            if (arrow_row == row) lcd.write(byte(1));
            else lcd.print(" ");
            switch (current_text_row)   {
                case DEC_: {
                    lcd.print(menu_str[DEC_]); lcd.print(" : "); lcd.print(menu_op_value[DEC_]/60); lcd.write(byte(223)); lcd.print(menu_op_value[DEC_]%60); lcd.print("'   ");
                    if (auto_mode) {lcd.setCursor(LCD_COLS-1, row); lcd.write(byte(4));}
                    else lcd.print("    ");
                } break;
                case RA: { 
                    lcd.print(menu_str[RA]); lcd.print(" : "); lcd.print(menu_op_value[RA]/60); lcd.print("h"); lcd.print(menu_op_value[RA]%60); lcd.print("m   ");
                    if (auto_mode) {lcd.setCursor(LCD_COLS-1, row); lcd.write(byte(4));}
                    else lcd.print("    ");
                } break;
                case Hemisphere: { 
                    lcd.print(menu_str[Hemisphere]); lcd.print(": "); menu_op_value[Hemisphere]?lcd.print("N    "):lcd.print("S    ");
                } break;
                case tempo_tela: { 
                    lcd.print(menu_str[tempo_tela]); lcd.print("    "); lcd.print(menu_op_value[tempo_tela]); lcd.print("s     ");
                } break;
                default:    {
                    lcd.print(menu_str[current_text_row]); lcd.print(" : "); lcd.print(menu_op_value[current_text_row]); lcd.print("        "); //linha print
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

            on_menu       = !on_menu;
            value_preview = menu_op_value[current_selection];
            lock_value    = value_preview;
            wake          = true;
        }
    }

    else { //sub-menu de modificar valores
        lcd.setCursor(0, 0);
        lcd.print(" "); lcd.print(menu_str[current_selection]); lcd.print("           "); //titulo submenu

        //especific configs
        switch (current_selection) {
            case DEC_: { //declination
                if (auto_mode)  (value_preview==lock_value)?:value_preview=lock_value; //lock
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.write(byte(223)); lcd.print(value_preview%60); lcd.print("'    ");
                lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
                if (auto_mode) lcd.write(byte(4)); 
                else lcd.print("    ");
            } break;
            case RA: { //right ascendence
                if (auto_mode)  (value_preview==menu_op_value[1])?:value_preview=menu_op_value[1]; //lock on auto ra
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); lcd.print(value_preview/60); lcd.print("h"); lcd.print(value_preview%60); lcd.print("m    ");
                lcd.setCursor(LCD_COLS-1, LCD_ROWS-1);
                if (auto_mode) lcd.write(byte(4)); 
                else lcd.print("    ");
            } break;
            case Hemisphere: { //hemisphere
                (value_preview>0)?value_preview=1:value_preview; //boolify
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("     "); value_preview?lcd.print("Norte         "):lcd.print(" Sul        ");
            } break;
            case automatic_mode: { //auto-mode
                (value_preview>0)?value_preview=1:value_preview; //boolify
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); value_preview?lcd.print("ON        "):lcd.print("OFF       ");
            } break;
            case brilho_tela: { //brightness
                (value_preview>100)?value_preview=100:(value_preview<0)?value_preview=0:false;
                analogWrite(lcdBrightness_pin, ((value_preview*255)/100));
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("%        "); //linha 2 print
            } break;  
            case tempo_tela: { //brightness
                (value_preview>240)?value_preview=240:(value_preview<1)?value_preview=1:false;
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("s       "); //linha 2 print
            } break;   
            case (save_configs):     { //save (ultima op)
                lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(3)); lcd.print("  Salvando..."); lcd.print("         "); //linha 2 print
                uint8_t eeprom_brightness, eeprom_light_time, eeprom_Hemisphere;
                EEPROM.begin(menu_size);
                eeprom_brightness = EEPROM.read(0);
                eeprom_light_time = EEPROM.read(1);
                eeprom_Hemisphere = EEPROM.read(2);

                uint8_t brightness = menu_op_value[brilho_tela];
                uint8_t light_time = menu_op_value[tempo_tela];
                uint8_t hemisphere = menu_op_value[Hemisphere];
                //selective save
                if (eeprom_brightness != brightness)   {
                    EEPROM.write(0, brightness);
                    delay(800); //simular espera do save
                }
                if (eeprom_light_time != light_time) {
                    EEPROM.write(1, light_time);
                    delay(800);
                }
                if (eeprom_Hemisphere != hemisphere) {
                    EEPROM.write(2, hemisphere);
                    delay(800);
                }

                EEPROM.end();
                lcd.setCursor(0, 1); lcd.write(byte(2)); //ok
                delay(500);
                on_menu = !on_menu; //sai automaticamente da tela de salvamento
            } break;  
            default:    { //submenu generico
                for (uint8_t row = 1; row < LCD_ROWS; row++)  {
                    lcd.setCursor(0, row);
                    if (row == LCD_ROWS-1){ 
                        lcd.write(byte(1)); lcd.print("      "); lcd.print(value_preview); lcd.print("        ");
                    }
                    else lcd.print("                  ");
                }
            }
        }
        
        if (digitalRead(SELECT) && (current_selection!=save_configs))    { //press sem ser no save
            menu_op_value[current_selection] = value_preview; //salva o setting
            lcd.setCursor(0, LCD_ROWS-1); lcd.write(byte(2)); //da o simbolo de ok;
            while (digitalRead(SELECT)) delay(800);
            wake         = true;
            ra_last_time = millis();
            on_menu      = !on_menu; //sai do sub-menu
            if (menu_op_value[automatic_mode])   {auto_mode = true; ra_last_time = millis();} else auto_mode = false; //inicia o modo automatico
        }
            
    }

};

void shift_bits(byte* source, byte* dest, uint8_t shift, bool right)  {
    if (!right) {
        for (uint8_t i = 0; i < 8; i++)   {
           dest[i] = source[i] << shift; 
        }
    }
    else    {
        for (uint8_t i = 0; i < 8; i++)   {
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
            if (!digitalRead(ROT_B))    { //anti horario
                if (arrow_row) arrow_row--;
                else !menu_top_row?:menu_top_row--; //guard pra signed assignment e limite superior do menu
            }
            else    { //horario
                if (arrow_row<LCD_ROWS-1) arrow_row++;
                else (menu_top_row==(menu_size-LCD_ROWS))?:menu_top_row++;
            }
        }

        last_Interrupt = interrupt_now; //remember the change (if there was one)
    }

    else    {
        if ((interrupt_now - last_Interrupt) > DEBOUNCE_ROTARY_MS)    { //debounce sem delay()
            if (!digitalRead(ROT_B))   {
                (value_preview<1)?:value_preview--; //guard pra signed assignment
            }
            else {
                value_preview+=5;
            }
        }

        last_Interrupt = interrupt_now;
    }
    wake = true;
    while(digitalRead(ROT_A)); //espara cair
}

// union {
//     const char* menu[] = {
//         "DEC_",
//         "R.A",
//         "Hemisphere", 
//         "Modo automatico", 
//         "Brilho da tela", 
//         "Salvar configs"
//     };
//     struct {
//         const char* option_0;
//         const char* option_1;
//         const char* option_2;
//         const char* option_3;
//         const char* option_4;
//         const char* option_5;
//     };
// } menu_str;
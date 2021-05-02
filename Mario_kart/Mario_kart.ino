//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"

#include <SD.h>
File root;
File myFile;

#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  
//***************************************************************************************************************************************
// Variables
//**********************************************************************************************

unsigned int chc[] = {57,107,157,207};

int Menu = 0;
int Menu1 = 0;
int Menu2 = 0;
int DrawMenu = 0;
int DrawRace = 0;
int Play = 0;
int PL1 = 0;
int PL2 = 0;

int x=150;
int y=54;
int x2=160;
int y2=25;

int pos2 = 1;
int cont1R = 0;
int cont1L = 0;
int cont1S = 0;
int cont2R = 0;
int cont2L = 0;
int cont2S = 0;

int lap1 = 0;
int lap2 = 0;


//***************************************************************************************************************************************
// Def. Botones
//*******************************************************************************************
const int Player1L = 37;  //PC_4
const int Player1R = PC_5;  //PC_5
const int Start1 = 31;    //PD_7
const int Player1U= PD_6;    //PD_6


const int Player2L = 35;  //PC_6
const int Player2R = 34;  //PC_7
const int Start2 = 17;    //PD_6
const int Player2U= PD_7;    //PD_6


//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

//*********************************************
// Funcion para leer valores de la SD
//*********************************************
void SD_archivo(unsigned char Bitmap_SD[], unsigned long Size_bitmap, char* filename) {
  File myFile = SD.open(filename);          // Abre el archivo 
  unsigned long i = 0;              
  char Bitmap_SD_HEX[] = {0, 0};          //Arreglo de dos numeros para cada posicion
  int Pos_1, Pos_2;                     //Var posiciones
  if (myFile) {                 
    do {
      Bitmap_SD_HEX[0] = myFile.read();          
      Pos_1 = Trans_ACII_HEX(Bitmap_SD_HEX);       //Realiza la conversion de Ascii a Hex
      Bitmap_SD_HEX[0] = myFile.read();         
      Pos_2 = Trans_ACII_HEX(Bitmap_SD_HEX);       //Trasforma la segunda
      Bitmap_SD[i] = (Pos_1 << 4) | (Pos_2 & 0xF);  //Concatenamos
      i++;                        
    } while (i < (Size_bitmap + 1));
  }
  myFile.close();                       
}

//*********************************************
// Funcion que pasa valores de la SD a valores hexadecimales
//*********************************************
int Trans_ACII_HEX(char *puntero) {
  int i = 0;
  for (;;) {
    char num = *puntero;
    if (num >= '0' && num <= '9') {
      i *= 16;
      i += num - '0';
    }
    else if (num >= 'a' && num <= 'f') {
      i *= 16;
      i += (num - 'a') + 10;
    }
    else break;
    puntero++;
  }
  return i;
}

extern uint8_t fondo[];
extern uint8_t mario_s[];
extern uint8_t yoshi_m[];
//extern uint8_t DK_m[];
extern uint8_t toad_m[];
extern uint8_t circuit[];
extern uint8_t personajes_cart[];
extern uint8_t personajes_cart_up[];
extern uint8_t personajes_cart_down[];

unsigned char Dk_menu[4901]={0};
//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  LCD_Init();
  //LCD_Clear(0x00);
  //pinMode botones
  pinMode(Player1R, INPUT_PULLUP);
  pinMode(Player1L, INPUT_PULLUP);
  pinMode(Player1U, INPUT_PULLUP);
  pinMode(Start1, INPUT_PULLUP);  

  pinMode(Player2L, INPUT_PULLUP);
  pinMode(Player2R,   INPUT_PULLUP);  
  pinMode(Player2U, INPUT_PULLUP); 
  pinMode(Start2,   INPUT_PULLUP);
  //muestra fondo pantalla de inicio   
  FillRect(0,0,320,240,0xffff);
  
  for (int i = 0; i < 10; i++) {  
      LCD_Bitmap((32*i),(40),32,32,meta);
    
  }

  for (int i = 0; i < 10; i++) {  
      LCD_Bitmap((32*i),(165),32,32,meta);
    
  }
  //Texto
  LCD_Bitmap(0, 205, 320, 35, fondo); 
  LCD_Print("M. E. Kart", 80, 100, 2, 0x0000, 0xffff);
  LCD_Print("PRESS START", 118, 135, 1, 0x0000, 0xffff);
  
  
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  //Si se presiona Start vamos a la seleccion de personaje
  if (digitalRead(Start1) == LOW && Play == 0){
    Play = 1;
    DrawMenu = 1;
    Menu = 1;
  }
  if (digitalRead(Start2) == LOW && Play == 0){
    Play = 1;
    DrawMenu = 1;
    Menu = 1;
    }
//***************************************************************************************************************************************
// Menu J1
//***************************************************************************************************************************************
  if (Play == 1){
      //Menu de seleccion de carro
      if (DrawMenu == 1){     
        LCD_Clear(0xF9A0);//Fondo naranja en menu
        LCD_Print("M. E. Kart", 80, 40, 2, 0xffff, 0xF9A0);
        DrawMenu = 0;
        DrawRace = 1;
        Menu = 1;
        Menu1 = 0;
        Menu2 = 0;
        }
      if (Menu == 1){  
      
      LCD_Bitmap(60, 150, 35, 35, mario_s);                    //Estilo de carro 1
      LCD_Bitmap(110, 150, 35, 35, yoshi_m);                  //Estilo de carro 2
      SD_archivo(Dk_menu,4901, "menudk.txt");           //Llamando la funcion para estrar archivo de la SD
      LCD_Bitmap(160, 150, 35, 35, Dk_menu);            //Estilo de carro 3
      LCD_Bitmap(210, 150, 35, 35, toad_m);              //Estilo de carro 4

      //Cuadro selector de carro
      if (Menu1  == 0){
               LCD_Print("    Choose your driver", 50, 100, 1, 0xffff, 0xF9A0);      //Imprime indicaciones a jugador uno 
               LCD_Print("Player 1", 120, 80, 1, 0xffff, 0xF9A0);
               if (digitalRead(Player1R) == LOW){                  //Antirebote boton 1R  
                cont1R = 1;
               }
               if (digitalRead(Player1R) == HIGH && cont1R == 1){  // Al presionar 1R aumenta PL1 para cambiar de estilo de carro
                //void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
                Rect(chc[PL1],147,40,40,0xF9A0);                   //Borra ultima posicion de rectangulo 
                PL1 = PL1 + 1;
          
                
               if (PL1 == 4){                                     //Si contador pasa el limite, se reinicia contador
                  PL1 = 0;
                 
                }
                cont1R = 0;
              }
              
              if (digitalRead(Player1L) == LOW){                //Antirebote de boton
                cont1L = 1;
               }
               if (digitalRead(Player1L) == HIGH && cont1L == 1){    //Movimineto de cuadro a izquierda para escoger carro
                Rect(chc[PL1],147,40,40,0xF9A0);                     //Borra ultima posicion de rectangulo 
                PL1 = PL1 - 1;
                
                if (PL1 < 0){
                  PL1 = 3;                                              //Si contador pasa el limite, se reinicia contador
                 
                  
                }
                cont1L = 0;
              } 
              
              if (digitalRead(Start1) == LOW){                     //Antirebote de start
                cont1S = 1;
              }
              
              Rect(chc[PL1],147,40,40,0xffff);
              if (digitalRead(Start1) == HIGH && cont1S ==1){                                      //Jugador 1 confirmacion listo para jugar
                  Menu1 = 1;
                  cont1S = 0;
                  Rect(chc[PL1],147,40,40,0x001f);                                                 //Borra rectangulo de eleccion de carro de jugador 1
                   
                }
                
      }
//***************************************************************************************************************************************
// Menu J2
//***************************************************************************************************************************************
      if (Menu2 == 0 && Menu1 == 1){
                   //LCD_Clear(0x0320);//Fondo verde en menu
                   LCD_Print("M. E. Kart", 80, 40, 2, 0xffff, 0xF9A0);
                   LCD_Print("    Choose your driver", 50, 100, 1, 0xffff, 0xF9A0);            //Imprime instrucciones de jugador 2
                   LCD_Print("Player 2", 120, 80, 1, 0xffff, 0xF9A0);
                  if (digitalRead(Player2R) == LOW){                                           //Antirebote
                    cont2R = 1;
                  }
                  if (digitalRead(Player2R) == HIGH && cont2R == 1){                           //Jugador escogiendo carro derecha
                    Rect(chc[PL2],147,40,40,0xF9A0);
                    PL2 = PL2 + 1;
                    if (PL2 == 4){                                                                     //Si contador pasa el limite, se reinicia contador
                      PL2 = 0;
                    }
                    cont2R = 0;
                  }
                  if (digitalRead(Player2L) == LOW){                                                    //Antirebote
                    cont2L = 1;
                  }
                  if (digitalRead(Player2L) == HIGH && cont2L == 1){                              //Jugador escogiendo carro izquierda
                    Rect(chc[PL2],147,40,40,0xF9A0);
                    PL2 = PL2 - 1;
                    if (PL2 < 0){                                            //Si contador pasa el limite, se reinicia contador
                      PL2 = 3;
                    }
                    cont2L = 0;
                  }
                  if (digitalRead(Start2) == LOW){                                                  //Antirebote
                    cont2S = 1;
                  }
                  if (digitalRead(Start2) == HIGH && cont2S ==1){          //Jugador 2 confirmacion listo para jugar
                    Menu2 = 1;
                    cont2S = 0;
                    LCD_Print("Ready", 129, 200, 1, 0x0000, 0xF9A0);
                    }
                    Rect(chc[PL2],147,40,40,0xffff);
      } 
//***************************************************************************************************************************************
// Confirmacion continuacion
//***************************************************************************************************************************************
      if (Menu1 == Menu2 && Menu1 == 1){                                  //  Confirmar que estan listos presionando start confirmando menu 1 y 2 que estan listos
        Menu = 0; 
        FillRect(0,80,280,10,0xF9A0);                                   //Fondo naranja de ultimo menu 
        LCD_Print("Press Start to Continue", 60, 80, 1, 0xffff, 0xF9A0);
      }
    }
    if (Menu == 0 && Play == 1){
      if (digitalRead(Start1) == LOW){                     //Antirebote de start
        cont1S = 1;
      }
      if (digitalRead(Start1) == HIGH && cont1S == 1){     //Start jugador 1, dirijirse a estado de play2 que es generacion de carrera
          Play = 2;
          cont1S = 0;
        }
        if (digitalRead(Start2) == LOW){                     //Antirebote de start
          cont2S = 1;
        }
        if (digitalRead(Start2) == HIGH && cont2S == 1){    //Start jugador 2, dirijirse a estado de play2 que es generacion de carrera
          Play = 2;
          cont2S = 0;
        }
      }  
  }
//**************************************************
//Contador
//**************************************************
if ((x == 195) &&  (10<= y) &&(y<= 58)){
  lap1++;
  x=196;
  switch (lap1){
    case 1:
    
    LCD_Print(" J1 ", 95, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("1/3", 95, 120, 1, 0xffff, 0xF9A0);
    break;

    case 2:
    
    LCD_Print(" J1 ", 95, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("2/3", 95, 120, 1, 0xffff, 0xF9A0);
    break;

    case 3:
     
    LCD_Print(" J1 ", 95, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("3/3", 95, 120, 1, 0xffff, 0xF9A0);
    break;

    default:
    break;
  }
}

 else if ((x2 == 195) &&  (10<= y2)&&(y2<= 58)){
  lap2++;
  x2=196;
  switch (lap2){
    case 1:
     
    LCD_Print(" J2 ", 200, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("1/3", 200, 120, 1, 0xffff, 0xF9A0);
    break;

    case 2:
     
    LCD_Print(" J2 ", 200, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("2/3", 200, 120, 1, 0xffff, 0xF9A0);
    break;

    case 3:
     
    LCD_Print(" J2 ", 200, 105, 1, 0xffff, 0xF9A0);
    LCD_Print("3/3", 200, 120, 1, 0xffff, 0xF9A0);
    break;

    default:
    break;
 }
 }

 if ((x == 196) &&  (10<= y) &&(y<= 58)){
  x=198;
 }

 if ((x2 == 196) &&  (10<= y2) &&(y2<= 58)){
  x2=198;
 }
//***************************************************************************************************************************************
// Mapa
//***************************************************************************************************************************************
  if (Play == 2){
    if (DrawRace==1){
      //LCD_Clear(0x0320);
      DrawRace=0;
      //FillRect(0,0,320,240,0x0320);
      LCD_Bitmap(0, 0, 320, 240, circuit);
      //FillRect( 62,78, 195,90,   0x0000);
      LCD_Print(" J1 ", 95, 105, 1, 0xffff, 0xF9A0);
      LCD_Print("0/3", 95, 120, 1, 0xffff, 0xF9A0);
      LCD_Print(" J2 ", 200, 105, 1, 0xffff, 0xF9A0);
      LCD_Print("0/3", 200, 120, 1, 0xffff, 0xF9A0);
     
    }
//************************************************************************************************************************************
//Jugador 1
//**********************************************************************************************************************************
   
//Posicion inicial
    if (digitalRead(Start1)==HIGH){
      LCD_Sprite(x,y,20,20,personajes_cart,4,PL1,1,0);
      //FillRect(150,51,20,20,0x955F);
    }
//******************************
//derecha
    if (digitalRead(Player1R) == LOW){              
       x++;
       //Limites
       if ((y>=58)&&(y<=162)){
            if((x>43)&&(x<=255)){
              x=40; 
            }
            else if(x>290){
              x=290;
            }
          }
       else if (x>290){
            x=290;
          }
        //Imprecion y parte para que no deje rastro
        LCD_Sprite(x,y,20,20,personajes_cart,4,PL1,1,0);
        V_line( x-1, y, 20, 0x955F);
        delay(5);
      
    }
//**********************************
//abajo
     if (digitalRead(Start1) == LOW){      
          //int anim2 = (x/35)%2;        
          y++;
          //Si 0<x<60 y 50<y<100
          if ((x>=46)&&(x<=255)){
            if((y>53)&&(y<=160)){
              y=52; 
            }
            else if(y>200){
              y=200;
            }
          }
          else if(y>200){
              y=200;
            }
          LCD_Sprite(x,y,20,20,personajes_cart_down,4,PL1,0,0);         
          FillRect( x, y-1, 20,1, 0x955F);
          delay(5);
          //cont1R = 0;
      
        }

        
//**********************************
//Izquierda
     if (digitalRead(Player1L) == LOW){      
        //int anim2 = (x/35)%2;        
        x--;
        if ((y>=58)&&(y<=162)){
            if((x>43)&&(x<=258)){
              x=259; 
            }
            else if(x<6){
              x=6;
            }
          }
       else if (x<6){
            x=6;
          }
        LCD_Sprite(x,y,20,20,personajes_cart,4,PL1,0,0);
        FillRect( x +20, y, 1,20, 0x955F);
        delay(5);
        //cont1R = 0;
      
        }
  //LCD_Bitmap(0, 0, 320, 240, circuit);  
//**********************************
//Arriba
     if (digitalRead(Player1U) == LOW){      
        //int anim2 = (x/35)%2;        
        y--;
        if ((x>=44)&&(x<=255)){
            if((y>53)&&(y<=162)){
              y=163; 
            }
            else if(y<18){
              y=18;
            }
          }
          else if(y<18){
              y=18;
            }
        LCD_Sprite(x,y,20,20,personajes_cart_up,4,PL1,0,0);
        FillRect( x, y+20, 20,1, 0x955F);
        delay(5);
        //cont1R = 0;
      
        }
  
//************************************************************************************************************************************
//Jugador 2
//**********************************************************************************************************************************
   
//Posicion inicial
    if (digitalRead(Start2)==HIGH){
      LCD_Sprite(x2,y2,20,20,personajes_cart,4,PL2,1,0);
      //FillRect(150,51,20,20,0x955F);
    }
//******************************
//derecha
    if (digitalRead(Player2R) == LOW){              
       x2++;
       if ((y2>=58)&&(y2<=162)){
            if((x2>43)&&(x2<=255)){
              x2=42; 
            }
            else if(x2>290){
              x2=290;
            }
          }
       else if (x2>290){
            x2=290;
          }
        LCD_Sprite(x2,y2,20,20,personajes_cart,4,PL2,1,0);
        V_line( x2-1, y2, 20, 0x955F);
        delay(5);
        //cont1R = 0;
        
      
    }
//**********************************
//abajo
     if (digitalRead(Start2) == LOW){      
          //int anim2 = (x2/35)%2;        
          y2++;
          //Si 0<x2<60 y2 50<y2<100
         if ((x2>=46)&&(x2<=255)){
            if((y2>53)&&(y2<=160)){
              y2=52; 
            }
            else if(y2>200){
              y2=200;
            }
          }
          else if(y2>200){
              y2=200;
            }
          
          LCD_Sprite(x2,y2,20,20,personajes_cart_down,4,PL2,0,0);         
          FillRect( x2, y2-1, 20,1, 0x955F);
          delay(5);
          //cont1R = 0;
      
        }

        
//**********************************
//Izquierda
     if (digitalRead(Player2L) == LOW){      
        //int anim2 = (x2/35)%2;        
        x2--;
        if ((y2>=58)&&(y2<=162)){
            if((x2>43)&&(x2<=258)){
              x2=259; 
            }
            else if(x2<6){
              x2=6;
            }
          }
       else if (x2<6){
            x2=6;
          }
        LCD_Sprite(x2,y2,20,20,personajes_cart,4,PL2,0,0);
        FillRect( x2 +20, y2, 1,20, 0x955F);
        delay(5);
        //cont1R = 0;
        
        }
  //LCD_Bitmap(0, 0, 320, 240, circuit);  
//**********************************
//Arriba
     if (digitalRead(Player2U) == LOW){      
        //int anim2 = (x2/35)%2;        
        y2--;
        if ((x2>=44)&&(x2<=255)){
            if((y2>53)&&(y2<=162)){
              y2=163; 
            }
            else if(y2<18){
              y2=18;
            }
          }
          else if(y2<18){
              y2=18;
            }
        LCD_Sprite(x2,y2,20,20,personajes_cart_up,4,PL2,0,0);
        FillRect( x2, y2+20, 20,1, 0x955F);
        delay(5);
        //cont1R = 0;
      
        }
//*************************************************************************************************************************************
//Ganador
//*************************************************************************************************************************************
  if (lap1 == 4){
    lap1 = 3;
    FillRect(0,100,320,35,  0xEDA0);
    LCD_Print("P1 WINS", 100 , 110 , 2 , 0x0000, 0xEDA0);
    
    delay(50000);
  }
  else if(lap2 == 4){
    lap2 = 3;
    FillRect(0,100,320,35,  0xEDA0);
    LCD_Print("P2 WINS", 100 , 110 , 2 , 0x0000, 0xEDA0);
    
    delay(500000);
  }
  
}

  
}//Loop

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
*/

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}

// =============================================================================================================
// --- Bibliotecas Auxiliares ---

#include <TimeLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <DS3231.h>



//=============================================================================================================
// --- Mapeamento de Hardware --- 

#define  led          2   // pino do Led
#define  buzzer       3   // pino do Buzzer
#define  e1           6   // pino da entrada 1 do motor de passo
#define  e2           7   // pino da entrada 2 do motor de passo
#define  e3           8   // pino da entrada 3 do motor de passo
#define  e4           9   // pino da entrada 4 do motor de passo
#define  bt_direito   14  // botão que avança para a direita
#define  bt_esquerdo  15  // botão que avança para a esquerda
#define  bt_enter     16  // botão utilizado para entrar
#define  bt_voltar    17  // botão utilizado para voltar



// =============================================================================================================
// --- Constantes e Objetos --- 

#define   menu_max   5          // número máximo de menus existentes
#define DELAY_BOT_SLOW    300   // Delay após pressionar botões
#define DELAY_BOT_FAST    100   // Delay após pressionar botões
#define endereco 0x27           // Endereço do display I2C
#define colunas  16             // Define a quantidade de colunas do display I2C
#define linhas   2              // Define a quantidade de linhas do display I2C

LiquidCrystal_I2C lcd(endereco, colunas, linhas); 

DS3231 rtc(SDA, SCL);

Time t;

const int passosPorVolta = 2048; // Total de passos por volta

Stepper mp(passosPorVolta, e1, e3, e2, e4);


// =============================================================================================================
// --- Protótipo das Funções ---
void keyboard();
void menu0();
void menu1();
void menu2();
void menu3();
void menu4();
void menu5();
void verificar_alarme(void);
void tocar(char* mus[], int tempo[]);
String get_hour(void);
String get_date(void) ;



// =============================================================================================================
// --- Variáveis Globais --- 
int menu_num = 0, sub_menu = 1;


byte SIMBOLO_SETA[8] = // Simbolo da seta na configuraçao
  {
  B00100,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte SIMBOLO_RELOGIO[8] = // Simbolo do relogio
  {
    B00000,
    B10001,
    B01110,
    B10101,
    B10111,
    B10001,
    B01110,
    B00000,
};

// Musica do despertador
const char* MUSICA[] = {"Re", "Mi", "Fa", "Sol", "Do", "La", "Fa", "Do", "Fim"};
const int DURACAO[] = {500, 500, 500, 500, 500, 500, 500, 500};

// Variaveis auxiliares dos menus
bool unico = false;
uint8_t d1, d2, pos = 0;
uint16_t d3; // É de 16 bits, pois pode armazenar o ano da data

// Variaveis para controle dos alarmes
bool parou_alarme = false, alarme_ativado[3] = {false}; 

// Horas e minutos do alarme
unsigned char alarmes[3][2] = {{12, 1}, {12,2}, {12,3}};



// =============================================================================================================
// --- Configurações Iniciais ---
void setup() 
{
  pinMode(led, OUTPUT);               // Define o pino Led (2) como entrada
  pinMode(buzzer, OUTPUT);            // Define o pino buzzer (3) como saída
  pinMode(bt_direito,  INPUT_PULLUP); // Define bt_direito (A0) como entrada com resitor interno
  pinMode(bt_esquerdo, INPUT_PULLUP); // Define bt_esquerdo(A1) como entrada com resitor interno
  pinMode(bt_enter,    INPUT_PULLUP); // Define bt_enter   (A2) como entrada com resitor interno
  pinMode(bt_voltar,   INPUT_PULLUP); // Define bt_voltar  (A3) como entrada com resitor interno

  lcd.init();                          // Inicia a comunicação com o display
  lcd.backlight();                     // Liga a iluminação do display
  lcd.clear();                         // Limpa todos os caracteres do lcd
  lcd.createChar(1, SIMBOLO_SETA);     // Cria o simbolo de seta
  lcd.createChar(2, SIMBOLO_RELOGIO);  // Cria o simbolo de alarme ativado
  lcd.setCursor(0,0);                  // Posiciona o cursor do display
  lcd.print(F(" Ola, eu sou o  "));    // Imprime o texto entre aspas
  lcd.setCursor(0,1);                  // Posiciona o cursor do display
  delay(1000);                         // Da um delay até a próxima instrução
  lcd.print(F("    \"DiCo\"      "));    // Imprime o texto entre aspas
  delay(3000);                         // Da um delay até a próxima instrução
  lcd.clear();
  lcd.setCursor(0,0);                  // Posiciona o cursor do display
  lcd.print(F(" seu dispenser  "));    // Imprime o texto entre aspas
  lcd.setCursor(0,1);                  // Posiciona o cursor do display
  lcd.print(F(" de comprimidos!"));    // Imprime o texto entre aspas
  delay(3000);                         // Da um delay até a próxima instrução
  lcd.clear();

  rtc.begin();
  // rtc.setTime(20,00, 0);     // Set the time to 12:00:00 (24hr format)
  
  setTime(12, 0, 0, 1, 1, 2001);     // Define hora/min/segundo - dia/mes/ano

  Serial.begin(9600);

  mp.setSpeed(15); // Definido rotações por minuto
  
 
} //end setup


// =============================================================================================================
// --- Configurações Iniciais ---
void loop() 
{

  t = rtc.getTime();
  
  keyboard();
  
  switch(menu_num)
  {
    case 0: menu0(); break;
    case 1: menu1(); break;
    case 2: menu2(); break;
    case 3: menu3(); break;
    case 4: menu4(); break;
    case 5: menu5(); break;
    
  } //end switch

} //end loop


// =============================================================================================================
// --- Desenvolvimento das Funções ---
void keyboard()
{
   if(!digitalRead(bt_direito) && sub_menu == 1)
   {
      delay(150);
      if(menu_num <= menu_max) menu_num += 1;
    
   } //end bt_direito

   if(!digitalRead(bt_esquerdo) && sub_menu == 1)
   {
      delay(150);
      if(menu_num > 0)
      {
        menu_num -= 1;
        lcd.clear();
      }
    
   } //end bt_esquerdo

   if(!digitalRead(bt_enter))
   {
      delay(150);
      if(sub_menu <= 2) sub_menu += 1;
    
   } //end bt_enter

   if(!digitalRead(bt_voltar))
   {
      delay(150);
      if(sub_menu > 0) sub_menu -= 1;
    
   } //end bt_voltar
   
  
} //end keyboard

// --- MENU INICIAL (TELA DE REPOUSO)---
void menu0()                      
{
  lcd.setCursor(0,0);
  //lcd.print(get_hour());
  lcd.print(rtc.getTimeStr());
  lcd.setCursor(0,1);
  //lcd.print(get_date());
  lcd.print(rtc.getDateStr());
  lcd.setCursor(14,1);
  lcd.print(">>");
  verificar_alarme();         // Verifica se esta na hora de despertar

// Simbolo do relogio caso o despertador 1 esteja ativo
  if (!parou_alarme && alarme_ativado[0])
  {
    lcd.setCursor(13,0);
    lcd.write(2);
  }

  // Simbolo do relogio caso o despertador 2 esteja ativo
  if (!parou_alarme && alarme_ativado[1])
  {
    lcd.setCursor(14,0);
    lcd.write(2);
  }

  // Simbolo do relogio caso o despertador 3 esteja ativo
  if (!parou_alarme && alarme_ativado[2])
  {
    lcd.setCursor(15,0);
    lcd.write(2);
  }
 
} //end menu0

// =============================================================================================================
// --- MENU 1 - MENU CONFIGURAÇÂO DE HORA ---
void menu1()
{
    switch(sub_menu)
    {
       case 1:
         lcd.setCursor(0,0);
         lcd.print(F("===== MENU ====="));
         lcd.setCursor(0,1);
         lcd.print(F(" Ajustar horas >"));
         unico = false;
         break;
  
       case 2:     
        if (!unico)
        {
          lcd.clear();
          pos = 0;
          d1 = t.hour;
          d2 = t.min;
          unico = true;
        }
        
        lcd.setCursor(5, 0);
        if (d1 < 10)
        {
          lcd.print("0");
        }    
        lcd.print(d1);
        lcd.print(":");
       
        if (d2 < 10)
        {
          lcd.print("0");
        }
        lcd.print(d2);
  
        // Desenha a seta
        lcd.setCursor(6 + pos*2, 1);  
        lcd.write(1);
     
// ----- Testa os botoes -----
        if(!digitalRead(bt_direito))
        {
        
          if (pos == 0 && d1 < 23)
          {
            
            d1 ++;
            
          }
          else if (pos == 1 && d2 < 59)
          {
            d2 ++;
          }
          delay(DELAY_BOT_SLOW);
        }
       else if(!digitalRead(bt_esquerdo))
        {
        
          if (pos == 0 && d1 > 0)
          {
            d1 --;
          }
          else if (pos == 1 && d2 > 0)
          {
            d2 --;
          }
          delay(DELAY_BOT_SLOW);
        }
        else if (!digitalRead(bt_enter) && pos < 1 )
        {
            pos++;  
            lcd.setCursor(6, 1);
            lcd.print(" ");
            delay(DELAY_BOT_SLOW);
        }
        else if (!digitalRead(bt_voltar) && pos > 0)
        {
            pos--;  
            lcd.setCursor(8, 1);
            lcd.print(" ");
            delay(DELAY_BOT_SLOW);
        }
        else if (!digitalRead(bt_enter) && pos == 1)
        {      
         rtc.setTime(d1, d2, 0);
          
   
        menu_num = 2;
        sub_menu = 1;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);
       }
    
        break;

   }
   
} //end menu1

// =============================================================================================================
// --- MENU 2 - MENU CONFIGURAÇÂO DE DATA ---

void menu2()
{
  switch(sub_menu)
  {
     case 1:
       lcd.setCursor(0,0);
       lcd.print(F("===== MENU ====="));
       lcd.setCursor(0,1);
       lcd.print(F("< Ajustar data >"));
       unico = false;
       break;
     case 2:
       if (!unico)
          {
            lcd.clear();
            pos = 0;
            d1 = t.date;
            d2 = t.mon;
            d3 = t.year;           
            unico = true;
          }      
       lcd.setCursor(3,0);
       if (d1 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d1);
       lcd.print("/");

       if (d2 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d2);
       
       lcd.print("/");
       lcd.print(d3);

       lcd.setCursor(4 + pos*3, 1);
       lcd.write(1);

// ----- Testa os botoes -----
      if (!digitalRead(bt_direito))
      {
        uint8_t aux;
       
        // Restringe o valor do dia de acordo com o mês e ano (se for bissexto)
        if(d2 == 1 || d2 == 3 || d2 == 5 || d2 == 7 || d2 == 8 || d2 == 10 || d2 == 12)
        {
          aux = 31;
        }
        else if(d2 == 2)
        {
          if(d3%4 == 0)
          {
            aux = 29;
          }
          else
          {
            aux = 28;
          }
        }
        else // Condições faltantes: mes 4, 6, 9 ou 11
        {
          aux = 30;
        }
       
        if (pos == 0 && d1 < aux)
        {
          d1++;
        }
        else if (pos == 1 && d2 < 12)
        {
          d2++;
        }
        else if(pos == 2 && d3 < 9999)
        {
          d3++;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (!digitalRead(bt_esquerdo))
      {
        if (pos == 0 && d1 > 1)
        {
          d1--;
        }
        else if (pos == 1 && d2 > 1)
        {
          d2--;
        }
        else if(pos == 2 && d3 > 0)
        {
          d3--;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (!digitalRead(bt_enter) && pos < 2)
      {
          pos++;  
          lcd.setCursor(4, 1);
          lcd.print("       ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_voltar) && pos > 0)
      {
          pos--;  
          lcd.setCursor(4, 1);
          lcd.print("       ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_enter) && pos == 2)
      {
        rtc.setDate(d1, d2, d3);

        menu_num = 0;
        sub_menu = 1;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);
      }

      break;
       
  }
   
} //end menu2

// =============================================================================================================
// --- MENU 3 - MENU CONFIGURAÇÂO DE ALARME 1 ---

void menu3()
{
  switch(sub_menu)
  {
     case 1:
       lcd.setCursor(0,0);
       lcd.print(F("===== MENU ====="));
       lcd.setCursor(0,1);
       lcd.print(F("<Ajusta alarme1>"));
       unico = false;
       break;
     case 2:
       if (!unico)
          {
            lcd.clear();
            pos = 0;
            d1 = alarmes[0][0];
            d2 = alarmes[0][1];
            d3 = alarme_ativado[0];           
            unico = true;
          }       
     
       lcd.setCursor(0,0);
       if (d1 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d1);
       lcd.print(":");

       if (d2 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d2);

       lcd.setCursor(6, 0);
       if (!d3)
       {
        lcd.print("Desativado");
       }
       else
       {
        lcd.setCursor(7, 0);
        lcd.print("Ativado");

       }

// Desenha a seta
      switch (pos)
      {
        case 0:
          lcd.setCursor(1, 1);
          break;
        case 1:
          lcd.setCursor(3, 1);
          break;
        case 2:
          lcd.setCursor(10, 1);
          break;
      }
      lcd.write(1);

// ----- Testa os botoes -----
      if (!digitalRead(bt_direito))
      {
        if (pos == 0 && d1 < 23)
        {
          d1 ++;
        }
        else if (pos == 1 && d2 < 59)
        {
          d2 ++;
        }
        else if (pos == 2 && d3 < 1)
        {
          d3 ++;
          lcd.setCursor(6, 0);
          lcd.print(" ");
          lcd.setCursor(14, 0);
          lcd.print("  ");
        }
        delay(DELAY_BOT_FAST);
      }

      else if (!digitalRead(bt_esquerdo))
      {
        if (pos == 0 && d1 > 0)
        {
          d1 --;
        }
        else if (pos == 1 && d2 > 0)
        {
          d2 --;
        }
        else if (pos == 2 && d3 > 0)
        {
          d3 --;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (!digitalRead(bt_enter) && pos < 2)
      {
          pos++;
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_voltar) && pos > 0)
      {
          pos--;  
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_enter) && pos == 2)
      {
        
        alarmes[0][0] = d1;
        alarmes[0][1] = d2;
        alarme_ativado[0] = d3;


        menu_num = 0;
        sub_menu = 1;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);

      }
       break;
  }
 
} //end menu3

// =============================================================================================================
// --- MENU 4 - MENU CONFIGURAÇÂO DE ALARME 2 ---

void menu4()
{
  switch(sub_menu)
  {
     case 1:
       lcd.setCursor(0,0);
       lcd.print(F("===== MENU ====="));
       lcd.setCursor(0,1);
       lcd.print(F("<Ajusta alarme2>"));
       unico = false;
       break;
     case 2:
       if (!unico)
          {
            lcd.clear();
            pos = 0;
            d1 = alarmes[1][0];
            d2 = alarmes[1][1];
            d3 = alarme_ativado[1];           
            unico = true;
          }       
     
       lcd.setCursor(0,0);
       if (d1 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d1);
       lcd.print(":");

       if (d2 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d2);

       lcd.setCursor(6, 0);
       if (!d3)
       {
        lcd.print("Desativado");
       }
       else
       {
        lcd.setCursor(7, 0);
        lcd.print("Ativado");

       }

// Desenha a seta
      switch (pos)
      {
        case 0:
          lcd.setCursor(1, 1);
          break;
        case 1:
          lcd.setCursor(3, 1);
          break;
        case 2:
          lcd.setCursor(10, 1);
          break;
      }
      lcd.write(1);

// ----- Testa os botoes -----
      if (!digitalRead(bt_direito))
      {
        if (pos == 0 && d1 < 23)
        {
          d1 ++;
        }
        else if (pos == 1 && d2 < 59)
        {
          d2 ++;
        }
        else if (pos == 2 && d3 < 1)
        {
          d3 ++;
          lcd.setCursor(6, 0);
          lcd.print(" ");
          lcd.setCursor(14, 0);
          lcd.print("  ");
        }
        delay(DELAY_BOT_FAST);
      }

      else if (!digitalRead(bt_esquerdo))
      {
        if (pos == 0 && d1 > 0)
        {
          d1 --;
        }
        else if (pos == 1 && d2 > 0)
        {
          d2 --;
        }
        else if (pos == 2 && d3 > 0)
        {
          d3 --;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (!digitalRead(bt_enter) && pos < 2)
      {
          pos++;
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_voltar) && pos > 0)
      {
          pos--;  
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_enter) && pos == 2)
      {
        
        alarmes[1][0] = d1;
        alarmes[1][1] = d2;
        alarme_ativado[1] = d3;


        menu_num = 0;
        sub_menu = 1;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);

      }
       break;
  }
 
} //end menu4

// =============================================================================================================
// --- MENU 5 - MENU CONFIGURAÇÂO DE ALARME 3 ---

void menu5()
{
  switch(sub_menu)
  {
     case 1:
       lcd.setCursor(0,0);
       lcd.print(F("===== MENU ====="));
       lcd.setCursor(0,1);
       lcd.print(F("<Ajusta alarme3 "));
       unico = false;
       break;
     case 2:
       if (!unico)
          {
            lcd.clear();
            pos = 0;
            d1 = alarmes[2][0];
            d2 = alarmes[2][1];
            d3 = alarme_ativado[2];           
            unico = true;
          }       
     
       lcd.setCursor(0,0);
       if (d1 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d1);
       lcd.print(":");

       if (d2 < 10)
       {
        lcd.print("0");
       }
       lcd.print(d2);

       lcd.setCursor(6, 0);
       if (!d3)
       {
        lcd.print("Desativado");
       }
       else
       {
        lcd.setCursor(7, 0);
        lcd.print("Ativado");

       }

// Desenha a seta
      switch (pos)
      {
        case 0:
          lcd.setCursor(1, 1);
          break;
        case 1:
          lcd.setCursor(3, 1);
          break;
        case 2:
          lcd.setCursor(10, 1);
          break;
      }
      lcd.write(1);

// ----- Testa os botoes -----
      if (!digitalRead(bt_direito))
      {
        if (pos == 0 && d1 < 23)
        {
          d1 ++;
        }
        else if (pos == 1 && d2 < 59)
        {
          d2 ++;
        }
        else if (pos == 2 && d3 < 1)
        {
          d3 ++;
          lcd.setCursor(6, 0);
          lcd.print(" ");
          lcd.setCursor(14, 0);
          lcd.print("  ");
        }
        delay(DELAY_BOT_FAST);
      }

      else if (!digitalRead(bt_esquerdo))
      {
        if (pos == 0 && d1 > 0)
        {
          d1 --;
        }
        else if (pos == 1 && d2 > 0)
        {
          d2 --;
        }
        else if (pos == 2 && d3 > 0)
        {
          d3 --;
        }
        delay(DELAY_BOT_FAST);
      }
      else if (!digitalRead(bt_enter) && pos < 2)
      {
          pos++;
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_voltar) && pos > 0)
      {
          pos--;  
          lcd.setCursor(1, 1);
          lcd.print(" ");
          lcd.setCursor(3, 1);
          lcd.print(" ");
          lcd.setCursor(10, 1);
          lcd.print(" ");
          delay(DELAY_BOT_SLOW);
      }
      else if (!digitalRead(bt_enter) && pos == 2)
      {
        
        alarmes[2][0] = d1;
        alarmes[2][1] = d2;
        alarme_ativado[2] = d3;


        menu_num = 0;
        sub_menu = 1;
        pos = 0;
       
        lcd.clear();
        delay(DELAY_BOT_SLOW);

      }
       break;
  }
 
} //end menu5


// =============================================================================================================
// Verifica se está na hora de algum alarme tocar


void verificar_alarme(void)
{

   unsigned char i = 0;
   for(i = 0; i < 4; i++)
   {
     if (t.hour == alarmes[i][0] && t.min == alarmes[i][1] && !parou_alarme && alarme_ativado[i])
    {
      if(alarme_ativado[0] == true && alarme_ativado[1] == true && alarme_ativado[2] == true)
      {
        Serial.println("Usuário utilizando sistema com 3 alarmes");
        mp.step(-passosPorVolta/22);
      }

      else if(alarme_ativado[0] == true && alarme_ativado[1] == true && alarme_ativado[2] == false)
      {
        
        if(i == 0)
          {
           Serial.print("O valor da variável i é: ");Serial.print(i);Serial.println(" - Primeiro alarme do dia");
            mp.step(-passosPorVolta/22);
          }
         else
          {
            Serial.print("O valor da variável i é: ");Serial.print(i);Serial.println(" - Segundo alarme do dia");
            mp.step(-passosPorVolta/22);
            Serial.print("Aguardar 2 segundos");
            delay(2000); 
            mp.step(-passosPorVolta/22);           
          }
      }

      else
       {      
          Serial.print("Usuário utilizando sistema com alarme unico");Serial.println(" - Dispensar 1 comprimido");
          mp.step(-passosPorVolta/15);
          Serial.print("Aguardar 2 segundos");
          delay(2000);
          mp.step(-passosPorVolta/15);
          delay(2000);
          mp.step(-passosPorVolta/15);
        }

      digitalWrite(led, HIGH);
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(F("Segure Enter"));
      lcd.setCursor(3, 1);
      lcd.print(F("para parar"));
      
     
      while (!parou_alarme)
      {
        tocar(MUSICA, DURACAO);
        if (!digitalRead(bt_enter))
        {
          parou_alarme = true;
          digitalWrite (led, LOW);
        }
        delay(200);
      }
  
      lcd.clear();
     
   } //end if
   
} //end for

  // So reinicia a variável quando os segundos forem menor que 2 (próximo minuto)
  if(parou_alarme && t.sec <= 2)
  {
    parou_alarme = false;
    menu_num = 0;
    sub_menu = 1;
    pos = 0;
       
    lcd.clear();
    delay(DELAY_BOT_SLOW);
  }
 
} //end void verificar_alarme

//Toca uma musica de acordo com o parâmetro

  void tocar(char* mus[], int tempo[])
  {
    int tom = 0;
    for (int i = 0; mus[i] != "Fim"; i++)
    {
      if (mus[i] == "Do") tom = 262;
      if (mus[i] == "Re") tom = 294;
      if (mus[i] == "Mi") tom = 330;
      if (mus[i] == "Fa") tom = 349;
      if (mus[i] == "Sol") tom = 392;
      if (mus[i] == "La") tom = 440;
      if (mus[i] == "Si") tom = 494;
      if (mus[i] == "Do#") tom = 528;
      if (mus[i] == "Re#") tom = 622;
      if (mus[i] == "Fa#") tom = 370;
      if (mus[i] == "Sol#") tom = 415;
      if (mus[i] == "La#") tom = 466;
      if (mus[i] == "Pausa") tom = 0;
     
      tone(buzzer, tom, tempo[i]);
      delay(tempo[i]);
    
  // CONDIÇAO DO DESPERTADOR - Não tem relação com essa função
      if (!digitalRead(bt_enter))
      {
        parou_alarme = true;  
        break;
      }
    }
  }


//===========================================================================================
/*
 * Retorna uma string com o horário atual no formato:
 * HH:MM:SS
 */

String get_hour(void)
{
  String horario = "";
  unsigned char i, aux;
 
  for (i=0; i < 3; i++)
  {
    if(i == 0)
    {
      aux = hour();
    }
    else if(i == 1)
    {
      aux = minute();
    }
    else if(i == 2)
    {
      aux = second();
    }

    if(aux < 10)
    {
      horario += "0";
    }
    horario += aux;

    if(i != 2)
    {
      horario += ":";
    }
  }

  return horario;
}

/*
 * Retorna uma string com a data no formato:
 * DD/MM/AAAA
 */
String get_date(void)
{
  String data = "";
  unsigned int i, aux;
 
  for (i=0; i < 3; i++)
  {
    if(i == 0)
    {
      aux = day();
    }
    else if(i == 1)
    {
      aux = month();
    }
    else if(i == 2)
    {
      aux = year();
    }

    if(aux < 10)
    {
      data += "0";
    }
    data += aux;

    if(i != 2)
    {
      data += "/";
    }
  }

  return data;
}










 

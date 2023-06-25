#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

#define Buton_B3          13      //buton 3
#define Buton_B2          12      //buton 2
#define Buton_B1          11      //buton 1
#define S_Noapte          10      //switch far-ochi de pisica
#define S_Stanga          9       //buton semnalizare stanga
#define S_Frana           8       //buton frana
#define S_Dreapta         7       //buton semnalizare dreapta
#define L_Noapte          6       //LED semnalizare noapte activat
#define L_Frana           5       //LED semnalizare frana
#define L_Ochi_de_Pisica  4       //LED Ochi de Pisica
#define L_Distanta        3       //LED semnalizare dreapta
#define L_Dreapta         1       //LED semnalizare frana
#define L_Stanga          0       //LED semnalizare stanga
//#define S_Viteza          0       //senzorul de masurare a vitezei

#define  Unghi_Ghidon     A0      //unghi ghidon - analogic
#define  Lumina           A1      //intensitate luminoasa - analogic
#define echoPin           A2      //echo - de la sonar
#define trigPin           A3      //trigger - pentru sonar


#define ct_Timer1   65550         //100 msec

#define dist_100cm   100
#define dist_80cm     80
#define dist_70cm     70
#define dist_60cm     60
#define dist_m50cm    50

//senzor ultrasonic
float perioada;
//int mas_distanta_spate;
int mas_distanta_spate;

//unghi ghidon
volatile int mas_unghi_ghidon=512;

//bare LED-uri
CRGB LED_Stanga[16];
CRGB LED_Dreapta[16];
CRGB LED_Distanta[8];
CRGB LED_Ochi_Pisica[8];
CRGB LED_Frana[8];

// Afisajul LCD la adresa 0x27 pentru 20 caractere si 4 linii
LiquidCrystal_I2C lcd(0x20, 20, 4);
void  afis_ecran(char sir_afis[80]);

char ecran_lucru[80];

//contoare timer
volatile int cont_500msec = 0;
volatile int cont_1sec = 0;
volatile bool pulse_500msec = false;
volatile bool pulse_1sec = false; 

//viteza, distanta
float viteza = 0.0;               //viteza in km/h
float dist_viteza = 0.0;
float distanta = 0;             //distanta in km
float old_distanta = 0.0;
float r_roata = 0.305;          //24 inch -> raza de 30.48 cm  20 = 25.4, 24 =30.48 ; 26=33.02, 29=36.83
float d_roata = 24.0;
float nr_imp = 24.0;            //24 imp/rotatie
float L_roata;                  //circumferinta roata
float L_imp;                    //distanta pentru un impuls;
float contor_distanta = 0.0;
float old_contor_distanta = 0.0;
volatile int  old_impuls_dist = LOW;
int sec = 0;
int min = 0;
int ora = 0;
bool start_stop = false;

/******************************************************************
 Definitii functii semnalizare directie, frana si noapte
*******************************************************************/
void semnal_stanga(void);
void semnal_dreapta(void);
void semnal_frana(void);
void semnal_noapte(void);
int  mas_unghi(void);
void setari(void);

/******************************************************************
 Definitii functie calcul viteza si distanta
*******************************************************************/
//void calcul_viteza_distanta(void);

/******************************************************************
 Definitii functii semnalizare distanta
*******************************************************************/
void afis_distanta(void);

//intrerupere timer 1
ISR (TIMER1_OVF_vect){
//reincarcare constanta de timp
  TCNT1 = ct_Timer1;
 
//masurare 500msec
  cont_500msec++;
  if(cont_500msec == 2){
    cont_500msec = 0;
    pulse_500msec=!pulse_500msec;
    if(start_stop == false){
        return;
    }
//afisare viteza  
     dist_viteza =(contor_distanta - old_distanta);
     old_distanta = contor_distanta; 
    if(dist_viteza != 0.0){  
     viteza = dist_viteza * 12.0;
    } 
    else
      viteza = 0.0;
    sec++;
    if(sec == 60){
       sec = 0;
       min++;
    }   
  }  

//masurare 1sec
  cont_1sec++;
  if(cont_1sec == 10){
    cont_1sec = 0;
    pulse_1sec = !pulse_1sec;
  }  
 
}

void setup() {
int tmp_i;
// Butoane 
  pinMode(S_Stanga,INPUT_PULLUP);       //buton semnalizare stanga
  pinMode(S_Frana,INPUT_PULLUP);        //buton frana
  pinMode(S_Dreapta,INPUT_PULLUP);      //buton semnalizare dreapta
  pinMode(S_Noapte,INPUT_PULLUP);       //switch semnalizare noapte
  pinMode(Buton_B1,INPUT_PULLUP); 
  pinMode(Buton_B2,INPUT_PULLUP); 
  pinMode(Buton_B3,INPUT_PULLUP); 

//LED-uri semnalizare
  FastLED.addLeds<WS2812, L_Stanga, GRB>(LED_Stanga, 16);
  FastLED.addLeds<WS2812, L_Dreapta, GRB>(LED_Dreapta, 16);
  FastLED.addLeds<WS2812, L_Distanta, GRB>(LED_Distanta, 8);
  FastLED.addLeds<WS2812, L_Ochi_de_Pisica, GRB>(LED_Ochi_Pisica, 8);
  FastLED.addLeds<WS2812, L_Frana, GRB>(LED_Frana, 8);

  for(tmp_i=0;tmp_i<16;tmp_i++){  
      LED_Stanga[tmp_i] = CRGB ( 0, 0, 0);
      FastLED.show();
  }  

//Releu noapte
  pinMode(L_Noapte,OUTPUT);
  digitalWrite(L_Noapte,LOW);

// initialize the LCD
	lcd.begin();
//            *       Linia 0    **     Linia 1      **   Linia 2        **       Linia 3    *  
//            01234567890123456789012345678901234567890123456789012345678901234567890123456789
  afis_ecran(" PROIECT DE DIPLOMA     HODUT RALUCA     CALCULATOR DE BORD   PENTRU BICICLETA  ");
//  while(digitalRead(Buton_B3)==HIGH);
//            *       Linia 0    **     Linia 1      **   Linia 2        **       Linia 3    *  
//            01234567890123456789012345678901234567890123456789012345678901234567890123456789
//  afis_ecran("Buton B1: Setup     Buton B2: Start                                             ");
  while(1){
    if(digitalRead(Buton_B1)==LOW){
      start_stop = false;
      setari();   
    }
    if(digitalRead(Buton_B2)==LOW){
      start_stop = true;
      break;    
    }  
  }

//            *       Linia 0    **     Linia 1      **   Linia 2        **       Linia 3    *  
//            01234567890123456789012345678901234567890123456789012345678901234567890123456789
  afis_ecran("Viteza:      Km/h   Distanta:       Km  Dist. spate:      m Timp:     min    sec");

//initializare timer 1
  cli();
  TCCR1A = 0x00;
  TCCR1B = 0x03;        //fact presc 256
  TCNT1 = ct_Timer1;    //100 msec
  TIMSK1 |=(1<<TOIE1);
  sei();
//senzor ultrasonic
  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  

//calcule miscare
 L_roata = 2*3.14*r_roata;                  //circumferinta roata
 L_imp = L_roata/nr_imp;                    //distanta pentru un impuls;


 attachInterrupt(digitalPinToInterrupt(2),calcul_viteza_distanta,RISING);
}


void loop() {
//masurare unghi ghidon
  mas_unghi_ghidon = mas_unghi();
//masurare distanta
//  afis_distanta();    
//miscare stanga
  semnal_stanga();
//miscare dreapta
  semnal_dreapta();
//frana
  semnal_frana();
//activare semnalizare noaptea
  semnal_noapte();  
//afisare distanta spate      
  afis_distanta();
//calcul viteza distanta
//  calcul_viteza_distanta();  

//afisare distanta state
  if( mas_distanta_spate > dist_100cm){
      lcd.setCursor(13,2);
      lcd.print("----");
  }   
  else{
//      tmpDist_Spate = (float)mas_distanta_spate/100;
      lcd.setCursor(13,2);
      lcd.print((float)mas_distanta_spate/100);
  }  
//  delay(10);  
  if(viteza < 10){
    lcd.setCursor(10,0);
    lcd.print("   ");
    lcd.setCursor(11,0);
    lcd.print(viteza,0);
  }
  else{
    lcd.setCursor(10,0);
    lcd.print(viteza,0);
  } 
  lcd.setCursor(10,1);
  lcd.print(contor_distanta/100.0,3);
  if(sec<10){
    lcd.setCursor(14,3);
    lcd.print(" ");
    lcd.setCursor(15,3);
    lcd.print(sec);
  }
  else{
    lcd.setCursor(14,3);
    lcd.print(sec);
  }   
  lcd.setCursor(6,3);
  lcd.print(min);

//reset
  if(digitalRead(Buton_B3)==LOW){
    viteza = 0.0;
    contor_distanta = 0.0;
    old_distanta = 0.0;
    sec = 0;
    min = 0;
    start_stop = false;
  }
//start_stop
  if(digitalRead(Buton_B2)==LOW){
    start_stop = !start_stop;
  }  
//afisare start_stop
  if(start_stop == false){
    lcd.setCursor(19,0);
    lcd.print("S");
  }
  else{
    lcd.setCursor(19,0);
    lcd.print("R");
  } 

}

int mas_unghi(void){
  int tmpAnalog;

  tmpAnalog = analogRead(Unghi_Ghidon);
  return(tmpAnalog);
}

//miscare stanga
void semnal_stanga(void){
  int tmpS_Stanga;
  int tmp_i;

  tmpS_Stanga=digitalRead(S_Stanga);
  if(tmpS_Stanga == LOW || mas_unghi_ghidon < 400){
//  if(tmpS_Stanga == LOW ){    
    if(pulse_500msec == true){
      for(tmp_i=0;tmp_i<16;tmp_i++){  
        LED_Stanga[tmp_i] = CRGB ( 255, 255, 0);
      }  
      FastLED.show();
  //    delay(40);
    }   
    else{
      for(tmp_i=0;tmp_i<16;tmp_i++){  
        LED_Stanga[tmp_i] = CRGB ( 0, 0, 0);
      }  
      FastLED.show();
  //    delay(40);
    }       
  } 
  else{
      for(tmp_i=0;tmp_i<16;tmp_i++){  
        LED_Stanga[tmp_i] = CRGB ( 0, 0, 0);
      }  
      FastLED.show();
  //    delay(40);
  }                
} 

//miscare dreapta
void semnal_dreapta(void){
  int tmpS_Dreapta;
  int tmp_i;

  tmpS_Dreapta=digitalRead(S_Dreapta);
  if(tmpS_Dreapta == LOW || mas_unghi_ghidon > 600){
    if(pulse_500msec == true){
      for(tmp_i=0;tmp_i<16;tmp_i++)  
        LED_Dreapta[tmp_i] = CRGB ( 255, 255, 0);
      FastLED.show();
      delay(40);
    }
    else{
      for(tmp_i=0;tmp_i<16;tmp_i++)  
        LED_Dreapta[tmp_i] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
    }  
  }  
  else{
      for(tmp_i=0;tmp_i<16;tmp_i++)  
        LED_Dreapta[tmp_i] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
 }

//frana
void semnal_frana(void){  
  int tmpS_Frana;
  int tmp_i;

  tmpS_Frana = digitalRead(S_Frana); 
  if (tmpS_Frana==LOW){
      for(tmp_i=0;tmp_i<8;tmp_i++)  
        LED_Frana[tmp_i] = CRGB ( 255, 0, 0);
      FastLED.show();
      delay(40);
  }  
  else{  
      for(tmp_i=0;tmp_i<8;tmp_i++)  
        LED_Frana[tmp_i] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
}


//activare semnalizare noapte
void semnal_noapte(void){
  int tmp_i;
  int tmp_lumina;

  tmp_lumina = analogRead(Lumina);

  if(digitalRead(S_Noapte) == LOW || tmp_lumina >100){
    digitalWrite(L_Noapte, HIGH);
    for(tmp_i=0;tmp_i<8;tmp_i++)  
        LED_Ochi_Pisica[tmp_i] = CRGB ( 255, 0, 0);
    FastLED.show();
    delay(40);
  }
  else{
    digitalWrite(L_Noapte, LOW);
     for(tmp_i=0;tmp_i<8;tmp_i++)  
        LED_Ochi_Pisica[tmp_i] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  } 
}


//afisare distanta
void afis_distanta(void){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  perioada = pulseIn(echoPin, HIGH);
  mas_distanta_spate = (int)((perioada*0.034)/20);

  if(mas_distanta_spate <= dist_100cm && mas_distanta_spate > dist_80cm){
      LED_Distanta[0] = CRGB ( 255, 0, 0);
      LED_Distanta[1] = CRGB ( 255, 0, 0);
      LED_Distanta[2] = CRGB ( 0, 0, 0);
      LED_Distanta[3] = CRGB ( 0, 0, 0);
      LED_Distanta[4] = CRGB ( 0, 0, 0);
      LED_Distanta[5] = CRGB ( 0, 0, 0);
      LED_Distanta[6] = CRGB ( 0, 0, 0);
      LED_Distanta[7] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
  if(mas_distanta_spate <= dist_80cm && mas_distanta_spate > dist_70cm){
      LED_Distanta[0] = CRGB ( 255, 0, 0);
      LED_Distanta[1] = CRGB ( 255, 0, 0);
      LED_Distanta[2] = CRGB ( 255, 0, 0);
      LED_Distanta[3] = CRGB ( 255, 0, 0);
      LED_Distanta[4] = CRGB ( 0, 0, 0);
      LED_Distanta[5] = CRGB ( 0, 0, 0);
      LED_Distanta[6] = CRGB ( 0, 0, 0);
      LED_Distanta[7] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
  if(mas_distanta_spate <= dist_60cm && mas_distanta_spate > dist_m50cm){
      LED_Distanta[0] = CRGB ( 255, 0, 0);
      LED_Distanta[1] = CRGB ( 255, 0, 0);
      LED_Distanta[2] = CRGB ( 255, 0, 0);
      LED_Distanta[3] = CRGB ( 255, 0, 0);
      LED_Distanta[4] = CRGB ( 255, 0, 0);
      LED_Distanta[5] = CRGB ( 255, 0, 0);
      LED_Distanta[6] = CRGB ( 0, 0, 0);
      LED_Distanta[7] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
  if(mas_distanta_spate <= dist_m50cm){
      LED_Distanta[0] = CRGB ( 255, 0, 0);
      LED_Distanta[1] = CRGB ( 255, 0, 0);
      LED_Distanta[2] = CRGB ( 255, 0, 0);
      LED_Distanta[3] = CRGB ( 255, 0, 0);
      LED_Distanta[4] = CRGB ( 255, 0, 0);
      LED_Distanta[5] = CRGB ( 255, 0, 0);
      LED_Distanta[6] = CRGB ( 255, 0, 0);
      LED_Distanta[7] = CRGB ( 255, 0, 0);
      FastLED.show();
      delay(40);
  }
  if(mas_distanta_spate > dist_100cm){
      LED_Distanta[0] = CRGB ( 0, 0, 0);
      LED_Distanta[1] = CRGB ( 0, 0, 0);
      LED_Distanta[2] = CRGB ( 0, 0, 0);
      LED_Distanta[3] = CRGB ( 0, 0, 0);
      LED_Distanta[4] = CRGB ( 0, 0, 0);
      LED_Distanta[5] = CRGB ( 0, 0, 0);
      LED_Distanta[6] = CRGB ( 0, 0, 0);
      LED_Distanta[7] = CRGB ( 0, 0, 0);
      FastLED.show();
      delay(40);
  }
}

//afisare sir pe LCD
void  afis_ecran(char sir_afis[80]){
int   col;
int   linie;
int   i;

  i=0;
  for(linie=0;linie<4;linie++){
    for(col=0;col<20;col++){
      lcd.setCursor(col,linie);
      lcd.print(sir_afis[i]);
      i++;
    }
  }
}

/******************************************************************
 Functie calcul viteza si distanta
*******************************************************************/
void calcul_viteza_distanta(void){

  if(start_stop == false)
      return;
  contor_distanta += L_imp;
}

void setari(void){

//                  *       Linia 0    **     Linia 1      **   Linia 2        **       Linia 3    *  
//               01234567890123456789012345678901234567890123456789012345678901234567890123456789
  afis_ecran("Selectie diametru   1. D. roata: 24 inch2. D. roata: 26 inch3. D. roata: 29 inch");      
  while(1){
 //24 =30.48 ; 26=33.02, 29=36.83   
    if(digitalRead(Buton_B1)==LOW){      
      r_roata =0.305; 
      break;
    }
   if(digitalRead(Buton_B2)==LOW){      
      r_roata =0.330; 
      break;
    }
   if(digitalRead(Buton_B3)==LOW){      
      r_roata =0.368; 
      break;
    }
  }

//                  *       Linia 0    **     Linia 1      **   Linia 2        **       Linia 3    *  
//            01234567890123456789012345678901234567890123456789012345678901234567890123456789
  afis_ecran("Viteza:      Km/h   Distanta:       Km  Dist. spate:      m Timp:     min    sec");
}
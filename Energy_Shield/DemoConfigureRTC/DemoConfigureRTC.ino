/* ==================================================================================
 File:       DemoConfigureRTC.ino
 
 Author:	MCI Electronics
 www.olimex.cl
 
 Description:  Class constructor and function implementation for ADE7753
 IC with Arduino Energy Meter Shield MCI-PY-0087
 
 
 Target Device: Arduino Duemilanove, Uno, Mega
 ==================================================================================
 Copyright 2010 MCI electronics
 
 Licensed under the Apache License, 
 Version 2.0 (the "License"); you may not use this file except in compliance 
 with the License. You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0 
 
 Unless required by applicable law or agreed to in writing, software distributed
 under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied. See the License for
 the specific language governing permissions and limitations under the License.
 
 // ==================================================================================
 //
 // Original by E.M.U.
 // Modified by C.M.P.
 //
 //  Ver | dd mmm yyyy | Author  | Description
 // =====|=============|========|=====================================================
 // 1.00 | 25 Nov 2010 | E.M    | First release 
 // ==================================================================================
 // 2.00 | 15 Apr 2013 | C.M    | Improve ADE7753 dependency 
 // ==================================================================================*/


#include "Rtc.h"

#include <SPI.h>
#include <Wire.h>

#define RTC_ADDR 0xD0                // RTC Chip Address

#define TRANSMIT_RATE 1              // Wait time between 2 transmissions


void setup(){ 

  Serial.begin(9600);  // UART init

  // SPI Init
  SPI.setDataMode(SPI_MODE2);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();
  pinMode(10,OUTPUT);
  digitalWrite(10, HIGH);

  // I2C init (for RTC)
  Wire.begin();
  delay(1000);
}


char * preguntar(char * msj){
    char *p;
    char resp[255];
    p=resp;
    Serial.println(msj);
    while(!Serial.available())//esperar a que el usuario escriba algo
        ;
    while(Serial.available()>0){//Serial.peek() != -1)
        *(p++) = Serial.read();
        delay(30);//esperamos transferencia de datos por puerto serial
    }
    *(p++)=0;
    return resp;

}
  
 /**
 * wait until a key is sent to the arduino trough serial port
 * @param message to print to user
 * @return the key pressed
 */
char waitKey(char * msg){
    Serial.println(msg);
    while(Serial.available() == 0){
        delay(20);
    }
    return Serial.read();   
} 


void loop() { 
  //  unsigned long aa=0,wdiv=0,mode=0,offvolt=0;
  //  unsigned char chk[6];
  char resp[280], auxi[8];
  char flag_calibrar=1;
  int i;
  long tiempo_unix,consumo_previo=0, consumo_actual;
  long energia_nueva=0, energia_vieja=0, aparente_nueva=0, aparente_vieja=0;

  char seg=0,minu=0,hr=0,dia_semana=0,dia=0,mes=0,anio=0;
  char *dato, *p, *aux,constante=0;
  
  
  //inicializamos reloj
  Rtc reloj = Rtc(0,0,0,1,1,1,11,0xAA);
  
  
//  ADE7753 meter;
  Serial.println("+---------------------------+");
  Serial.println("| PROGRAMA DE CONFIGURACION |");
  Serial.println("|    ENERGY METER SHIELD    |");
  Serial.println("+---------------------------+");
  Serial.println("|  MCI Electronics - 2011   |");  
  Serial.println("+---------------------------+");
  
  Serial.println("\n\nConfiguracion de reloj");
  
  p=resp;
  aux=auxi;
  constante=1;
  while(constante){
      p=preguntar("Ingrese fecha en formato dd/mm/aa");
      
    /*  dato=strtok(p,"/");
      dia=*dato;
      Serial.println(dato);
      dato=strtok(NULL,"/");
      mes=*dato;
      Serial.println(dato);
      dato=strtok(NULL,"/");
      anio=*dato;
      Serial.println(dato);*/
      aux=strtok(p,"/");
      if(strlen(aux)!=2) Serial.println("Fecha no valida, ingresar dos digitos para el dia (no 7 sino 07)");
      dia=(aux[0]-48)*10+aux[1]-48;
      
      aux=strtok(NULL,"/");
      if(strlen(aux)!=2) Serial.println("Fecha no valida, ingresar dos digitos para el dia (no 7 sino 07)");
      mes=(aux[0]-48)*10+aux[1]-48;
      
      aux=strtok(NULL,"/");
      if(strlen(aux)>2) Serial.println("Solo los dos ultimos numeros del anio");
      anio=(aux[0]-48)*10+aux[1]-48;
      //validar fecha
      if(mes<0 || dia<0 || anio<0 || mes>12 || anio>99 ||
      (mes==1 && dia>31) ||
      (mes==2 &&    (  (dia>29 && !(anio%4==0 && anio%100==0 && anio&400!=0))   ||   (dia>29 && anio%4==0 && anio%100==0 && anio&400!=0)  )) ||
      (mes==3 && dia>31) ||
      (mes==4 && dia>30) ||
      (mes==5 && dia>31) ||
      (mes==6 && dia>30) ||
      (mes==7 && dia>31) ||
      (mes==8 && dia>31) ||
      (mes==9 && dia>30) ||
      (mes==10 && dia>31) ||
      (mes==11 && dia>30) ||
      (mes==12 && dia>31)
      ) 
          Serial.println("Fecha no valida");
      else{
          constante=0;
      }
  }
    




  constante=1;
  while(constante){
//  p=resp;
//  aux=auxi;
      p=preguntar("Ingrese hora en formato hh:mm:ss");
      
      aux=strtok(p,":");
      hr=(aux[0]-48)*10+aux[1]-48;
      
      aux=strtok(NULL,":");
      minu=(aux[0]-48)*10+aux[1]-48;
      
      aux=strtok(NULL,":");
      seg=(aux[0]-48)*10+aux[1]-48;
      
     
      //validar hora
      if(hr<0 || minu<0 || seg<0 || hr>23 || minu>59 || seg>59)
          Serial.println("Hora no valida");
      else{
          reloj.SetDate(seg,minu,hr,dia_semana,dia,mes,anio);
          Serial.println("Reloj configurado con exito");
          constante=0;
      }
  }
  

  
  //mostramos el reloj cada un segundo
  while(1){
      reloj.GetDate();
      Serial.print(reloj.Date());//fecha
      Serial.print(" - ");
      Serial.println(reloj.Time());//hora
      delay(992);
  }  
}

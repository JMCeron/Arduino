/* El siguiente programa es una demostración de las mediciones
* que el Energy Shield es capaz de realizar. Se abarcan medidas de Voltaje RMS 
* Corriente RMS , Energía Activa Acumulada 
* durante un período de tiempo particular y Período de la señal de voltaje (Canal 2),
* cabe destacar que la energía acumulada durante un segundo 
* es igual a la potencia activa.

* Para ejecutar las mediciones se ocupó la proporcionalidad existente entre el valor 
* entregado por el Energy Shield y el valor real de la magnitud calculada. 

* Las constantes de proporcionalidad fueron calculadas de forma experimental 
* (una demostración de aquellas pruebas se encuentran en el sketch DetConst),
* para el caso del Período la constante viene definida desde el datasheet.

* Las constante son las siguientes:

* Voltaje: kv=0,000000171849155 [volt/LSB]
* Corriente: ki=0,00000039122624397277 [amp/LSB]
* Energía Activa Acumulada: ke=0.00041576242446899414 [J/LSB]
* Período: kt=2.2*pow(10,-6) [seg/LSB]
*/

#include "ADE7753.h"  
#include "Rtc.h"
#include <SPI.h>   
#include <Wire.h>
#include <SoftwareSerial.h>

#define tXbee 3                     // SoftSerial Txpin
#define rXbee 2                     // SoftSerial Rxpin 
SoftwareSerial esXbee(rXbee,tXbee);

#define txGlcd 8                     // SoftSerial Txpin
#define rxGlcd 0                     // SoftSerial Rxpin 
SoftwareSerial esGLCD(rxGlcd,txGlcd);

#define ratioTraf (220/12)          //18.3

void setup(){
  
  // Xbee communications  
  pinMode(rXbee,INPUT);
  pinMode(tXbee,OUTPUT);
  
  // GLCD comm
//  pinMode(rxGlcd,INPUT);
  pinMode(txGlcd,OUTPUT);

  Serial.begin(9600);  // UART init
//  Serial.print("Setup");
  delay(100);
  
  esXbee.begin(9600);
  delay(100);
  
  esGLCD.begin(115200);
  serLCDSetup();
  backlightOn();
  delay(1000);
  clearLCD();
  
  

}

void loop() { 

ADE7753 meter = ADE7753();  
Rtc reloj = Rtc();

//esXbee.println("Setup");

meter.analogSetup(GAIN_1,GAIN_1, 0,0,0,0); 
meter.resetStatus();

long v1,i1,e1,e2,e3,ae1,ae2,ae3,r1,r2,r3;
float totEnergy = 0;

float kv,ki,ke,ka,kt,f1,basetime;
float voltage, current, energy, aparent, reactive, PF;
String typeLoad = "";
int t1;
int loopCounter = 1;


//Constantes de proporcionalidad.
kv = (ratioTraf)*VOLTDIV*(0.5/0x2851EC);    //(0.5/0x2851EC) From Datasheet                    
ki = CURRDIV*(0.5/0x17D338);  //(0.5/0x17D338) From Datasheet
ke = (10/16.0)*VOLTDIV*CURRDIV/4096.0; // 1/(2^12)
basetime = (1.0*NUMCYC)/100.0; // tiempo por el cual se acumula energia
kt=CLKIN/8;  //period register, resolution x.y[us]/LSB -per bit-

//ke = (1.0*NUMCYC)/4096.0; // El 1.0 esta relacionado con el numero de ciclos //1/2^12
//ke original es 100/4096, considerando las atenuaciones de entrada de voltaje y corriente

//Period Resolution
/*
la formula para el calculo de la frecuencia
f =(CLKIN/4)*(1/32)*16*(1/COUNTER)

f = CLKIN/(8*COUNTER)
T = 1/f = 8*COUNTER/CLKIN

kt = CLKIN/8
frecuency = kt/COUNTER
period = COUNTER/kt

If COUNTER = 10.000, the frecuency is 50[Hz]
*/

  while(1){
  
    //--Reloj de tiempo real--
    reloj.GetDate();
    //--Medición de Voltaje--
    v1=meter.vrms();
    //--Medición de Corriente--
    i1=meter.irms();
    //--Medición de Período--
    t1=meter.getPeriod();
    f1 = kt/t1; //se calcula la frecuencia
    
    /*********************************************************/
    //Medición de Energía Activa Acumulada
    meter.setMode(0x0080); //Se inicia el modo de acumulación de energía.
    meter.setLineCyc(1*NUMCYC); //Se fija el número de medios ciclos ocupados en la medición. 10 medio ciclos equivalen a 0,1 segundo trabajando en una red de 50 Hz (Chile).
    e1=meter.getLAENERGY(); //Extrae la energía activa acumulada, sincronizando la medición con los cruces por cero de la señal de voltaje. 
    
    meter.setMode(0x0080);
    meter.setLineCyc(2*NUMCYC); // 0,2 segundos de medición.
    e2=meter.getLAENERGY();
    
    meter.setMode(0x0080);
    meter.setLineCyc(3*NUMCYC); // 0,3 segundos de medición.
    e3=meter.getLAENERGY();
      
    /*********************************************************/
    //Medición de Energía Aparente Acumulada
    meter.setMode(0x0080); //Se inicia el modo de acumulación de energía.
    meter.setLineCyc(1*NUMCYC); //Se fija el número de medios ciclos ocupados en la medición. 10 medio ciclos equivalen a 0,1 segundo trabajando en una red de 50 Hz (Chile).
    ae1=meter.getLVAENERGY(); //Extrae la energía aparente acumulada, sincronizando la medición con los cruces por cero de la señal de voltaje. 
    
    meter.setMode(0x0080);
    meter.setLineCyc(2*NUMCYC); // 0,2 segundos de medición.
    ae2=meter.getLVAENERGY();
    
    meter.setMode(0x0080);
    meter.setLineCyc(3*NUMCYC); // 0,3 segundos de medición.
    ae3=meter.getLVAENERGY();
  
    /*********************************************************/
    //Medición de Energía Reactiva Acumulada
    meter.setMode(0x0080); //Se inicia el modo de acumulación de energía.
    meter.setLineCyc(10); //Se fija el número de medios ciclos ocupados en la medición. 10 medio ciclos equivalen a 1 segundo trabajando en una red de 50 Hz (Chile).
    r1=meter.getReactiveEnergy(); //Extrae la energía activa acumulada, sincronizando la medición con los cruces por cero de la señal de voltaje. 
    
    meter.setMode(0x0080);
    meter.setLineCyc(20); // 0,2 segundos de medición.
    r2=meter.getReactiveEnergy();
    
    meter.setMode(0x0080);
    meter.setLineCyc(30); // 0,3 segundos de medición.
    r3=meter.getReactiveEnergy();
  
    /*********************************************************/
  
    /* Si tenemos los valores de energia activa y aparente
    podemos realizar el calculo, y nos evitamos hacer nuevamente
    las mediciones ahorrando tiempo. 
    */
  //  PF = getFPOWER();
    PF = calcFPOWER(e2,e3,ae2,ae3,ke);
    /* El calculo del Factor de Potencia es solo referencial, y su uso actual se limita
     * a saber si es distinto de cero y si la carga es de tipo Inductiva o 
     *
    */
    
    Serial.println("\n------------------------------------------");
    Serial.print(reloj.Date());//fecha
    Serial.print(" - ");
    Serial.print(reloj.Time());//hora
    Serial.print("\nVoltaje RMS [V]: ");
    Serial.print(kv*v1,2);
    Serial.print("\nCorriente RMS [A]: ");
    Serial.print(ki*i1,3);
  
    energy = ke*(e3-e2)/basetime;
    if(fabs(energy ) > 1e6){energy = 0.0;}
    Serial.print("\nPotencia Activa [W] (Acumulada en [J] por 0,1 seg :) ");
    Serial.print(fabs(energy),2);  //Se ve la diferencia entre la energía activa acumulada en 0,3 segundos y la acumulada en 0,2 segundos. 
    //Se desprecia la energía activa acumulada en 0,1 segundo ya que los valores de mediciones iniciales no siempre son correctas. 
  
    aparent = ke*(ae3-ae2)/basetime;
    if(fabs(aparent ) > 1e6){aparent = 0.0;}
    Serial.print("\nPotencia Aparente [VA] (Acumulada en [J] por 0,1 seg :) ");
    Serial.print(fabs(aparent),2);  //Se ve la diferencia entre la energía aparente acumulada en 0,3 segundos y la acumulada en 0,2 segundos. 
    //Se desprecia la energía activa acumulada en 0,1 segundo ya que los valores de mediciones iniciales no siempre son correctas. 
  
    reactive = ke*(r3-r2)/basetime;
    if(fabs(reactive ) > 1e6){reactive = 0.0;}
    Serial.print("\nPotencia Reactiva [var] (Acumulada en [J] por 0,1 seg :) ");
    Serial.print(fabs(reactive),2);  //Se ve la diferencia entre la energía reactiva acumulada en 1 segundos y la acumulada en 2 segundos. 
  
    Serial.print("\nPeriodo de la senal de voltaje [ms]: ");
    Serial.print(1000*(t1/kt),0);    
   
    Serial.print("\nFrecuencia de la senal de voltaje [Hz]: ");
    Serial.print(f1);  
    
    if(reactive > 0)
      typeLoad = "cap";
    else if (reactive < 0)
      typeLoad = "ind";
    else
      typeLoad = "___";
      
    Serial.print("\nTipo de Carga : ");
    Serial.println(typeLoad); 
    
    Serial.print("\nFactor de Potencia: ");
    Serial.println(PF,2);  
    
    totEnergy += fabs(energy)*basetime;
    Serial.print("Energia Total estimada [Wh]:  ");
    Serial.println(totEnergy/(3600));

    /*************************************************************************/
    
     if( (loopCounter++%20) == 0 ){
      clearLCD();
      //imprimir el patron de diseno
    }

    gotoLine(1);
    esGLCD.print(reloj.Time());//hora
    esGLCD.write("  ");
    esGLCD.print(1000*(t1/kt),0);
    esGLCD.write("ms ");
    esGLCD.print(f1,0);  
    esGLCD.write("Hz");
    
    serLine(1,0,54,127,54);
    lcdPosition(0,11);
    esGLCD.write("  V   C");
    lcdPosition(60,11);
    esGLCD.write(" P     S   ");
    
//    gotoLine(3);
    gotoLine(4);
    esGLCD.print( kv*v1,0 );
    esGLCD.write(" ");
    esGLCD.print( ki*i1,2);
    
    lcdPosition(60,24);    
    esGLCD.print(fabs(energy),1);     
    esGLCD.write("  ");
    esGLCD.print(fabs(aparent),1);
    serLine(1,58,54,58,31);
    serLine(1,0,30,127,30);
//    gotoLine(4);
    
    
    gotoLine(5);
    
  
    gotoLine(6);

  
    gotoLine(7);
    esGLCD.write("PF = ");
    esGLCD.print(PF);
    
    gotoLine(8);
    esGLCD.print(typeLoad);
    delay(200);
    
 
    /***************************************************************************/

    esXbee.print(reloj.Date());//fecha
    esXbee.write(" - ");
    esXbee.println(reloj.Time());//hora
    
    esXbee.write("Voltaje [V]: ");
    esXbee.println( kv*v1,2 );
    
    esXbee.write("Corriente [A]: ");
    esXbee.println( ki*i1,3);
    
    esXbee.write("Potencia Activa [W]: ");
    esXbee.println(fabs(energy),2);  //Se ve la diferencia entre la energía activa acumulada en 3 segundos y la acumulada en 2 segundos. Se desprecia la energía activa acumulada en 1 segundo ya que los valores de mediciones iniciales no siempre son correctas. 
    
    esXbee.write("Potencia Aparente [VA]: ");
    esXbee.println(fabs(aparent),2);  //Se ve la diferencia entre la energía activa acumulada en 3 segundos y la acumulada en 2 segundos. Se desprecia la energía activa acumulada en 1 segundo ya que los valores de mediciones iniciales no siempre son correctas. 
  
    esXbee.write("Potencia Reactiva [var]: ");
    esXbee.println(fabs(reactive),2);  //Se ve la diferencia entre la energía activa acumulada en 3 segundos y la acumulada en 2 segundos. Se desprecia la energía activa acumulada en 1 segundo ya que los valores de mediciones iniciales no siempre son correctas. 
  
  
    esXbee.print(1000*(t1/kt),0);
    esXbee.write(" [ms] -  ");
    esXbee.print(f1);  
    esXbee.write(" [Hz]");
    
    esXbee.println("");
    esXbee.write("PF = ");
    esXbee.print(PF);
    esXbee.write(" - ");
    esXbee.println(typeLoad);
    esXbee.println("++++");
    delay(200);


  }
}

float calcFPOWER(long e2, long e3, long a2, long a3, float ke){

  float PF;

  if( fabs(ke*(e3-e2))  <= 0.0000002){
    PF = 1.0;
  }
  else if ( fabs(ke*(a3-a2)) <= 0.0000002)
  {
    PF = 0.0;
    Serial.println("WARNING - Power Factor");
  }
  else if( (ke*(a3-a2)) >= (ke*(e3-e2)) ) {
    PF = fabs((ke*(e3-e2))/(ke*(a3-a2)));
  }
  else{
    PF = fabs((ke*(a3-a2)) / (ke*(e3-e2)) );
  }
  
  if(PF > 1.0)
    PF = 2-PF;
    
  if( (PF > 1.0) || (PF < 0.0000002)){
   PF = 1.0;
    Serial.println("WARNING - Power Factor");
  }
    
  return PF;
}


void setup() {
 Serial.begin(9600); //Inicio del serial a 9600 baudios
 }

void loop() {
 //Escribe 'h' y 'l' en intervalos de un segundo vía Serial, que XBee emitirá por radio
 Serial.print('h');
 delay(1000);      //Retrasos de 1 segundo
 Serial.print('1');
 delay(1000); 
}

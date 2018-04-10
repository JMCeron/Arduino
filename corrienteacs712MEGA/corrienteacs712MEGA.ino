float Sensibilidad=0.1315; //Sensibilidad 30A
float Offset=0;

void setup() {
  
  Serial.begin(9600);
}

void loop() {
  
  float I=get_corriente(200);//obtenemos la corriente promedio de 500 muestras 
  Serial.print("Corriente: ");
  Serial.println(I,3); 
  delay(1000);     
}

float get_corriente(int n_muestras)
{
  float voltajeSensor;
  float corriente=0;
  for(int i=0;i<n_muestras;i++)
  {
    voltajeSensor = analogRead(A0) * (4.96 / 1023.0);////lectura del sensor
    corriente=corriente+(voltajeSensor-0.495)/Sensibilidad-Offset; //Ecuación  para obtener la corriente
  }
  corriente=corriente/n_muestras;
  return(corriente);
}




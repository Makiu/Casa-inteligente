#include <DHT.h> // Librería para el sensor de humedad y temperatura DHT11
#include <LiquidCrystal.h> // Librería para el LCD
#include <Servo.h> // Librería para el servo

int ldrPin = A0; // Pin analógico del LDR
int measPin = A1; // Pin analógico del MEAS

int echoPin = 8; // Pin digital para el receptor del sensor de distancia
int triggerPin = 9; // Pin digital para el emisor del sensor de distancia
int ledPin = 7; // Pin digital para el led
int buzzerPin = 13; // Pin digital para el timbre

int ldrValue = 0; // Variable para almacenar el valor que recibimos del LDR
int distanceValue = 0; // Variable para almacenar el valor que recibimos del sensor de distancia
int measValue = 0; // Variable para almacenar el valor que recibimos del MEAS
int incomingByte; // Variable para almacenar la información que recibimos del móvil
int alarmType = 0; // Variable para distinguir entre la alarma de incendios y la de robo en el display
float humValue = 0; // Variable para almacenar el valor que recibimos de humedad
float tempValue = 0; // Variable para almacenar el valor que recibimos de temperatura

int ldrState = 0; // Variable para determinar el estado del sistema de luz
int distanceState = 0; // Variable para determinar el estado del sistema de distancia
int dhtState = 0; // Variable para determinar el estado del sistema de humedad y temperatura
int vibrState = 0; // Variable para determinar el estado del sistema de vibración

// Arrays para crear los carácteres especiales del display(luz, humedad, temperatura, etc)
byte lightSignal[8] = { 0b00100, 0b10101, 0b01110, 0b11011, 0b01110, 0b10101, 0b00100, 0b00000};
byte humedad[8] = { 0b00000, 0b00100, 0b01010, 0b01010, 0b10001, 0b10001, 0b01110, 0b00000};
byte temperatura[8] = { 0b00100, 0b01010, 0b01010, 0b01110, 0b11111, 0b11111, 0b01110, 0b00000};
byte llave[8] = { 0b01110, 0b10001, 0b01010, 0b00100, 0b00110, 0b00100, 0b00111, 0b00000};
byte fuego[8] = { 0b10001, 0b00100, 0b01110, 0b11011, 0b10001, 0b10101, 0b01110, 0b00000};
byte grados[8] = { 0b01110, 0b01010, 0b01110, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};


Servo myservo; // Crea un objeto para controlar el servo
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // Declara los pines que usamos para el display
#define DHTPIN 10    //Data
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
 Serial.begin(9600); // Abrimos el puerto serie
 // Declaramos algunos pines como salidas o entradas
 pinMode(ledPin, OUTPUT);
 pinMode (triggerPin, OUTPUT);
 pinMode (echoPin, INPUT);
 myservo.attach(6); // Declaramos el pin del servo en el pin 6
 myservo.write (0); // Reseteamos la posición del servo
 lcd.begin(16, 2); // Fijamos el número de filas y columnas del display
 dht.begin();

 // Asignamos a cada array un nombre para después poder imprimirlos en el display
 lcd.createChar(1, lightSignal);
 lcd.createChar(2, humedad);
 lcd.createChar(3, temperatura);
 lcd.createChar(4, llave);
 lcd.createChar(5, fuego);
 lcd.createChar(6, grados);



}

void loop() {

 int h = dht.readHumidity();
 int t = dht.readTemperature();

 if (Serial.available() > 0) { // Comprueba que haya datos entrantes
    incomingByte = Serial.read(); // Lee el dato y lo almacena en la variable
    comunicacion(); // Activamos un subsistema según lo que hayamos recibido

 }

 //Según que subsistema hayamos seleccionado realizaremos el bloque correspondiente al subsistema
 if (dhtState == 1) { // DHT
    ambiente();
 }

 if  (ldrState == 1) { // LUZ
    luz();
 }

 if  (distanceState == 1) { // DISTANCIA
    distancia();
 }

 if (vibrState == 1) { // MEAS
    vibracion();
 }

 // Imprimimos los valores en el puerto serie
 Serial.print ("   Ldr:");
 Serial.print (ldrValue);
 Serial.print ("   Distancia: ");
 Serial.print (distanceValue);
 Serial.print ("   Vibracion: ");
 Serial.println (measValue);

 lcd.setCursor(0, 0); // Nos situamos en la primera columna y primera fila

 // Imprimimos el carácter del subsistema de luz, el valor del sensor y dejamos un espacio en blanco
 lcd.write(1);
 lcd.print(ldrValue);
 lcd.print("%  ");
 lcd.write(3);
 lcd.print(t);
 lcd.write(6);
 lcd.print("C  ");
 lcd.write(2);
 lcd.print(h);
 lcd.print("%");


 lcd.setCursor(0, 1); // Nos situamos en la primera columna y segunda fila

 // Según el tipo de alarma muestra la alarma de incendios o la de robo( o ninguna)
 if (alarmType == 0) {
    lcd.setCursor(0, 8);
    lcd.print("             ");
 }



 if (alarmType == 1) {
    lcd.write(4);
    lcd.print(" ATENCION ");
    lcd.write(4);
 }
 if (alarmType == 2) {
    lcd.write(5);
    lcd.print(" ATENCION ");
    lcd.write(5);
 }
delay(200);
}

// LUZ

void luz() {
 ldrValue = analogRead(ldrPin); // Leemos los valores que recibe el LDR
 ldrValue = map(ldrValue,0,1023,0,100);

 // Cuando ese valor sea mayor a 400 se enciende el led
 if (ldrValue < 400) {
    digitalWrite(ledPin, HIGH);
 }
 else {
    digitalWrite(ledPin, LOW);
 }

}

// DISTANCIA

void distancia() {
 // Inicializamos el sensor
 digitalWrite (triggerPin, LOW);
 delayMicroseconds(5);
 // Enviamos una señal activando la salida trigger durante 10 microsegundos
 digitalWrite(triggerPin, HIGH);
 delayMicroseconds(10);
 digitalWrite (triggerPin, LOW);
 // Medimos el ancho del pulso, cuando la lectura sea HIGH
 // Devuelve el tiempo transcurrido en microsegundos
 distanceValue = pulseIn(echoPin, HIGH);
 // Calculamos la distancia en cm
 distanceValue = distanceValue * 0.01715;
 // Enviamos los datos medidos a través del puerto serie

 // Cuando la señal sea menor que 17 la puerta sube
 if (distanceValue > 17) {
    myservo.write (0);

 }
 else {

    myservo.write(90);
 }
}

// VIBRACIÓN

void vibracion() {
 measValue = analogRead(measPin); // Leemos los valores que recibe el MEAS

 // Cuando dicho valor sea mayor a 300 se activa el zumbador y cambia al tipo de alarma 1
 if (measValue > 300) {
    alarmType = 1;
    analogWrite(buzzerPin, 128);
    delay(50);
    digitalWrite(buzzerPin, LOW);
    delay(50);
 }

 if (measValue < 300)
    alarmType = 0;

}

// DHT11(HUMEDAD Y TEMPERATURA)

void ambiente() {
 humValue = dht.readHumidity();
 tempValue = dht.readTemperature();

}


// COMUNICACIÓN

void comunicacion() {

 // Activamos o desactivamos un subsistema según lo que hayamos recibido
 if (incomingByte == 'L') {
    ldrState = 1;

 }

 if (incomingByte == 'l') {
    ldrState = 0;

 }

 if (incomingByte == 'D') {
    distanceState = 1;
 }

 if (incomingByte == 'd') {
    distanceState = 0;
 }
 if (incomingByte == 'T') {
    dhtState = 1;
 }

 if (incomingByte == 't') {
    dhtState = 0;
 }
 if (incomingByte == 'V') {
    vibrState = 1;
 }

 if (incomingByte == 'v') {
    vibrState = 0;
 }
}

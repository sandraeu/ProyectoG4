#include <SoftwareSerial.h>
//---- variables de la comunicacion -------
SoftwareSerial btSerial(11, 12); // RX, TX
String serialData = "";
char movementCommand = 'a';
//-----------------------------------------

// --------------- variables para el sensor de flujo ----------
volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
int PinSensor = 2;    //Sensor conectado en el pin 2
float factor_conversion = 7.5; //para convertir de frecuencia a caudal
float litros;
float litro;
unsigned long pulsos_Acumulados = 0;
//-------------------------------------------------------------

// ------------------------ variables sensor ultrasonico ------------------
// Pines utilizados
#define TRIGGER 7
#define ECHO 8

// Constantes
const float sonido = 34300.0; // Velocidad del sonido en cm/s
//--------------------------------------------------------------------------

//---------------------- pines de los led ---------------------
int  led1 = 22;
int  led2 = 23;
int  led3 = 24;
int  led4 = 25;
int  led5 = 26;
int  led6 = 27;
int  led7 = 28;
int  led8 = 29;
int  led9 = 30;
int  led10 = 31;

int  ledv = 32;
int  leda = 33;
int  ledr = 34;
//--------------------------------------------------------------
// ----------------------- variables de control del programa ---------
int estado = 0; // 0 sistema apagado , 1 sistema encendido
int estado_sensor = 0; // 0 cierra la valvula , 1 deja pasar agua
// -------------------------------------------------------------------
void setup() {
  btSerial.begin(9600);
  pinMode(5, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);
  pinMode(led9, OUTPUT);
  pinMode(led10, OUTPUT);

  pinMode(ledv, OUTPUT);
  pinMode(leda, OUTPUT);
  pinMode(ledr, OUTPUT);

  digitalWrite(5 , LOW);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);
  digitalWrite(led6, LOW);
  digitalWrite(led7, LOW);
  digitalWrite(led8, LOW);
  digitalWrite(led9, LOW);
  digitalWrite(led10, LOW);

  digitalWrite(ledv, LOW);
  digitalWrite(leda, LOW);
  digitalWrite(ledr, LOW);

  // sensor de flujo
  Serial.begin(9600);
  pinMode(PinSensor, INPUT);
  attachInterrupt(0, ContarPulsos, RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  //-----

  // Modo entrada/salida de los pines
  pinMode(ECHO, INPUT);
  pinMode(TRIGGER, OUTPUT);
}

void loop() {
  //---------------------- SENSOR ULTRASONICO --------------------------------
  // Preparamos el sensor de ultrasonidos
  iniciarTrigger();
  // Obtenemos la distancia
  float distancia = calcularDistancia();
  //---------------------------------------------------------------------------

  //-------------------------------- COMUNICACION ----------------------------
  serialData = "";
  while (btSerial.available())
  {
    serialData.concat(char(btSerial.read()));
  }

  movementCommand = serialData.charAt(0);

  if (movementCommand != '\0')
  {
    switch (movementCommand)
    {
      case 'e': // abre la valvula
        digitalWrite(5 , LOW);
        btSerial.println("e");
        estado = 1;
        break;
      case 'a':
        digitalWrite(5, HIGH);
        btSerial.println("a");
        estado = 0;
        break;
      case 'v':
        digitalWrite(ledv, HIGH);
        digitalWrite(leda, LOW);
        digitalWrite(ledr, LOW);
        break;
      case 'm':
        digitalWrite(ledv, LOW);
        digitalWrite(leda, HIGH);
        digitalWrite(ledr, LOW);
        break;
      case 'r':
        digitalWrite(ledv, LOW);
        digitalWrite(leda, LOW);
        digitalWrite(ledr, HIGH);
        break;
    }
  }
  //----------------------------------------------------------------------------------

  // ----- sensor de flujo ----
  float frecuencia = ObtenerFrecuencia(); //obtenemos la Frecuencia de los pulsos en Hz
  float caudal_L_m = frecuencia / factor_conversion; //calculamos el caudal en L/m
  float caudal_L_h = caudal_L_m * 60; //calculamos el caudal en L/h


  // ------------------------------------------------------ ACCIONES SI EL SISTEMA ESTA ACTIVO ----------------------------------
  if (estado == 1) {
Serial.println(" ACTIVO");
    litro ++;
    btSerial.print (litros, 3);
    btSerial.println (" Litros");

    if (estado_sensor == 1) {
      digitalWrite(5 , HIGH);

    } else {
      digitalWrite(5 , LOW);
    }

    //------------------------ INDICADOR LED DE CONSUMO SE AGUA ---------------------
    switch ((int)litro) {
      case 10 :
        digitalWrite(led1 , HIGH);
        break;

      case 20:
        digitalWrite(led2, HIGH);
        break;
      case 30 :
        digitalWrite(led3 , HIGH);
        break;

      case 40:
        digitalWrite(led4, HIGH);
        break;
      case 50 :
        digitalWrite(led5 , HIGH);
        break;

      case 60:
        digitalWrite(led6, HIGH);
        break;
      case 70 :
        digitalWrite(led7 , HIGH);
        break;

      case 80:
        digitalWrite(led8, HIGH);
        break;

      case 90 :
        digitalWrite(led9 , HIGH);
        break;

      case 100:
        digitalWrite(led10, HIGH);
        break;
    }
    //--------------------------------------------------------------------------

  } else {
    estado = 0;
    litros = 0;
    litro = 0;
    digitalWrite(5 , LOW);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
    digitalWrite(led6, LOW);
    digitalWrite(led7, LOW);
    digitalWrite(led8, LOW);
    digitalWrite(led9, LOW);
    digitalWrite(led10, LOW);
    digitalWrite(ledv, LOW);
    digitalWrite(leda, LOW);
    digitalWrite(ledr, LOW);
  }
  //------------------------------------------------------------------------------------------------------------------------------
}

void ContarPulsos ()
{
  NumPulsos++;  //incrementamos la variable de pulsos
}

//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuencia()
{
  int frecuencia;
  NumPulsos = 0;   //Ponemos a 0 el número de pulsos
  interrupts();    //Habilitamos las interrupciones
  delay(1000);   //muestra de 1 segundo
  noInterrupts(); //Desabilitamos las interrupciones
  frecuencia = NumPulsos; //Hz(pulsos por segundo)
  pulsos_Acumulados += NumPulsos;
  litros = pulsos_Acumulados * 1.0 / 450;
  return frecuencia;
}

//-------------------------- SENSOR ULTRASONICO --------------------------------------------

// Método que calcula la distancia a la que se encuentra un objeto.
// Devuelve una variable tipo float que contiene la distancia
float calcularDistancia()
{
  // La función pulseIn obtiene el tiempo que tarda en cambiar entre estados, en este caso a HIGH
  unsigned long tiempo = pulseIn(ECHO, HIGH);

  // Obtenemos la distancia en cm, hay que convertir el tiempo en segudos ya que está en microsegundos
  // por eso se multiplica por 0.000001
  float distancia = tiempo * 0.000001 * sonido / 2.0;
  Serial.print(distancia);
  Serial.print(" cm ");
  activar(distancia);
  delay(500);

  return distancia;
}

void activar(float dis) {

  if (dis <= 30) {
    Serial.print(" activar agua");
    estado_sensor = 1;
    Serial.println();
  } else {
    Serial.print(" Desactivar agua");
    estado_sensor = 0;
    Serial.println();
  }

}

// Método que inicia la secuencia del Trigger para comenzar a medir
void iniciarTrigger()
{
  // Ponemos el Triiger en estado bajo y esperamos 2 ms
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  // Ponemos el pin Trigger a estado alto y esperamos 10 ms
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);

  // Comenzamos poniendo el pin Trigger en estado bajo
  digitalWrite(TRIGGER, LOW);
}
//------------------------------------------------------------------------

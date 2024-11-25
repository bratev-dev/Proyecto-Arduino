#include "StateMachineLib.h"
#include "AsyncTaskLib.h"
#include <Keypad.h>
#include "Average.h"
#include <string.h>
#include <LiquidCrystal.h>
#include "DHT.h"

//---------------------------------------------------------
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
//---------------------------------------------------------

//0 cuando hay un objeto cerca = infrarojo
//1 cuando hay campo magnetico = hall

Average<float> aveT(10);
Average<float> aveL(10);

#define DEBUG(a) \
  Serial.print(millis()); \
  Serial.print(": "); \
  Serial.println(a);

//diego
//#define LED_ROJO 46
//#define LED_AZUL 48
//#define LED_VERDE 50

//nuestro
#define LED_VERDE 9
#define LED_ROJO 10
#define LED_AZUL 8
#define ZUMBADOR 7

#define DHTPIN 40    // Digital pin connected to the DHT sensor
#define beta 4090  //the beta of the thermistor, for the temp
#define pinLight A0

#define pinIFR 13 // Sensor Infrarrojo
#define pinHall 6 // Sensor Hall

#define DHTTYPE DHT11   // DHT 11
#define TH_TEMP_HIGH 27.3



const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;// nuestro
//const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 =8,d7=7;//diego
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

double TempC;
double TempF;
double Light;
double Humedad;

unsigned char pinIFRon = LOW;
unsigned char pinHallon = LOW;

const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte filaPines[FILAS] = { 24, 26, 28, 30 };
byte columnaPines[COLUMNAS] = { 32, 34, 36, 38 };
Keypad keypad = Keypad(makeKeymap(teclas), filaPines, columnaPines, FILAS, COLUMNAS); //inicializar el keypad
DHT dht(DHTPIN, DHTTYPE); //inicializar sensor DHT

unsigned char bandera = 0;
unsigned char indice = 0;
unsigned char intentos = 0;
unsigned char contadorAlerta = 0;
char clave[5];
char contrasenia[5] = "12345";

//--------------------------------------------------------------
// change this to make the song slower or faster
int tempo = 105;


// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // Pacman
  // Score available at https://musescore.com/user/85429/scores/107109
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
  NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
  NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

  NOTE_B4, 16,  NOTE_B5, 16,  NOTE_FS5, 16,   NOTE_DS5, 16,  NOTE_B5, 32,  //2
  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_DS5, 32, NOTE_E5, 32,  NOTE_F5, 32,
  NOTE_F5, 32,  NOTE_FS5, 32,  NOTE_G5, 32,  NOTE_G5, 32, NOTE_GS5, 32,  NOTE_A5, 16, NOTE_B5, 8
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;
//----------------------------------------------------------------------------------

// State Alias
enum State {
  INICIO = 0,
  MONITOREO = 1,
  BLOQUEADO = 2,
  EVENTOS = 3,
  ALERTA = 4,
  ALARMA = 5
};

// Input Alias
enum Input {
  Sign_T = 0,
  Sign_P = 1,
  Sign_B = 2,
  Sign_K = 3,
  Unknow = 4
};

Input input;/*! variable global para leer el input que hace que cambie de estado la maquina */

float tempC = 0.0;/*! variable global para leer la temperatura */

/** @brief Tarea para prender luz roja en estado bloqueado con frecuencia de 500ms */
AsyncTask TareaPrenderLuz(500, []() { digitalWrite(LED_ROJO, HIGH); });
/** @brief Tarea para apagar luz roja en estado bloqueado con frecuencia de 500ms */
AsyncTask TareaApagarLuz(500, []() { digitalWrite(LED_ROJO, LOW); });
/** @brief Tarea para prender luz azul en estado alerta con frecuencia de 800ms */
AsyncTask TareaPrenderLuzAlerta(800, []() { digitalWrite(LED_AZUL, HIGH); });
/** @brief Tarea para apagar luz roja en estado alerta con frecuencia de 800ms */
AsyncTask TareaApagarLuzAlerta(800, []() { digitalWrite(LED_AZUL, LOW); });


void timeout(void) {
  Serial.println("Contador Alerta Timeout = ");
  Serial.println(contadorAlerta);
  if (contadorAlerta > 2){
    input = Input::Sign_B;
    return;
  }
  input = Input::Sign_T;

}
AsyncTask TaskTiempo(3000, false, timeout);

void timeoutPassword(void){
    lcd.setCursor(0, 0);
    lcd.print("Clave Incorrecta");
    intentos++;
    digitalWrite(LED_AZUL, HIGH);
    delay(1000);
    digitalWrite(LED_AZUL, LOW);
    indice = 0;
    limpiarPantalla();
}
AsyncTask TaskTiempoLimite(5000,false,timeoutPassword);

void read_keypad(void) {
  char key = keypad.getKey();
  Serial.println(key);
  if (key) {
    Serial.println(key);
    if (key == '*') {
      input = Input::Sign_K;  // cuando recive un * revuelve la señal k
    }
  }
}
AsyncTask TaskKeypad(100, true, read_keypad);


void mostrarDigite() {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite la clave:");
}
AsyncTask TareaDigiteClave(1, false, mostrarDigite);


void limpiarPantalla() {
  lcd.clear();
  TareaDigiteClave.Start();
}
AsyncTask TareaLimpiarPantalla(2000, false, limpiarPantalla);


void activarAlarma() {
  for (int i = 200; i <= 800; i++)  // bucle de frecuencia de 200 a 800
  {
    tone(ZUMBADOR, i);  // activar el zumbador
    //delay(2);           // esperar 2 milisegundos
  }
  for (int i = 800; i >= 200; i--)  // bucle de frecuencia de 800 a 200
  {
    tone(ZUMBADOR, i);
    //delay(3);  // esperar 3 milisegundos
  }
}
AsyncTask TareaActivarAlarma(0, false, activarAlarma);


void detenemosAlarma() {
  noTone(ZUMBADOR);
  bandera = 1;
}
AsyncTask TareaDetenemosAlarma(3000, false, detenemosAlarma);


void manejarTeclado() {
  char tecla = keypad.getKey();

  if (tecla) {
    TaskTiempoLimite.Start();
    lcd.setCursor(indice, 1);
    lcd.print("*");
    clave[indice++] = tecla;
  }
}
AsyncTask TareaManejarTeclado(0, true, manejarTeclado);


void manejarSistema() {
  if (intentos > 2) {
    lcd.begin(16, 2);
    lcd.print("SISTEMA BLOQUEADO");
    intentos = 0;
    input = Input::Sign_B;
    //digitalWrite(LED_ROJO, LOW);
  }
}
AsyncTask TareaManejarSistema(0, true, manejarSistema);


void manejarContrasenia() {
  if (indice > 4) {
    if (strncmp(clave, contrasenia, 5) == 0) {

      lcd.begin(16, 2);
      lcd.print("Clave Correcta");
      digitalWrite(LED_VERDE, HIGH);
      delay(3000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BIENVENIDO :)");
      intentos = 0;
      input = Input::Sign_P;
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Clave Incorrecta");
      intentos++;
      digitalWrite(LED_AZUL, HIGH);
      delay(1000);
      limpiarPantalla();
    }
    lcd.clear();
    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_AZUL, LOW);
    digitalWrite(LED_VERDE, LOW);
    indice = 0;
  }
}
AsyncTask TareaManejarContrasenia(0, true, manejarContrasenia);


void readTemperature() {
  Humedad = dht.readHumidity();
  // Read temperature as Celsius (the default)
  TempC = dht.readTemperature();

  if (TempC >= TH_TEMP_HIGH){
    input=Input::Sign_P;
  }
  // Check if any reads failed and exit early (to try again).
  if (isnan(Humedad) || isnan(TempC)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(TempC, Humedad, false);

  Serial.print(F("Humidity: "));
  Serial.print(Humedad);
  Serial.print(F("%  Temperature: "));
  Serial.print(TempC);
  Serial.print(F("°C "));
  //a
  // long a = 1023 - analogRead(pinTemp);
  // TempC = beta / (log((1025.0 * 10 / a - 10) / 10) + beta / 298.0) - 273.0;
  // TempF = TempC + 273.15;
  // Serial.print("temp: ");
  // Serial.print(TempC);
  // Serial.println(" C");
  // Serial.print("temp: ");
  // Serial.print(TempF);
  // Serial.println(" F");

  // Serial.print("Temp promedio:   ");
  // Serial.println(aveT.mean());
  // aveT.push(TempC);
}
AsyncTask TaskTemp(1200, true, readTemperature);


void readLight() {
  Light = analogRead(pinLight);
  Serial.print("Luz: ");
  Serial.println(Light);

  Serial.print("Luz promedio:   ");
  Serial.println(aveL.mean());
  aveL.push(Light);
}
AsyncTask TaskLight(700, true, readLight);


void writeTextMonitoreo() {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.setCursor(2, 0);
  lcd.print(TempC);

  lcd.setCursor(7, 0);
  lcd.print("L:");
  lcd.setCursor(9, 0);
  lcd.print(aveL.mean());

  lcd.setCursor(0,1);
  lcd.print("  Hum: ");
  lcd.setCursor(7,1);
  lcd.print(Humedad);
}
AsyncTask TaskTextMonitoreo(100, true, writeTextMonitoreo);


void writeTextEventos() {
  //lcd.clear();
  read_sensores();

  lcd.setCursor(0, 1);
  lcd.print("I: ");
  lcd.setCursor(2, 1);
  if (pinIFRon == LOW){
    lcd.print("ON ");
  }else{
    lcd.print("OFF");
  }

  lcd.setCursor(7, 1);
  lcd.print("H:");
  lcd.setCursor(9, 1);
  if (pinHallon == HIGH){
    lcd.print("ON ");
  }else{
    lcd.print("OFF");
  }
}
AsyncTask TaskTextEventos(100, true, writeTextEventos);

/**
*@brief funcion que se utiliza para leer sensor HALL e INFRAROJO en estado eventos
*/
void read_sensores(void)
{
  pinIFRon = digitalRead(pinIFR);
  pinHallon = digitalRead(pinHall);

  if((pinIFRon == LOW) && (pinHallon == HIGH))
  {
    input = Input::Sign_P;
    TaskTiempo.Stop();

  }
  //para testing
  // if(true)
  // {
  //   input = Input::Sign_P;
  //   TaskTiempo.Stop();
  // }

  Serial.print("IR: ");
  Serial.println(pinIFRon);
  Serial.print("HALL: ");
  Serial.println(pinHallon);
}

// Create new StateMachine
StateMachine stateMachine(6, 10);



// Setup the State Machine
void setupStateMachine() {
  // Add transitions
  stateMachine.AddTransition(INICIO, BLOQUEADO, []() {
    return input == Sign_B;
  });

  stateMachine.AddTransition(INICIO, MONITOREO, []() {
    return input == Sign_P;
  });
  stateMachine.AddTransition(MONITOREO, EVENTOS, []() {
    return input == Sign_T;
  });
  stateMachine.AddTransition(EVENTOS, MONITOREO, []() {
    return input == Sign_T;
  });

  stateMachine.AddTransition(MONITOREO, ALARMA, []() {
    return input == Sign_P;
  });
  stateMachine.AddTransition(ALARMA, INICIO, []() {
    return input == Sign_K;
  });
  stateMachine.AddTransition(EVENTOS, ALERTA, []() {
    return input == Sign_P;
  });

  stateMachine.AddTransition(ALERTA, EVENTOS, []() {
    return input == Sign_T;
  });
  stateMachine.AddTransition(ALERTA, ALARMA, []() {
    return input == Sign_B;
  });
  stateMachine.AddTransition(BLOQUEADO, INICIO, []() {
    return input == Sign_T;
  });

  // Add actions
  stateMachine.SetOnEntering(INICIO, funct_Init_Inicio);
  stateMachine.SetOnEntering(MONITOREO, funct_Init_Monitoreo);
  stateMachine.SetOnEntering(BLOQUEADO, funct_Init_Bloqueado);
  stateMachine.SetOnEntering(EVENTOS, funct_Init_Eventos);
  stateMachine.SetOnEntering(ALERTA, funct_Init_Alerta);
  stateMachine.SetOnEntering(ALARMA, funct_Init_Alarma);

  stateMachine.SetOnLeaving(INICIO, funct_Fin_Inicio);
  stateMachine.SetOnLeaving(MONITOREO, funct_Fin_Monitoreo);
  stateMachine.SetOnLeaving(BLOQUEADO, funct_Fin_Bloqueado);
  stateMachine.SetOnLeaving(EVENTOS, funct_Fin_Eventos);
  stateMachine.SetOnLeaving(ALERTA, funct_Fin_Alerta);
  stateMachine.SetOnLeaving(ALARMA, funct_Fin_Alarma);
}

void funct_Init_Inicio(void) {
  TareaDigiteClave.Start();
  TareaManejarTeclado.Start();
  TareaManejarSistema.Start();
  TareaManejarContrasenia.Start();
  TaskKeypad.Start();
}

void funct_Fin_Inicio(void) {
  TaskTiempoLimite.Stop();
  TareaDigiteClave.Stop();
  TareaActivarAlarma.Stop();
  TareaDetenemosAlarma.Stop();
  TareaManejarTeclado.Stop();
  TareaManejarSistema.Stop();
  TareaLimpiarPantalla.Stop();
  TareaManejarContrasenia.Stop();
}

void funct_Init_Monitoreo(void) {
  Serial.println("ESTADO MONITOREO");
    TaskTemp.Start();
  TaskLight.Start();
  TaskTextMonitoreo.Start();
  //TaskTextEventos.Start();
  TaskTiempo.SetIntervalMillis(5000);
  TaskTiempo.Start();
}

void funct_Fin_Monitoreo(void) {
  TaskTemp.Stop();
  TaskLight.Stop();
  TaskTextMonitoreo.Stop();
  TaskTiempo.Stop();
}

void funct_Init_Bloqueado(void) {
    TareaPrenderLuz.SetIntervalMillis(500);
    TareaApagarLuz.SetIntervalMillis(500);
    TareaPrenderLuz.Start();
    TareaActivarAlarma.Start();
    TareaDetenemosAlarma.Start();
    TaskTiempo.SetIntervalMillis(7000);
    TaskTiempo.Start();
}
void funct_Fin_Bloqueado(void) {
    TareaLimpiarPantalla.Start();
    TareaPrenderLuz.Stop();
    TareaApagarLuz.Stop();
    digitalWrite(LED_ROJO,LOW);
}

void funct_Init_Eventos(void) {
  lcd.setCursor(0,0);
  lcd.print("ESTADO EVENTOS");
  lcd.clear();
  TaskTextEventos.Start();

  TaskTiempo.SetIntervalMillis(3000);
  TaskTiempo.Start();
}
void funct_Fin_Eventos(void) {
  TaskTextEventos.Stop();
  TaskTiempo.Stop();
}

void funct_Init_Alerta(void) {
  lcd.clear();
  lcd.print("ESTADO ALERTA");
  TareaPrenderLuzAlerta.Start();

  //tone(ZUMBADOR, 2500);
  contadorAlerta++;

  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(ZUMBADOR, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    //delay(noteDuration);

    // stop the waveform generation before the next note.
    
    
  }
  
    TaskTiempo.Start();
}

void funct_Fin_Alerta(void) {
  noTone(ZUMBADOR);
  TaskTiempo.Stop();
  TareaPrenderLuzAlerta.Stop();
  TareaApagarLuzAlerta.Stop();
  digitalWrite(LED_AZUL,LOW);
}

void funct_Init_Alarma(void) {
  TareaPrenderLuz.SetIntervalMillis(150);
  TareaApagarLuz.SetIntervalMillis(150);
  TareaPrenderLuz.Start();
  Serial.println("ESTADO ALARMA");
  contadorAlerta = 0;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("ESTADO ALARMA ");
  tone(ZUMBADOR, 2500);
}
void funct_Fin_Alarma(void) {
  lcd.clear();
  noTone(ZUMBADOR);
  TaskTiempo.Stop();
  TareaPrenderLuz.Stop();
  TareaApagarLuz.Stop();
  digitalWrite(LED_ROJO,LOW);
}


void setup() {
  Serial.begin(9600);

  Serial.println("Iniciando Maquina de Estados...");
  setupStateMachine();
  Serial.println("Maquina de Estados Iniciada");
  dht.begin();

  // Initial state
  stateMachine.SetState(INICIO, false, true);
  pinMode(ZUMBADOR, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  lcd.begin(16, 2);
  TareaDigiteClave.Start();
  TareaManejarTeclado.Start();
  TareaManejarSistema.Start();
  TareaManejarContrasenia.Start();
  TaskKeypad.Start();
}



void loop() {
  TareaDigiteClave.Update();
  TaskTiempoLimite.Update();
  TareaActivarAlarma.Update();
  TareaDetenemosAlarma.Update();
  TareaManejarTeclado.Update();
  TareaManejarSistema.Update();
  TareaLimpiarPantalla.Update();
  TareaManejarContrasenia.Update();
  TaskTemp.Update();
  TaskLight.Update();
  TaskTextMonitoreo.Update();
  TaskTextEventos.Update();
  TaskKeypad.Update();
  TaskTiempo.Update();
  TareaPrenderLuz.Update(TareaApagarLuz);
	TareaApagarLuz.Update(TareaPrenderLuz);
  TareaPrenderLuzAlerta.Update(TareaApagarLuzAlerta);
  TareaApagarLuzAlerta.Update(TareaPrenderLuzAlerta);

  // Update State Machine
  stateMachine.Update();
  input = Input::Unknow;
}
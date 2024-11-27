/**
* @file FinalMaquina.ino
* @brief Sistema de seguridad con máquina de estados
* @author Brayan Steven Gomes Lasso <brayangomes@unicauca.edu.co>
* @author Juan David Perdomo Ramos <juanperdomobp@unicauca.edu.co>
* @author Juan David Vela Coronado <juanvela@unicauca.edu.co>
* @date 27 de noviembre de 2024
*
* Este sistema implementa un sistema de seguridad con las siguientes características:
* - Autenticación por teclado
* - Monitoreo de temperatura, luz y sensores de proximidad
* - Máquina de estados para control de seguridad
* - Alertas visuales y sonoras
*/

#include "AsyncTaskLib.h"
#include "Average.h"
#include "DHT.h"
#include "StateMachineLib.h"

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <string.h>

//---------------------------------------------------------
/**
* @defgroup Notas_Musicales Definiciones de Frecuencias Musicales
* @brief Definiciones de frecuencias para notas musicales
*/
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

/** @}  */ // Fin del grupo Notas_Musicales

Average<float> aveT(10); ///< Objeto para calcular promedio de temperatura
Average<float> aveL(10); ///< Objeto para calcular promedio de luz

#define DEBUG(a) \
  Serial.print(millis()); \
  Serial.print(": "); \
  Serial.println(a);

/**
* @defgroup Configuracion_Pines Configuracion de Pines de Hardware
* @brief Definicion de pines para componentes del sistema
*/
#define LED_VERDE 9 ///< Pin para LED verde
#define LED_ROJO 10 ///< Pin para LED rojo
#define LED_AZUL 8  ///< Pin para LED azul
#define ZUMBADOR 7  ///< Pin para zumbador/buzzer

#define DHTPIN 40   ///< Pin para sensor DHT
#define beta 4090   ///< Valor beta para calculo de temperatura
#define pinLight A0 ///< Pin analogico para sensor de luz

#define pinIFR 13   ///< Pin para sensor infrarrojo
#define pinHall 6   ///< Pin para sensor Hall

#define DHTTYPE DHT11 ///< Tipo de sensor DHT
#define TH_TEMP_HIGH 27.3 ///< Temperatura maxima para alertar al usuario
/** @}  */  // Fin del grupo Configuracion_Pines

// Configuracion del LCD 
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variables globales para lecturas de sensores
double TempC;       ///< Temperatura en grados centigrados
double TempF;       ///< Temperatura en grados farenheit
double Light;       ///< Luz medida con el fotoresistor
double Humedad;     ///< Porcentaje de humedad

// Estados de sensores
unsigned char pinIFRon = LOW; ///< Sensor Infrarrojo inicializado en LOW
unsigned char pinHallon = LOW; ///< Sensor Hall inicializado en LOW

// Configuracion de teclado matricial
const byte FILAS = 4; ///< Numero de filas del teclado
const byte COLUMNAS = 4;  ///< Numero de columnas del teclado
char teclas[FILAS][COLUMNAS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte filaPines[FILAS] = { 24, 26, 28, 30 }; ///< Pines para filas del teclado
byte columnaPines[COLUMNAS] = { 32, 34, 36, 38 }; ///< Pines para columnas del teclado
Keypad keypad = Keypad(makeKeymap(teclas), filaPines, columnaPines, FILAS, COLUMNAS); //inicializar el keypad

// Inicializacion de sensores
DHT dht(DHTPIN, DHTTYPE);

// Variables de control del sistema
unsigned char bandera = 0;  ///< Bandera de estado general
unsigned char indice = 0;   ///< Indice para entrada de contraseña
unsigned char intentos = 0; ///< Contador de intentos de contraseña
unsigned char contadorAlerta = 0; ///< Contador para iniciar alarma de deteccion de eventos

// Configuracion de constraseña
char clave[5];              ///< Almacena la clave ingresada
char contrasenia[5] = "12345";  ///< Contraseña predeterminada

//--------------------------------------------------------------
// change this to make the song slower or faster
int tempo = 105;  ///< Tempo de la melodia

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
int notes = sizeof(melody) / sizeof(melody[0]) / 2; ///< Número de notas en la melodía

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;  ///< Duración de una nota completa en milisegundos

int divider = 0, noteDuration = 0;  ///< Variables para la duración de las notas
//----------------------------------------------------------------------------------

/**
* @enum State
* @brief Estados posibles de la maquina
*/
enum State {
  INICIO = 0,     ///< Estado inicial, contraseña para ingresar
  MONITOREO = 1,  ///< Estado de monitoreo ambiental, temperatura y humedad
  BLOQUEADO = 2,  ///< Estado de sistema bloqueado, contraseña fallida o tiempo de respuesta agotado
  EVENTOS = 3,    ///< Estado de deteccion de eventos, infrarrojo y hall
  ALERTA = 4,     ///< Estado de alerta, infrarrojo y hall activados o temperatura maxima superada
  ALARMA = 5      ///< Estado de alarma, cuando el sistema esta bloqueado
};

/**
* @enum Input
* @brief Señales de entrada para cambios de estado
*/
enum Input {
  Sign_T = 0,     ///< Señal de tiempo
  Sign_P = 1,     ///< Señal de proceso
  Sign_B = 2,     ///< Señal de bloqueo
  Sign_K = 3,     ///< Señal de teclado
  Unknow = 4      ///< Señal desconocida
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

/**
* @brief Función de timeout para manejar el tiempo de alerta
*/
void timeout(void) {
  Serial.println("Contador Alerta Timeout = ");
  Serial.println(contadorAlerta);
  if (contadorAlerta > 2){
    input = Input::Sign_B;
    return;
  }
  input = Input::Sign_T;

}
/** @brief Tarea para manejar el timeout de alerta */
AsyncTask TaskTiempo(3000, false, timeout);

/**
* @brief Función de timeout para manejar el tiempo de espera de contraseña
*/
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
/** @brief Tarea para manejar el timeout de la contraseña */
AsyncTask TaskTiempoLimite(5000,false,timeoutPassword);

/**
* @brief Función para leer el teclado matricial
*/
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
/** @brief Tarea para manejar la lectura del teclado */
AsyncTask TaskKeypad(100, true, read_keypad);

/**
* @brief Función para mostrar el mensaje de ingreso de clave en el LCD
*/
void mostrarDigite() {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite la clave:");
}
/** @brief Tarea para mostrar el mensaje de ingreso de clave */
AsyncTask TareaDigiteClave(1, false, mostrarDigite);

/**
* @brief Función para limpiar la pantalla del LCD
*/
void limpiarPantalla() {
  lcd.clear();
  TareaDigiteClave.Start();
}
/** @brief Tarea para limpiar la pantalla del LCD */
AsyncTask TareaLimpiarPantalla(2000, false, limpiarPantalla);

/**
* @brief Función para activar la alarma
*/
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
/** @brief Tarea para activar la alarma */
AsyncTask TareaActivarAlarma(0, false, activarAlarma);

/**
* @brief Función para detener la alarma
*/
void detenemosAlarma() {
  noTone(ZUMBADOR);
  bandera = 1;
}
/** @brief Tarea para detener la alarma */
AsyncTask TareaDetenemosAlarma(3000, false, detenemosAlarma);

/**
* @brief Función para manejar el sistema de seguridad
*/
void manejarTeclado() {
  char tecla = keypad.getKey();

  if (tecla) {
    TaskTiempoLimite.Start();
    lcd.setCursor(indice, 1);
    lcd.print("*");
    clave[indice++] = tecla;
  }
}
/** @brief Tarea para manejar el teclado */
AsyncTask TareaManejarTeclado(0, true, manejarTeclado);

/**
* @brief Función para manejar el estado del sistema
*/
void manejarSistema() {
  if (intentos > 2) {
    lcd.begin(16, 2);
    lcd.print("SISTEMA BLOQUEADO");
    intentos = 0;
    input = Input::Sign_B;
  }
}
/** @brief Tarea para manejar el estado del sistema */
AsyncTask TareaManejarSistema(0, true, manejarSistema);

/**
* @brief Función para manejar la contraseña ingresada
*/
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
/** @brief Tarea para manejar la contraseña ingresada */
AsyncTask TareaManejarContrasenia(0, true, manejarContrasenia);

/**
* @brief Función para leer la temperatura y humedad del sensor DHT
*/
void readTemperature() {
  Humedad = dht.readHumidity();
  // Lectura de la temperatura en grados Centigrados
  TempC = dht.readTemperature();

  if (TempC >= TH_TEMP_HIGH){
    input=Input::Sign_P;
  }
  // Verifica si alguna lectura falló y retorna (para volver a intentarlo).
  if (isnan(Humedad) || isnan(TempC)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Calcular el índice de calor en grados Centigrados (isFahreheit = false)
  float hic = dht.computeHeatIndex(TempC, Humedad, false);

  Serial.print(F("Humidity: "));
  Serial.print(Humedad);
  Serial.print(F("%  Temperature: "));
  Serial.print(TempC);
  Serial.print(F("°C "));
}
/** @brief Tarea para leer la temperatura */
AsyncTask TaskTemp(1200, true, readTemperature);

/**
* @brief Función para leer la luz del sensor
*/
void readLight() {
  Light = analogRead(pinLight);
  Serial.print("Luz: ");
  Serial.println(Light);

  Serial.print("Luz promedio:   ");
  Serial.println(aveL.mean());
  aveL.push(Light);
}
/** @brief Tarea para leer la luz */
AsyncTask TaskLight(700, true, readLight);

/**
* @brief Función para mostrar los datos de monitoreo en el LCD
*/
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
/** @brief Tarea para mostrar los datos de monitoreo */
AsyncTask TaskTextMonitoreo(100, true, writeTextMonitoreo);

/**
* @brief Función para mostrar los datos de eventos en el LCD
*/
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
/** @brief Tarea para mostrar los datos de eventos */
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

  Serial.print("IR: ");
  Serial.println(pinIFRon);
  Serial.print("HALL: ");
  Serial.println(pinHallon);
}

// Create new StateMachine
StateMachine stateMachine(6, 10);

/**
* @brief Función para configurar la máquina de estados
*/
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

/**
* @brief Función que se ejecuta al entrar en el estado INICIO
*/
void funct_Init_Inicio(void) {
  TareaDigiteClave.Start();
  TareaManejarTeclado.Start();
  TareaManejarSistema.Start();
  TareaManejarContrasenia.Start();
  TaskKeypad.Start();
}

/**
* @brief Función que se ejecuta al salir del estado INICIO
*/
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

/**
* @brief Función que se ejecuta al entrar en el estado MONITOREO
*/
void funct_Init_Monitoreo(void) {
  Serial.println("ESTADO MONITOREO");
    TaskTemp.Start();
  TaskLight.Start();
  TaskTextMonitoreo.Start();
  //TaskTextEventos.Start();
  TaskTiempo.SetIntervalMillis(5000);
  TaskTiempo.Start();
}

/**
* @brief Función que se ejecuta al salir del estado MONITOREO
*/
void funct_Fin_Monitoreo(void) {
  TaskTemp.Stop();
  TaskLight.Stop();
  TaskTextMonitoreo.Stop();
  TaskTiempo.Stop();
}

/**
* @brief Función que se ejecuta al entrar en el estado BLOQUEADO
*/
void funct_Init_Bloqueado(void) {
    TareaPrenderLuz.SetIntervalMillis(500);
    TareaApagarLuz.SetIntervalMillis(500);
    TareaPrenderLuz.Start();
    TareaActivarAlarma.Start();
    TareaDetenemosAlarma.Start();
    TaskTiempo.SetIntervalMillis(7000);
    TaskTiempo.Start();
}

/**
* @brief Función que se ejecuta al salir del estado BLOQUEADO
*/
void funct_Fin_Bloqueado(void) {
    TareaLimpiarPantalla.Start();
    TareaPrenderLuz.Stop();
    TareaApagarLuz.Stop();
    digitalWrite(LED_ROJO,LOW);
}

/**
* @brief Función que se ejecuta al entrar en el estado EVENTOS
*/
void funct_Init_Eventos(void) {
  lcd.setCursor(0,0);
  lcd.print("ESTADO EVENTOS");
  lcd.clear();
  TaskTextEventos.Start();

  TaskTiempo.SetIntervalMillis(3000);
  TaskTiempo.Start();
}

/**
* @brief Función que se ejecuta al salir del estado EVENTOS
*/
void funct_Fin_Eventos(void) {
  TaskTextEventos.Stop();
  TaskTiempo.Stop();
}

/**
* @brief Función que se ejecuta al entrar en el estado ALERTA
*/
void funct_Init_Alerta(void) {
  lcd.clear();
  lcd.print("ESTADO ALERTA");
  TareaPrenderLuzAlerta.Start();

  //tone(ZUMBADOR, 2500);
  contadorAlerta++;

  // Iterar sobre las notas de la melodia
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

/**
* @brief Función que se ejecuta al salir del estado ALERTA
*/
void funct_Fin_Alerta(void) {
  noTone(ZUMBADOR);
  TaskTiempo.Stop();
  TareaPrenderLuzAlerta.Stop();
  TareaApagarLuzAlerta.Stop();
  digitalWrite(LED_AZUL,LOW);
}

/**
* @brief Función que se ejecuta al entrar en el estado ALARMA
*/
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

/**
* @brief Función que se ejecuta al salir del estado ALARMA
*/
void funct_Fin_Alarma(void) {
  lcd.clear();
  noTone(ZUMBADOR);
  TaskTiempo.Stop();
  TareaPrenderLuz.Stop();
  TareaApagarLuz.Stop();
  digitalWrite(LED_ROJO,LOW);
}

/**
* @brief Función de configuración inicial del sistema
*/
void setup() {
  Serial.begin(9600);

  Serial.println("Iniciando Maquina de Estados...");
  setupStateMachine();
  Serial.println("Maquina de Estados Iniciada");
  dht.begin();

  // Estado inicial
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

/**
* @brief Bucle principal del sistema
*/
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

  // Actualizar la máquina de estados
  stateMachine.Update();
  input = Input::Unknow;
}
#include <ConexionWifiServidor.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include "FirebaseESP32.h"
#include <time.h>

// Pines GPIO para ESP32 SERVIDOR
#define TRIG_PIN_P1 26
#define ECHO_PIN_P1 25
#define TRIG_PIN_P2 14
#define ECHO_PIN_P2 12
#define FOTOR_PIN 32
#define LED_DISPONIBLE_PIN 4
#define LED_OCUPADO_PIN 0
#define PIN_SERVO_AP 15
#define LED_P1VERDE 2
#define LED_P1ROJO 5
#define LED_P2VERDE 18
#define LED_P2ROJO 19
#define SENSOR_APROX 33

// Declarar funciones
void inicializarSensores();
void medirDistancias();
void controlarLuces();
void controlarRespuesta();
void conexionFirebase();
void obtenerFechaHoraActual();
// Variables globales
Servo servoAbrirPuerta;
int CANT_ESPACIO;
float porcent_sensor;
int rafaga_luz = 0;
float duration_us_P1, duration_us_P2;
float distance_cm_P1, distance_cm_P2;
int obstaculo = 0;
// Declarar una variable para medir el tiempo
unsigned long tiempoUltimaRafaga = 0;
bool ban_puerta = false;
float value, pos;

#define FIREBASE_HOST "esp32-automation-9fd3b-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "2ytniBhap3RId6zjYmjmityIC7WfJ7QmUMi1YgeW"
// FIrebase Data
FirebaseData firebaseData;
// para el nodo
bool iterar = true;
String ruta = "Parqueo";

void setup()
{
  ConectarWifi();
  delay(500);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  inicializarSensores();
  servoAbrirPuerta.attach(PIN_SERVO_AP);
  servoAbrirPuerta.write(0);
  LCD_MENSAJE("Conexion Exitosa");
  delay(200);
  LCD_MENSAJE("Esperando...");
}

////// SERVIDOR  ///////
void loop()
{
  int analogValue = analogRead(FOTOR_PIN);
  porcent_sensor = map(analogValue, 0, 4980, 0, 100);
  if (porcent_sensor >= 5 && porcent_sensor <= 20)
  {
    rafaga_luz += 1;
    tiempoUltimaRafaga = millis();
  }
  // Reiniciar el contador si han pasado más de 5 segundos
  if ((millis() - tiempoUltimaRafaga) > 5000)
  {
    rafaga_luz = 0;
  }
  // Verificar si se han realizado las tres rafagas en menos de 2 segundos
  if (rafaga_luz >= 3)
  {
    if (CANT_ESPACIO < 4)
    {
      servoAbrirPuerta.write(95);
    }
    rafaga_luz = 0;
  }
  obstaculo = digitalRead(SENSOR_APROX);
  if (obstaculo == 0) // Verifica si no hay obstáculo
  {
    ban_puerta = true;
  }
  if (ban_puerta && obstaculo)
  {
    delay(1000);
    servoAbrirPuerta.write(0); // Cierra la puerta
    ban_puerta = false;
  }

  medirDistancias();
  LCD_MENSAJE("CANT ESPACIOS:");
  LCD_MENSAJE(" ", CANT_ESPACIO, 0);
  delay(500); // Pausa de 0.5 segundos
  controlarRespuesta();
  controlarLuces();
  conexionFirebase();
}

void inicializarSensores()
{
  pinMode(TRIG_PIN_P1, OUTPUT);
  pinMode(ECHO_PIN_P1, INPUT);
  pinMode(TRIG_PIN_P2, OUTPUT);
  pinMode(ECHO_PIN_P2, INPUT);
  pinMode(LED_DISPONIBLE_PIN, OUTPUT);
  pinMode(LED_OCUPADO_PIN, OUTPUT);
  pinMode(LED_P1VERDE, OUTPUT);
  pinMode(LED_P1ROJO, OUTPUT);
  pinMode(LED_P2VERDE, OUTPUT);
  pinMode(LED_P2ROJO, OUTPUT);
  pinMode(FOTOR_PIN, INPUT);
  pinMode(SENSOR_APROX, INPUT);
  digitalWrite(TRIG_PIN_P1, LOW);
  digitalWrite(TRIG_PIN_P2, LOW);
}

void medirDistancias()
{

  digitalWrite(TRIG_PIN_P1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN_P1, LOW);
  duration_us_P1 = pulseIn(ECHO_PIN_P1, HIGH);
  distance_cm_P1 = 0.0010 * duration_us_P1;
  delay(50);

  digitalWrite(TRIG_PIN_P2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN_P2, LOW);
  duration_us_P2 = pulseIn(ECHO_PIN_P2, HIGH);
  distance_cm_P2 = 0.0010 * duration_us_P2;

  int cont_1 = (distance_cm_P1 <= 0.40) ? (digitalWrite(LED_P1ROJO, HIGH), digitalWrite(LED_P1VERDE, 0), 1) : (digitalWrite(LED_P1VERDE, HIGH), digitalWrite(LED_P1ROJO, 0), 0);
  int cont_2 = (distance_cm_P2 <= 0.40) ? (digitalWrite(LED_P2ROJO, HIGH), digitalWrite(LED_P2VERDE, 0), 1) : (digitalWrite(LED_P2VERDE, HIGH), digitalWrite(LED_P2ROJO, 0), 0);
  int cont_3 = (value <= 0.40) ? (1) : (0);
  int cont_4 = (pos <= 0.40) ? (1) : (0);
  CANT_ESPACIO = cont_1 + cont_2 + cont_3 + cont_4;
}

void controlarRespuesta()
{
  WiFiClient client = server.available();
  if (client)
  {
    String command = "";
    String valueStr = "";
    String posStr = "";

    while (client.connected() && !client.available())
    {
      // Esperar a que haya datos disponibles
    }
    if (client.available())
    {
      command = client.readStringUntil(':');
      valueStr = client.readStringUntil(':');
      posStr = client.readStringUntil('\n');
    }
    if (!command.isEmpty() && !valueStr.isEmpty() && !posStr.isEmpty())
    {
      if (command == "CANT_ESPACIOS")
      {
        value = valueStr.toFloat();
        pos = posStr.toFloat();
      }
    }
  }
}

void controlarLuces()
{
  // Lógica de control de luces semaforo
  if (CANT_ESPACIO < 4)
  {
    digitalWrite(LED_DISPONIBLE_PIN, HIGH); // Enciende el LED de espacio disponible
    digitalWrite(LED_OCUPADO_PIN, LOW);     // Apaga el LED de espacio ocupado
  }
  else
  {
    digitalWrite(LED_OCUPADO_PIN, HIGH);   // Enciende el LED de espacio ocupado
    digitalWrite(LED_DISPONIBLE_PIN, LOW); // Apaga el LED de espacio disponible
  }
}

void obtenerFechaHoraActual(int &anio, int &mes, int &dia, int &hora, int &minuto, int &segundo)
{
  static unsigned long tiempoSimulado = 0;         // Variable para simular el tiempo
  const unsigned long tiempoEntreRegistros = 2000; // 2 segundos entre cada registro
  const unsigned long tiempoSimuladoPorDia = 5000; // 5 segundos simulados equivalen a un día
  const unsigned int diasSimuladosTotal = 5;       // Simular 5 días

  if (tiempoSimulado < diasSimuladosTotal * tiempoSimuladoPorDia)
  {
    tiempoSimulado += tiempoEntreRegistros;

    // Calcular días simulados
    int diasSimulados = tiempoSimulado / tiempoSimuladoPorDia;

    // Obtener el tiempo simulado en segundos
    unsigned long tiempoSimuladoSegundos = tiempoSimulado % tiempoSimuladoPorDia;

    struct tm timeinfo;
    timeinfo.tm_year = 100 + diasSimulados; // Asumiendo que cada día suma un año
    timeinfo.tm_mon = 0;                    // Enero
    timeinfo.tm_mday = 1 + diasSimulados;   // Asumiendo que cada día suma un día
    timeinfo.tm_hour = tiempoSimuladoSegundos / 3600;
    timeinfo.tm_min = (tiempoSimuladoSegundos % 3600) / 60;
    timeinfo.tm_sec = tiempoSimuladoSegundos % 60;

    anio = timeinfo.tm_year;
    mes = timeinfo.tm_mon + 1;
    dia = timeinfo.tm_mday;
    hora = timeinfo.tm_hour;
    minuto = timeinfo.tm_min;
    segundo = timeinfo.tm_sec;
  }
  else
  {
    // Reiniciar la simulación después de 5 días (ajustar según sea necesario)
    tiempoSimulado = 0;
  }
}

void conexionFirebase()
{
  // Obtener fecha y hora actual
  int anio, mes, dia, hora, minuto, segundo;
  obtenerFechaHoraActual(anio, mes, dia, hora, minuto, segundo);

  // Crear un nodo único para cada día
  String nodoDia = "Parqueo_" + String(anio) + "-" + String(mes) + "-" + String(dia);

  // Crear la ruta completa
  String rutaCompleta = ruta + "/" + nodoDia;

  // Escribir la cantidad de vehículos en el nodo específico del día
  Firebase.setInt(firebaseData, rutaCompleta + "/CantidadVehiculos", CANT_ESPACIO);

  // Obtener el nodo para el día actual
  String nodoDiaActual = ruta + "/" + nodoDia;

  // Obtener el número total de coches salidos hoy
  int cochesSalidosHoy = Firebase.getInt(firebaseData, nodoDiaActual + "/CochesSalidos");

  // Incrementar el contador de coches salidos
  cochesSalidosHoy += CANT_ESPACIO;

  // Escribir el nuevo valor en Firebase
  Firebase.setInt(firebaseData, nodoDiaActual + "/CochesSalidos", cochesSalidosHoy);
}

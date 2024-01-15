#include <ConexionWifiCliente.h>
#include <IRremote.h>
#include <Arduino.h>
#include <ESP32Servo.h>

// Pines GPIO para ESP32 CLIENTE
#define TRIG_PIN_P1 26
#define ECHO_PIN_P1 25
#define TRIG_PIN_P2 27
#define ECHO_PIN_P2 14
#define PIN_SERVO_CP 4
#define LED_P1VERDE 17
#define LED_P1ROJO 5
#define LED_P2VERDE 18
#define LED_P2ROJO 19
#define RECV_PIN 15

// Declarar funciones
void inicializarSensores();
void medirDistancias();
void controlarLuces();
void enviarDatosAlServidor();

// Variables globales
Servo servoCerrarPuerta;
int CANT_ESPACIO;
float porcent_sensor;
int rafaga_luz = 0;
float duration_us_P1, duration_us_P2;
float distance_cm_P1, distance_cm_P2;
int obstaculo = 0;
// Declarar una variable para medir el tiempo
unsigned long tiempoUltimaRafaga = 0;
bool ban_puerta = false;

#define Boton_mas 0xB946FF00
#define Boton_menos 0xEA15FF00

Ticker enviarDatosTicker;

void setup()
{
    ConectarWifi();
    delay(500);
    inicializarSensores();
    servoCerrarPuerta.attach(PIN_SERVO_CP);
    servoCerrarPuerta.write(25);
    IrReceiver.begin(RECV_PIN, DISABLE_LED_FEEDBACK); // inicializa recepcion de datos
    enviarDatosTicker.attach(0.1, enviarDatosAlServidor);
}

// CLIENTE
void loop()
{
    if (IrReceiver.decode())
    {
        if (IrReceiver.decodedIRData.decodedRawData == Boton_mas)
        {
            servoCerrarPuerta.write(100);
        }
        else if (IrReceiver.decodedIRData.decodedRawData == Boton_menos)
        {
            servoCerrarPuerta.write(25);
        }

        IrReceiver.resume(); // Reiniciar el receptor para el próximo código IR
    }
    medirDistancias();
    delay(10);
}

void inicializarSensores()
{
    pinMode(TRIG_PIN_P1, OUTPUT);
    pinMode(ECHO_PIN_P1, INPUT);
    pinMode(TRIG_PIN_P2, OUTPUT);
    pinMode(ECHO_PIN_P2, INPUT);
    pinMode(LED_P1VERDE, OUTPUT);
    pinMode(LED_P1ROJO, OUTPUT);
    pinMode(LED_P2VERDE, OUTPUT);
    pinMode(LED_P2ROJO, OUTPUT);
    digitalWrite(TRIG_PIN_P1, LOW);
    digitalWrite(TRIG_PIN_P2, LOW);
}

void enviarDatosAlServidor()
{
    WiFiClient client;

    if (client.connect("ESP_SERVER.local", serverPort))
    {
        sendNumericCommand(client, "CANT_ESPACIOS", distance_cm_P1, distance_cm_P2);
        client.stop();
    }
}

void medirDistancias()
{
    digitalWrite(TRIG_PIN_P1, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_P1, LOW);
    duration_us_P1 = pulseIn(ECHO_PIN_P1, HIGH);
    distance_cm_P1 = 0.0010 * duration_us_P1;
    delay(25);
    digitalWrite(TRIG_PIN_P2, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN_P2, LOW);
    duration_us_P2 = pulseIn(ECHO_PIN_P2, HIGH);
    distance_cm_P2 = 0.0010 * duration_us_P2;

    int cont_1 = (distance_cm_P1 <= 0.40) ? (digitalWrite(LED_P1ROJO, HIGH), digitalWrite(LED_P1VERDE, 0), 1) : (digitalWrite(LED_P1VERDE, HIGH), digitalWrite(LED_P1ROJO, 0), 0);
    int cont_2 = (distance_cm_P2 <= 0.40) ? (digitalWrite(LED_P2ROJO, HIGH), digitalWrite(LED_P2VERDE, 0), 1) : (digitalWrite(LED_P2VERDE, HIGH), digitalWrite(LED_P2ROJO, 0), 0);

    CANT_ESPACIO = cont_1 + cont_2;
}

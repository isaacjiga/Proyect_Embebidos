#include <WiFi.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include "ConexionLcd.h"

#define ledWifi 2
Ticker ticker; // Objeto creado tipo ticker

// Propiedades PWM
int frecuencia = 5000; // Frecuencia estándar
int canal = 1;         // Canal PWM
int resolucion = 8;    // Resolución de 8 bits de 0-255
int estado;

void parpadear()
{
  estado = !estado;
  if (estado == 1)
  {
    ledcWrite(canal, 15);
  }
  else
  {
    ledcWrite(canal, 0);
  }
}

const int serverPort = 82; // Puerto en el que el servidor está escuchando

void sendNumericCommand(WiFiClient &client, const char *command, float posicion1, float posicion2)
{
  client.print(command);
  client.print(":");
  client.println(posicion1);
  client.print(":");
  client.println(posicion2);
  delay(1000);
}

void ConectarWifi()
{
  // Configuramos la funcionalidad del PWM
  ledcSetup(canal, frecuencia, resolucion);
  ledcAttachPin(ledWifi, canal);

  WiFiManager wm;
  ticker.attach(0.4, parpadear); // Inicia la ejecucion cada 0.4ms se llama la funcion en un bucle

  // wm.resetSettings();                        // Reseteamos la configuracion
  bool res = wm.autoConnect("Esp32_client"); // Creamos Ap y portal cautivo

  if (!res)
  {
    ESP.restart();
  }
  else
  {
    Serial.println("Ya estas conectado :)");
    ticker.detach(); // detenemos la ejecucion
  }
  // Detenemos el parpadeo del LED
  ticker.detach();
  ledcWrite(canal, 0);
  LCD_CONFIG();
}

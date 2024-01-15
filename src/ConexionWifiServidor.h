#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include <Ticker.h>
#include "ConexionLcd.h"

#define ledWifi 2
Ticker ticker; // Objeto creado tipo ticker
// Propiedades PWM
int frecuencia = 5000;  // Frecuencia estándar
uint8_t canal = 1;      // Canal PWM
uint8_t resolucion = 8; // Resolución de 8 bits de 0-255
bool estado;

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

WiFiServer server(82);
void startServer()
{
  if (!MDNS.begin("ESP_SERVER"))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error mDNS!");
    while (1)
    {
      delay(1000);
    }
  }
  // Inicializar el servidor
  server.begin();
}

void ConectarWifi()
{

  // Configuramos la funcionalidad del PWM
  ledcSetup(canal, frecuencia, resolucion);
  ledcAttachPin(ledWifi, canal);
  // Configuramos la funcionalidad del PWM

  WiFiManager wm;
  ticker.attach(0.5, parpadear); // Inicia la ejecución cada 0.5ms, llamando la función en un bucle

  LCD_CONFIG();
  // wm.resetSettings();                       // Reseteamos la configuración
  bool res = wm.autoConnect("Esp32_serve"); // Creamos Ap y portal cautivo

  // Detenemos el parpadeo del LED
  ticker.detach();
  ledcWrite(1, 0);

  if (!res)
  {
    return;
  }

  String redWifi = WiFi.SSID();         // Nombre de la red wifi
  IPAddress ipaddress = WiFi.localIP(); // Ip correspondiente
  LCD_MENSAJE(redWifi, ipaddress);
  // Iniciar el servidor después de la conexión WiFi
  startServer();
}

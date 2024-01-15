#include <LiquidCrystal_I2C.h>

#define lcdColumns 16
#define lcdRows 2
#define SDA 21
#define SCL 22

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// CONFIGURACIONES DE LA LCD
void LCD_CONFIG()
{
  lcd.begin(lcdColumns, lcdRows);
  lcd.init(SDA, SCL); // pines configurados para la comunicacion I2C
  lcd.backlight();    // Encendemos la pantalla
}

void LCD_MENSAJE(String msj1, IPAddress msj2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msj1);
  lcd.setCursor(0, 1);
  lcd.print(msj2);
}

void LCD_MENSAJE(String msj1)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(msj1);
}

void LCD_MENSAJE(String mensaje, float valor, int decimales)
{
  lcd.setCursor(0, 1);
  lcd.print(mensaje);
  lcd.print(valor, decimales);
}
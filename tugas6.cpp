#include <LiquidCrystal_I2C.h>
int ldrPin = A0;
int ledPin = 2;
const float GAMMA = 0.7;
const float RL10 = 50;

LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup()
{
    pinMode(ledPin, OUTPUT);
    lcd.init();
    lcd.backlight();
}
void loop()
{
    // put your main code here, to run repeatedly:
    int ldrValue = analogRead(ldrPin);
    // nilaiLDR = map(nilaiLDR, 4095, 0, 1024, 0); //mengubah nilai pembacaan sensor LDR dari nilai ADC arduino menjadi nilai ADC ESP32   float voltase = ldrValue / 1024. * 5;   float resistansi = 2000 * voltase / (1-voltase / 5);
    float kecerahan = pow(RL10 * 1e3 * pow(10, GAMMA) / resistansi, (1 / GAMMA));
    lcd.setCursor(0, 0);
    lcd.print("Cahaya: ");
    if (kecerahan > 50)
    {
        lcd.print("Terang");
        digitalWrite(ledPin, LOW);
    }
    else
    {
        lcd.print("Gelap ");
        digitalWrite(ledPin, HIGH);
    }
    lcd.setCursor(0, 1);
    lcd.print("Lux: ");
    lcd.print(kecerahan);
    lcd.print("");
    delay(1000);
}
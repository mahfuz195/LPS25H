#include <Wire.h>
#include <LPS25H.h>

LPS25H ps;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  if (!ps.init())
  {
    Serial.println("Failed to autodetect pressure sensor!");
    while (1);
  }

  ps.enableDefault();
}

void loop()
{
  float pressure = ps.readPressureInchesHg();
  float altitude = ps.pressureToAltitudeFeet(pressure);
  float temperature = ps.readTemperatureF();

  Serial.print("p: ");
  Serial.print(pressure);
  Serial.print(" inHg\ta: ");
  Serial.print(altitude);
  Serial.print(" ft\tt: ");
  Serial.print(temperature);
  Serial.println(" deg C");

  delay(100);
}

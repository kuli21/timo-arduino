#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define out_LEDblue 2
#define out_LEDgreen 3
#define out_LEDred 4

#define taster1 5
#define taster2 6
#define taster3 7

#define DHTPIN 8   
#define DHTTYPE DHT22 //DHT11, DHT21, DHT22
DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(out_LEDblue, OUTPUT);
  pinMode(out_LEDgreen, OUTPUT);
  pinMode(out_LEDred, OUTPUT);

  pinMode(taster1,INPUT);
  pinMode(taster2,INPUT);
  pinMode(taster3,INPUT);

  dht.begin();
}

void loop()
{  
  
  humidity = dht.readHumidity();                           
  temperature = dht.readTemperature();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT22 konnte nicht ausgelesen werden");
  } else {
    if (digitalRead(taster2) == HIGH){
      Serial.print("Luftfeuchte: "); 
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperatur: "); 
      Serial.print(temperature);
      Serial.println(" C");
    }
  }
  
  if (digitalRead(taster1) == HIGH){
    Serial.println("Taster AN:");
    colorLed(3);
    delay(1000);
  } else {
    digitalWrite(out_LEDblue, LOW);
  }
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    colorLed(1);
    return;
  }
  
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    colorLed(1);
    return;
  }
  
  long code=0; // Als neue Variable fügen wir „code“ hinzu, unter welcher später die UID als zusammenhängende Zahl ausgegeben wird. Statt int benutzen wir jetzt den Zahlenbereich „long“, weil sich dann eine größere Zahl speichern lässt.
  
  for (byte i = 0; i < mfrc522.uid.size; i++){ 
    code=((code+mfrc522.uid.uidByte[i])*10); // Nun werden wie auch vorher die vier Blöcke ausgelesen und in jedem Durchlauf wird der Code mit dem Faktor 10 „gestreckt“. (Eigentlich müsste man hier den Wert 1000 verwenden, jedoch würde die Zahl dann zu groß werden.
  }
  
  Serial.print("Die Kartennummer lautet:"); // Zum Schluss wird der Zahlencode (Man kann ihn nicht mehr als UID bezeichnen) ausgegeben.
  Serial.println(code);
  colorLed(2);
  delay(1000);
}


void colorLed(int color)
{
  if (color == 1) {
    digitalWrite(out_LEDred, HIGH);
    digitalWrite(out_LEDblue, LOW);
    digitalWrite(out_LEDgreen, LOW);
  } else if (color == 2) {
    digitalWrite(out_LEDred, LOW);
    digitalWrite(out_LEDblue, LOW);
    digitalWrite(out_LEDgreen, HIGH);    
  } else if (color == 3) {
    digitalWrite(out_LEDred, LOW);
    digitalWrite(out_LEDblue, HIGH);
    digitalWrite(out_LEDgreen, LOW);    
  } else {
    digitalWrite(out_LEDred, LOW);
    digitalWrite(out_LEDblue, LOW);
    digitalWrite(out_LEDgreen, LOW);    
  }
}
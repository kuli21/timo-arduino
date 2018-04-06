#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define taster1 5
#define taster2 6
#define taster3 7

#define DHTPIN 8   
#define DHTTYPE DHT22 //DHT11, DHT21, DHT22
DHT dht(DHTPIN, DHTTYPE);
float humidity, temperature;

int t1, t2, t3, nc, t_shutdown, t_mute, t_volume, t_next;

long rfiduid;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(taster1,INPUT);
  pinMode(taster2,INPUT);
  pinMode(taster3,INPUT);

  dht.begin();  
  rfiduid = 0;

  t_shutdown = 0;
  t_mute = 0;
}

void loop()
{  
  t1 = 0;
  t2 = 0;
  t3 = 0;  
  t_volume = 0;
  t_next = 0;
  //Get temperature
  humidity = dht.readHumidity();                           
  temperature = dht.readTemperature();
  //Get Keys
  if (digitalRead(taster1) == HIGH ){
    t1 = 1;    
  }
  if (digitalRead(taster2) == HIGH){
    t2 = 1;
  }
  if (digitalRead(taster3) == HIGH){
    t3 = 1;
  }
  //Calc Buttons
  if (t1 == 1 && t2 == 0 && t3 == 0) {
    t_volume = -1;
  }
  if (t1 == 0 && t2 == 1 && t3 == 0) {
    t_volume = 1;
  }
  if (t1 == 0 && t2 == 0 && t3 == 1) {
    t_next = 1;
  }
  // ---- MUTE ------------------
  if (t1 == 1 && t2 == 1 && t3 == 0 && t_mute < 9999) {
    t_mute = t_mute + 1;
    t_volume = 0;
  } else {
    t_mute = 0;
  }
  // -------- SHUTDOWN -----------------------
  if (t1 == 1 && t3 == 1 && t2 == 0 && t_shutdown < 9999) {
    t_shutdown = t_shutdown + 1;
    t_volume = 0;
    t_mute = 0;
    t_next = 0;
  } else {
    t_shutdown = 0;
  }
  //---------------------------------------------
  if (isnan(temperature) || isnan(humidity)) {
    temperature = 999;
    humidity = 999;
  }

  //-------------------------------------------------
  //RFID Code ---------------------------------------
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  nc = 1;
  MFRC522::StatusCode status;
  byte block;
  byte len;
  byte ac;
  byte bufferall[80];
  byte bufferx[18];
  String urlString;
  String errorTxt = "";
  int startString = 0;
  int endString = 0;
  
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    nc = 0;
    rfiduid = 0;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    nc = 0;
    rfiduid = 0;
  }
  if (nc == 1) {
    //Card detected
    for (byte i = 0; i < mfrc522.uid.size; i++){ 
      rfiduid = ((rfiduid + mfrc522.uid.uidByte[i]) * 10);
    }    
    len = 18;
    ac = 0;
    for (uint8_t t = 4; t <= 8; t++) {
        block = t;
        if (block != 7) {
          status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &key, &(mfrc522.uid));
            if (status != MFRC522::STATUS_OK) {
              errorTxt = "Authentication failed: " + String(mfrc522.GetStatusCodeName(status));
              //Serial.println("error1");
            }          
            status = mfrc522.MIFARE_Read(block, bufferx, &len);
            if (status != MFRC522::STATUS_OK) {
              //Serial.println("error2");
              errorTxt = "Reading failed: " + String(mfrc522.GetStatusCodeName(status));
            }
            if (errorTxt == "") {
              for (uint8_t i = 0; i < 16; i++) {
                bufferall[ac] = bufferx[i];
                ac = ac + 1;
              }
            }            
        }     
     }      
     if (errorTxt == "") {
       for (char c : bufferall) {
           urlString += String(c);
       }
       startString = urlString.indexOf('{');
       endString = urlString.indexOf('}'); 
     }
     delay(100);
     mfrc522.PICC_HaltA();
     mfrc522.PCD_StopCrypto1();
  }
  //------------------------------------------------

  //Print JSON
  Serial.print("{\"temp\":\"" + String(temperature) +
              "\", \"humid\":\"" + String(humidity) +
              "\", \"t1\":\"" + String(t1) +
              "\", \"t2\":\"" + String(t2) +
              "\", \"t3\":\"" + String(t3) +
              "\", \"mute\":\"" + String(t_mute) +
              "\", \"shutdown\":\"" + String(t_shutdown) +
              "\", \"volume\":\"" + String(t_volume) +
              "\", \"next\":\"" + String(t_next) +
              "\", \"rfiduid\":\"" + String(rfiduid) +
              "\", \"rfiddata\":\"");   
  Serial.print(urlString.substring((startString+1), endString));
  if (errorTxt != "") {
    Serial.print(errorTxt);
  }
  Serial.println("\"}");

}

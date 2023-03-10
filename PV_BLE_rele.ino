#include <esp_now.h>
#include <WiFi.h>
#include <BLEDevice.h> 
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "ffe0" // UART service UUID a5f81d42-f76e-11ea-adc1-0242ac120002
#define CHARACTERISTIC_UUID_RX "ffe1"
#define CHARACTERISTIC_UUID_TX "ffe2"
#define rele 33

typedef struct struct_message {
    int msj;
} struct_message;

// Create a struct_message called myData
struct_message myData;

int cont = 0;
int reseteo = 35;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Mensaje: ");
  Serial.println(myData.msj);
  Serial.println();
  Serial.println("ACTIVANDO RELÉ Y REINICIANDO ESP");
  digitalWrite(rele, HIGH);
  delay(2000);
  digitalWrite(rele, LOW);
  delay(500);
 ESP.restart();
  
}


BLECharacteristic *pCharacteristic;
bool deviceConnected = false;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("***");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

 if (rxValue.find("1") != -1) {
  Serial.println("ACTIVANDO RELÉ Y REINICIANDO ESP");
  digitalWrite(rele, HIGH);
  delay(2000);
  digitalWrite(rele, LOW);
  ESP.restart();
  }

      } 
  }
};

void setup() {
   Serial.begin(115200);
   pinMode(rele, OUTPUT);
     Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  WiFi.mode(WIFI_STA);
   Serial.println("<--------- RECEPTOR BLE Y ACTIVADOR DE RELÉ ---------->");

    

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
 // Create the BLE Device
  BLEDevice::init("Beacon JC"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}

 
void loop() {

cont++;
delay(1000);
if (cont >= reseteo){
  Serial.println("<<<<<<<<<< REINICIANDO SOFTWARE >>>>>>>>>>");
ESP.restart();
}
}

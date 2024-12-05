#include <mcp_can.h>
#include <SPI.h>
#include <time.h>

#define s1r Serial1.read // fixed:it was serial isntaed of serial 1
#define print Serial.println

#define CAN0_INT 2  
MCP_CAN CAN0(53); 

// Definizione dei pin per il secondo modulo MCP2515
#define CAN1_INT 3  // Pin di interruzione per CAN1
MCP_CAN CAN1(10);  // Pin CS per il secondo modulo CAN

time_t last_filter_fetching = (time_t) (0);
const int FILTER_FETCHING_CYCLE_DELAY = 4;
const int FILTER_COUNT = 5;


void setup() {
  Serial.begin(9600);
  Serial1.being(115200);

  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);

  // Assicurati che il pin 10 (CS per CAN1) sia impostato come output
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  // Inizializzazione del primo modulo MCP2515 (CAN0)
  if (CAN0.begin(MCP_STDEXT, CAN_250KBPS, MCP_8MHZ) == CAN_OK) 
    {
    Serial.println("CAN0 Initialized Successfully!");

    CAN0.init_Mask(0, 1, 0x1FFFFFFF); 
    for (int i = 0; i < FILTER_COUNT; i++) 
      {
      CAN0.init_Filt(i, 1, 0x00000000); 
      }
    CAN0.setMode(MCP_NORMAL);
    } 
    
else 
    {
    Serial.println("Error Initializing CAN0...");
    while (1);
    }

  // Inizializzazione del secondo modulo MCP2515 (CAN1)
  if (CAN1.begin(MCP_STDEXT, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN1 Initialized Successfully!");
  } else {
    Serial.println("Error Initializing CAN1...");
    while (1);
  }

  CAN1.setMode(MCP_NORMAL); // Imposta CAN1 in modalità normale

  Serial.println("MCP2515 Dual CAN Bus Example...");
}

void loop() {

  long unsigned int rxId;
  unsigned char len = 0;
  unsigned char rxBuf[8];

  if (!digitalRead(CAN0_INT)) {
    // Leggi il messaggio da CAN0
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

 

    // Invia il messaggio letto a CAN1
    if (CAN1.sendMsgBuf(rxId, 1, len, rxBuf) == CAN_OK) {
      Serial.print("Message relayed to CAN1 ID: 0x");
      Serial.print(rxId, HEX);
      Serial.println();
    } else {
      Serial.println("Error sending message to CAN1");
    }
  }

  time_t now;
  time(&now);
  bool condition_1 = (now - last_filter_fetching ) > FILTER_FETCHING_CYCLE_DELAY;
  print("condition_1:");
  print(condition_1);
  if (condition_1)
    {
    Serial.println("sending the filter fetching command to esp");
    Serial1.write(85);
    Serial.println("sent the filter fetching command to esp");
    Serial1.flush();
    Serial.println("now waiting for esp to send the filters");
    delay(10); // calcualted 
    unsigned long filters[FILTER_COUNT];
    for (int index = 0; index != FILTER_COUNT; index += 1)
      {
      //reassembling the filters , byte by byte

      filters[index] += (Serial1.readStringUntil("\n")).toInt();
      print(filters[index])
      print(index);
      print(" -> ");
      print(filters[index]);
      }
    
    for (int index = 0; index != FILTER_COUNT; index+= 1)
      {
      CAN0.init_Filt(index, 1, filter[index]);
      }
    time(&last_filter_fetching);
    }



  







}

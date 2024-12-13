#include <time.h>
#define filter_data_serial Serial
#define s1r filter_data_serial.read // fixed:it was serial isntaed of serial 1
#define print Serial.println


//CAN PRES ===============================
#include <mcp_can.h>
#define CAN0_INT 2 // Set INT to pin 2
#define CAN1_INT 3 // Set INT to pin 3
// =========================================

//CAN GLOBALS ===============================
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];
MCP_CAN CAN0(53); // Set CS to pin 53 for CAN0
MCP_CAN CAN1(10); // Set CS to pin 10 for CAN1
// =========================================


unsigned long int last_filter_fetching = 0;
const int FILTER_FETCHING_CYCLE_DELAY = 4000;
const int FILTER_COUNT = 5;
const int filter_seprator = 120; //character new line

void setup() 
  {
  Serial.begin(9600);
  filter_data_serial.begin(9600);

  //CAN SETUP THINGS ===========================
  if (CAN0.begin(MCP_STDEXT, CAN_250KBPS, MCP_8MHZ) == CAN_OK)
    {
    print("CAN0 Initialized Successfully!");
    CAN0.init_Mask(0, 1, 0x1FFFFFFF); 
    for (int i = 0; i < FILTER_COUNT; i++) 
      {
      CAN0.init_Filt(i, 1, 0x00000000); 
      }
    CAN0.setMode(MCP_NORMAL); 
    } 
  else 
      {
      print("Error Initializing CAN0...");
      while (1);
      }

  if (CAN1.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) 
    {
    print("MCP2515 CAN1 Initialized Successfully!");
    } 
  else 
    {
    print("Error Initializing MCP2515 CAN1...");
    while (1);
    }

  CAN1.setMode(MCP_NORMAL); // Set operation mode to normal for CAN1

  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input (CAN0)
  pinMode(CAN1_INT, INPUT); // Configuring pin for /INT input (CAN1)
  // ===========================================

  }


void loop() {
  if (!digitalRead(CAN0_INT)) 
    {
    // Leggi il messaggio da CAN0
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    if ((rxId & 0x80000000) == 0x80000000) 
      {
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
      } 
    else 
      {
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
      }
    Serial.print(msgString);
    if ((rxId & 0x40000000) == 0x40000000) 
      {
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
      } 
    else 
      {
      for (byte i = 0; i < len; i++) 
        {
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
        }
      }

    Serial.println();

    // Invia il messaggio letto su CAN1
    if (CAN1.sendMsgBuf(rxId, 1, len, rxBuf) == CAN_OK) 
      {
      Serial.print("Message relayed to CAN1 ID: 0x");
      Serial.println(rxId, HEX);
      } 
    else 
      {
      Serial.println("Error sending message to CAN1");
      }
    }


  unsigned long int now = millis();
  bool condition_1 = (now - last_filter_fetching ) > FILTER_FETCHING_CYCLE_DELAY;
  print(".");
  delay(1000);
  if (condition_1)
    {
    print("");
    print("sending the filter fetching command to esp");
    filter_data_serial.write(85);
    print("sent the filter fetching command to esp");
    filter_data_serial.flush();
    print("now waiting for esp to send the filters");
    unsigned long filters[FILTER_COUNT];
    for (int index = 0; index != FILTER_COUNT; index += 1)
      {
      while (filter_data_serial.available() == 0)
        {
        print("no data received yet");
        delay(1000);
        }
      print("some data are now sent from esp; going to read them");
      String buffer = "";
      while (true)
        {
        int data = filter_data_serial.read();
        if (data == -1 || data == 10)
          {
          //when no data is actually availbale in buffer 
          print("null data was in buffer (-1)");
          delay(1000);
          continue;
          }
        if (data == filter_seprator)
          {
          print("encountred the sperator; breaking the reading loop");
          break;
          }
        print("recieved data of :");
        print(data);
        buffer += (char) data;
        }
      print("buffer [STRING]:");
      print(buffer);
      filters[index] = strtoul(buffer.c_str(), NULL, 10);
      print("buffer [UNSIGNED LONG]:");
      }

    print("=======================");
    print("filters are:");
    for (int filter : filters)
      {
      print(filter);
      }
    
    last_filter_fetching = now;
    }

}

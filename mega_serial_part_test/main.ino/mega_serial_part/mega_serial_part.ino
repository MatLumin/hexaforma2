
#include <time.h>
#define filter_data_serial Serial
#define s1r filter_data_serial.read // fixed:it was serial isntaed of serial 1
#define print Serial.println

unsigned long int last_filter_fetching = 0;
const int FILTER_FETCHING_CYCLE_DELAY = 4000;
const int FILTER_COUNT = 5;
const int filter_seprator = 120; //character new line

void setup() {
  Serial.begin(9600);
  filter_data_serial.begin(9600);

}
void loop() {
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

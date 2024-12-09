
#include <time.h>
#define filter_data_serial Serial1
#define s1r filter_data_serial.read // fixed:it was serial isntaed of serial 1
#define print Serial.println

unsigned long int last_filter_fetching = 0;
const int FILTER_FETCHING_CYCLE_DELAY = 4000;
const int FILTER_COUNT = 5;
void setup() {
  Serial.begin(9600);
  filter_data_serial.being(115200);
}
void loop() {
  unsigned long int now = milis();
  bool condition_1 = (now - last_filter_fetching ) > FILTER_FETCHING_CYCLE_DELAY;
  print("condition_1:");
  print(condition_1);
  if (condition_1)
    {
    Serial.println("sending the filter fetching command to esp");
    filter_data_serial.write(85);
    Serial.println("sent the filter fetching command to esp");
    filter_data_serial.flush();
    Serial.println("now waiting for esp to send the filters");
    delay(20); // calcualted 
    unsigned long filters[FILTER_COUNT];
    for (int index = 0; index != FILTER_COUNT; index += 1)
      {
      //reassembling the filters , byte by byte

      filters[index] += (filter_data_serial.readStringUntil("\n")).toInt();
      print(filters[index])
      print(index);
      print(" -> ");
      print(filters[index]);
      }
    
    for (int index = 0; index != FILTER_COUNT; index+= 1)
      {
      CAN0.init_Filt(index, 1, filters[index]);
      }
    last_filter_fetching = now;
    }

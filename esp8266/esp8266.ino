#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#define data_serial Serial1


const int FILTER_COUNT = 5;



//COMMANDS CODES 
const int CMD_FECTH_FILTERS_CODE = 85;

const String INDEX_HTML = R"(
<style>
:root {
  --nik_comp_color:#e02e28;
}
body {
  width: 100vw;
  height: 100vh;
  background: linear-gradient(0deg, #1a1a1a, rgb(34, 34, 34));
}
html {
  width: 100vw;
  height: 100vh;
}
* {
  font-family: sans-serif;
  font-size: 3vh;
  border-radius: 2vh;
  font-weight: bold;
}
input {
  display: block;
  background: white;
  color: black;
  width: 80%;
  margin-bottom: 5%;
  padding: 2%;
  text-align:center;
  border: 2px solid black;
}
input:focus {
  border-color: var(--nik_comp_color);
  outline-style: none;
}
button {
  border-style: none;
  padding: 5%;
  transition: 0.3s;
  background-color: var(--nik_comp_color);
  color: white;
}
button:hover {
  transform: translateY(-20px);
  transition: 0.3s;
}
button:active {
  transform: translateY(+20px);
  transition: 0.3s;
}
#main_form {
  display: flex;
  flex-direction: column;
  align-items: center;
}
label {
  color: var(--nik_comp_color);
}
</style>
<title>HEXA FORM</title>
<body>
<form id="main_form">
  <label>Field 1</label>
  <input type="text" value="HEX_VALUE_PLACE_HOLDER_0" name="0" maxlength="10">
  <label>Field 2</label>
  <input type="text" value="HEX_VALUE_PLACE_HOLDER_1" name="1" maxlength="10">
  <label>Field 3</label>
  <input type="text" value="HEX_VALUE_PLACE_HOLDER_2" name="2" maxlength="10">
  <label>Field 4</label>
  <input type="text" value="HEX_VALUE_PLACE_HOLDER_3" name="3" maxlength="10">
  <label>Field 5</label>
  <input type="text" value="HEX_VALUE_PLACE_HOLDER_4" name="4" maxlength="10">
  <button type="submit">Submit</button>
</form>
</body>
)";

ESP8266WebServer server(80);



void print_all_filters()
	{
  File file = SPIFFS.open("/data.txt", "r");
  while (file.available())
    {
    char charcode = (file.read());
    Serial.print(charcode);
    }
	}


void print_data_file()
  {
  
  }


void hh_ui_index() {
  unsigned long data[FILTER_COUNT] = {0};
  for (int i = 0; i < FILTER_COUNT; i++) 
    {
	  if (server.hasArg(String(i))) 
      {
      String value = server.arg(String(i));
      value.toUpperCase();
      value.replace("0X", "");
	    data[i] = strtol(value.c_str(), NULL, 16);
	    }
    }


  File file = SPIFFS.open("/data.txt", "w");
  for (int i = 0; i < FILTER_COUNT; i++) 
    {
	  file.println(data[i]);
    }
  file.close();


  file = SPIFFS.open("/data.txt", "r");
  for (int i = 0; i < FILTER_COUNT && file.available(); i++)
	  {
	  data[i] = file.readStringUntil('\n').toInt();
	  }
  file.close();


  String output = INDEX_HTML;
  for (int i = 0; i < FILTER_COUNT; i++)
	  {
	  output.replace("HEX_VALUE_PLACE_HOLDER_" + String(i), "0X"+String(data[i], HEX));
	  }


  server.send(200, "text/html", output);
}


String generate_ssid() {
  String output = "NIK-GTW-";
  String mac_addrs = WiFi.softAPmacAddress();
  mac_addrs = mac_addrs.substring(mac_addrs.length() - 5);
  output += mac_addrs;
  return output;
}

void setup() {
  Serial.begin(9600);
	Serial1.begin(9600);

  if (!SPIFFS.begin()) {
	Serial.println("SPIFFS FAILED TO START");
	return;
  }


  if (!SPIFFS.exists("/data.txt")) {
	File file = SPIFFS.open("/data.txt", "w");
	for (int i = 0; i < FILTER_COUNT; i++) {
	  file.println(0);
	}
	file.close();
  }

  print_all_filters();


  WiFi.mode(WIFI_AP);
  WiFi.softAP(generate_ssid().c_str(), "12345678");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("IP ADDRESS");
  Serial.println(IP);


  server.on("/index", hh_ui_index);
  server.begin();


}

void loop() {
  server.handleClient();

  if (data_serial)
    {
    int bytes_sent = data_serial.available();
    if (bytes_sent > 1)
      {
      int first_byte = data_serial.read();
      if (first_byte == CMD_FECTH_FILTERS_CODE)
        {
        data_serial.flush();
        Serial.println("SENDING THE FILTERS!!");
        File file = SPIFFS.open("/data.txt", "r");
        for (int i = 0; i < FILTER_COUNT && file.available(); i++)
          {
          String current = file.readStringUntil('\n');
          Serial.println(current);
          current += "x";
          data_serial.print(current);
          }
        file.close();


        }
      }
    }

}
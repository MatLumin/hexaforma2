#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>

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


unsigned long int * read_filters_from_spiffs()
	{
	unsigned long int data[FILTER_COUNT];
	File file = SPIFFS.open("/data.txt", "r");
	for (int i = 0; i < FILTER_COUNT && file.available(); i++)
		{
		data[i] = file.readStringUntil('\n').toInt();
		}
	file.close();
	return data;
	}

void print_all_filters()
	{
	unsigned long int * data = read_filters_from_spiffs();
	Serial.println("PRINTING ALL FILTERS ---------")
	for (int index=0; index!=FILTER_COUNT; index+=1)
		{
		Serial.println(data[index]);
		}
	Serial.println("------------------------------")
	}


void hh_ui_index() {
  int data[FILTER_COUNT] = {0};
  for (int i = 0; i < FILTER_COUNT; i++) {
	if (server.hasArg(String(i))) {
	  data[i] = strtol(server.arg(String(i)).c_str(), NULL, 16);
	}
  }


  File file = SPIFFS.open("/data.txt", "w");
  for (int i = 0; i < FILTER_COUNT; i++) {
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
	output.replace("HEX_VALUE_PLACE_HOLDER_" + String(i), String(data[i], HEX));
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
  Serial.begin(115200);
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

	int bytes_sent = Serial1.available();
	if (bytes_sent > 1)
		{
		int fisrt_byte = Serial1.read();
		if (first_byte == CMD_FECTH_FILTERS_CODE)
			{
			Serial.flush();
			unsigned long int * data = read_filters_from_spiffs();
			for (int i = 0; i!= FILTER_COUNT; i+=1)
				{
				unsigned long int value = data[i];
				Serial1.write((value & 0b00000000000000000000000011111111) >> 0);
				Serial1.write((value & 0b00000000000000001111111100000000) >> 8);
				Serial1.write((value & 0b00000000111111110000000000000000) >> 16);
				Serial1.write((value & 0b11111111000000000000000000000000) >> 24);
				}
			}
		}

}
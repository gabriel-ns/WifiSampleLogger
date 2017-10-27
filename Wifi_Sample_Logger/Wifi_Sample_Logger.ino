#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// Disponível em https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>

#define RED_LED		D1
#define BLUE_LED 	D5
#define YELLOW_LED	D2

#define DHT11_DATA_PIN D3

const char HTTP_HEAD[]          PROGMEM = "<!DOCTYPEhtml><htmllang=\"en\"><head><metacharset=\"UTF-8\"><title>ESP8266WiFiSampleServer</title><style>body{text-align:center;font-family:Calibri;margin:0;}button{border:0;border-radius:0.3rem;color:#fff;line-height:2.4rem;font-size:1.2rem;width:15%;}.btn_blue{background-color:darkblue;}.btn_yellow{background-color:goldenrod;}.btn_red{background-color:firebrick;}</style></head>";
const char HTTP_BUTTON[]		PROGMEM = "<a href=\"?{arg}={st}\"><button type=\"button\" class=\"{style}\">{msg}</button></a>";
const char HTTP_TEMP_GRAPH[]    PROGMEM = "<div>GRAPH_FRAME</div>";
const char HTTP_HUM_GRAPH[]     PROGMEM = "<div>GRAPH_FRAME</div>";

const char * ssid = "SSID";
const char * pw = "PASSWORD";

const char* host = "script.google.com";
const int httpsPort = 443;
const char * baseURL = "/macros/s/SCRIPT_IDexec";

DHT dht(DHT11_DATA_PIN, DHT11);

ESP8266WebServer server(80);
DNSServer        dnsServer;
uint32_t lastSensorRead = 0;
uint32_t lastUpload = 0;

int16_t temperature = 0;
float humidity = 0;

bool red_led_state = 0;
bool blue_led_state = 0;
bool yellow_led_state = 0;

void handleRoot();

void setup()
{
	Serial.begin(115200);

	dht.begin();

	temperature = (int16_t) dht.readTemperature();
	humidity = dht.readHumidity();

	pinMode(RED_LED, OUTPUT);
	pinMode(BLUE_LED, OUTPUT);
	pinMode(YELLOW_LED, OUTPUT);

	digitalWrite(RED_LED, red_led_state);
	digitalWrite(BLUE_LED, blue_led_state);
	digitalWrite(YELLOW_LED, yellow_led_state);

	WiFi.mode(WIFI_AP_STA);
	String stationName = "MyStation";
	WiFi.softAP(stationName.c_str(), "");

	WiFi.begin(ssid, pw);
	while(WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(100);
	}
	Serial.println("Connected! \nLocal IP = " + WiFi.localIP().toString());

    /* Adiciona as páginas da web do dispositivo */
    server.on("/", handleRoot);
    server.begin();

}

void loop()
{
	if(millis() - lastSensorRead >= 5000)
	{
		lastSensorRead = millis();
		temperature = (int16_t) dht.readTemperature();
		humidity = dht.readHumidity();
	}

	if(millis() - lastUpload >= 1000*60*10)
	{
		lastUpload = millis();

		String temp = String(temperature);
		String hum = String(humidity);
		hum.replace('.',',');

		String url = String(baseURL) + "?sensor_id=ControlStation";
		url += "&temp=" + String(temp);
		url += "&hum=" + String(hum);
		WiFiClientSecure client;

		if (!client.connect(host, httpsPort)) {
			Serial.println("connection failed");
		}
		else
		{
			client.print(String("GET ") + url + " HTTP/1.1\r\n" +
					"Host: " + host + "\r\n" +
					"User-Agent: BuildFailureDetectorESP8266\r\n" +
					"Connection: close\r\n\r\n");
		}
	}

	server.handleClient();
}

void handleRoot()
{
	if(server.args() > 0)
	{
		String arg = server.argName(0);
		String value = server.arg(0);

		if(arg.indexOf("red_led") != -1)
		{
			if(value.indexOf("on") != -1)
			{
				digitalWrite(RED_LED, HIGH);
				red_led_state = 1;
			}
			else
			{
				digitalWrite(RED_LED, LOW);
				red_led_state = 0;
			}
		}
		else if(arg.indexOf("blue_led") != -1)
		{
			if(value.indexOf("on") != -1)
			{
				digitalWrite(BLUE_LED, HIGH);
				blue_led_state = 1;
			}
			else
			{
				digitalWrite(BLUE_LED, LOW);
				blue_led_state = 0;
			}
		}
		else if(arg.indexOf("yellow_led") != -1)
		{
			if(value.indexOf("on") != -1)
			{
				digitalWrite(YELLOW_LED, HIGH);
				yellow_led_state = 1;
			}
			else
			{
				digitalWrite(YELLOW_LED, LOW);
				yellow_led_state = 0;
			}
		}
	}

	String message = FPSTR(HTTP_HEAD);

	message += "<body><p><h1>Control Station</h1></p>";
	message += "<p><h3> Humidity = {h}</h3><h3> Temperature = {t}</h3></p>";
	message.replace("{h}", String(humidity) + " %");
	message.replace("{t}", String(temperature) +  " C");

	message += "<div><p>";
	message += FPSTR(HTTP_BUTTON);
	message.replace("{arg}", "red_led");
	message.replace("{st}", red_led_state ? "off" : "on");
	message.replace("{style}", "btn_red");
	message.replace("{msg}", "Switch LED");

	message += FPSTR(HTTP_BUTTON);
	message.replace("{arg}", "blue_led");
	message.replace("{st}", blue_led_state ? "off" : "on");
	message.replace("{style}", "btn_blue");
	message.replace("{msg}", "Switch LED");

	message += FPSTR(HTTP_BUTTON);
	message.replace("{arg}", "yellow_led");
	message.replace("{st}", yellow_led_state ? "off" : "on");
	message.replace("{style}", "btn_yellow");
	message.replace("{msg}", "Switch LED");
	message += "</div></p>";

	message += FPSTR(HTTP_TEMP_GRAPH);
	message += FPSTR(HTTP_HUM_GRAPH);

	message += "</body></html>";

	server.send(200, "text/html", message);
}

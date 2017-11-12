/**
 * SGN - Robótica Educacional
 * OUT-2017
 *
 * Projeto: Exemplo de datalogger com ESP01 e Arduino UNO
 *
 * Resumo: Utilização do ESP01 como modem WiFi para hospedar um
 * servidor local comandado pelo Arduino UNO, com uma webpage que permite
 * o controle do led onboard e mostra ao cliente o millis() e o gráfico da
 * evolução do millis() obtido no Google Drive.
 *
 * Periodicamente os dados lidos são enviados para uma planilha do Google
 * utilizando o PushingBox e o Google App Script
 *
 */

#include <Arduino.h>
#include <SoftwareSerial.h>

#define SERIAL_LOW_SPEED 9600
#define SERIAL_HIGH_SPEED 115200
#define WIFI_RX_PIN 2
#define WIFI_TX_PIN 3

static bool espCheckPresence(uint16_t timeout, uint8_t retries);
static void espBegin();
static void wifiSetup();
static void handleServerClient();
static void wifiStartServer();
static void wifiSendData(uint32_t value);
static String sendModemCommand(String cmd, uint16_t timeout, uint8_t retries = 1);
static void serverProccessClientCommand();

String graph = "<div><iframe width=\"600\" height=\"371\" seamless frameborder=\"0\" scrolling=\"no\" src=\"https://docs.google.com/spreadsheets/d/e/2PACX-1vR8s42u9PIdtwFOw-40zyTslirCnkRh3qC8VLIpW0g0hfNXG_YMYwXO9wqkZMWlw2mI-HxiabLMg6jz/pubchart?oid=249031932&amp;format=interactive\"></iframe></div>";

SoftwareSerial SerialWifi(WIFI_RX_PIN,WIFI_TX_PIN);

uint32_t lastDataSent = millis();
uint16_t dataInterval = 60000;
String SSID = "UFABC";
String PW = "85265";

void setup()
{
	Serial.begin(SERIAL_LOW_SPEED);

	pinMode(13, OUTPUT);

	espBegin();

	wifiSetup();

	wifiStartServer();
}

void loop()
{
	/* Trata as requisições pendentes */
	handleServerClient();

	/* Envia os dados para a nuvem quando necessário */
	if(millis() - lastDataSent >= dataInterval)
	{
		lastDataSent = millis();
		wifiSendData(millis());
	}
	if(Serial.available() > 0)
	{
		Serial.read();
		sendModemCommand("AT+CIFSR\r\n", 1000);
	}
}

static String sendModemCommand(String cmd, uint16_t timeout, uint8_t retries)
{
	/* Cria uma string onde a resposta será armazenada */
	String response = "";
	uint8_t ret = retries;

	/* Tenta enviar o comando. Se não obtiver sucesso, tentará retries vezes */
	do{
		Serial.println("Sending: " + cmd);
		SerialWifi.print(cmd);
		uint32_t currentTime = millis();

		/* Após enviar o comando, aguarda pelo OK. Se ocorrer o timeout, ele desiste
		 * e vai para a próxima tentativa*/
		while(millis() < (currentTime + timeout))
		{
			while(SerialWifi.available()) response += (char) SerialWifi.read();
			/* Quebra o loop se o OK for encontrado */
			if(response.indexOf("OK") != -1) break;
		}
		Serial.println("Response " + String(retries - ret) + ": " + response);
		if(response.indexOf("OK") != -1) break;
		response = "";
	}while(ret-- > 1);

	Serial.println(response);
	return response;
}

static bool espCheckPresence(uint16_t timeout, uint8_t retries)
{
	/* Cria uma string onde a resposta será armazenada */
	String response = "";

	/* Envia a string AT diversas vezes, até conseguir uma resposta
	 * OK ou acabar o tempo/número de tentativas*/
	do{
		SerialWifi.print("AT\r\n");
		uint32_t currentTime = millis();
		while(millis() < (currentTime + timeout))
		{
			while(SerialWifi.available()) response += (char) SerialWifi.read();
			if(response.indexOf("OK") != -1) return true;
		}
		Serial.println(response);
		response = "";
	}while(retries-- > 1);

	return false;
}

static void espBegin()
{
	String cmd = F("AT+RST\r\n");
	bool presence = false;
	/* Ao ligar, o ESP está configurado para a velocidade de 115200.
	 * No início do programa, o ESP deve sofrer um RESET */
	SerialWifi.begin(SERIAL_HIGH_SPEED);

	/* Procura o ESP com BaudRate = 115200 */
	Serial.println("Looking for ESP8266 at SERIAL_HIGH_SPEED");
	presence = espCheckPresence(200, 10);

	/* Se ele for encontrado, enviar um comando de reset,
	 * se não, procura o sensor com baud = 19200 */
	if(presence)
	{
		/* Reseta o sensor após encontrá-lo */
		Serial.println("ESP8266 found with SERIAL_HIGH_SPEED");
		sendModemCommand(cmd, 2000);

		/* Aguarda o final do procedimento de reset, enviando AT até ele respodner OK */
		do
		{
			Serial.println("Waiting for reset..");
		}while( espCheckPresence(500, 10));

		/* Configura o ESP para Baud = 19200 */
		Serial.println("Setting the new baud rate");
		sendModemCommand("AT+CIOBAUD=" + String(SERIAL_LOW_SPEED) + "\r\n", 1000, 5);

	}
	else
	{
		/* Se o ESP não é encontrado com baud = 115200, tentamos com baud = 19200 */
		Serial.println("ESP8266 NOT found with SERIAL_HIGH_SPEED :(");
		SerialWifi.end();
		SerialWifi.begin(SERIAL_LOW_SPEED);
		Serial.println("Looking for ESP8266 at SERIAL_LOW_SPEED");
		presence = espCheckPresence(200, 10);

		/* Se o ESP for encontrado com baud = 19200, enviamos o comando de reset
		 * e depois o configuramos para a velocidade de 19200.
		 *
		 * Se o ESP não for encontrado, o código trava num erro. Deve-se analisar o hardware
		 */
		if(presence)
		{
			/* Envia o comando de reset */
			Serial.println("ESP8266 found with SERIAL_LOW_SPEED. Resetting");
			sendModemCommand(cmd, 2000);
			SerialWifi.end();
			SerialWifi.begin(SERIAL_HIGH_SPEED);

			/* Aguarda o término do processo de reset */
			do
			{
				Serial.println("Waiting for reset..");
			}while( espCheckPresence(500, 10));

			/* Configura a nova velocidade */
			Serial.println("Setting the new baud rate");
			sendModemCommand("AT+CIOBAUD=" + String(SERIAL_LOW_SPEED)+ "\r\n", 1000, 2);
		}
		else

		{
			Serial.println("ESP8266 not found.");
			while(1);
		}
	}

	/* Finaliza a função sempre com o ESP disponível já na velocidade de trabalho*/
	Serial.println("Restarting Serial with SERIAL_LOW_SPEED");
	SerialWifi.end();
	SerialWifi.begin(SERIAL_LOW_SPEED);
}

static void wifiStartServer()
{
	/* Envia o comando que permite múltiplas conexões, configurado para uma */
	sendModemCommand("AT+CIPMUX=1\r\n", 1000);

	/* Envia o comando que inicializa o servidor */
	sendModemCommand("AT+CIPSERVER=1,80\r\n", 1000);
}


/* Inicializa as configurações de WiFi. */
static void wifiSetup()
{
	String response;
	/* Envia o comando de conectar ao WiFi */
	response = sendModemCommand("AT+CWJAP=\"" + SSID + "\",\"" + PW + "\"\r\n", 10000);

	/* Se foi possível conectar, mostra o IP */
	if(response.indexOf("OK") != -1)
	{
		sendModemCommand("AT+CIFSR\r\n", 1000);
	}

	/* Configura o ESP como WiFi Station */
	sendModemCommand("AT+CWMODE=1\r\n", 1000);
}

/* Responde a conexão com o servidor */
static void handleServerClient()
{
	if(SerialWifi.available() > 0)
	{
		/* Se receber o aviso de conexão, tratar */
		if(SerialWifi.find((char *) "+IPD,"))
		{
			Serial.println("Client connected");
			/* Recebe o identificador de conexão */
			int connId = SerialWifi.read() - '0';

			/* Processa os parâmetros recebidos */
			serverProccessClientCommand();

			/* Envia a página HTML dividida em partes */
			/* HTML HEAD */
			String page = F("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">");
			page += F("<title>WifiSampleLogger</title>");
			page += F("<style>body {text-align: center;}</style></head>");
			sendModemCommand("AT+CIPSEND=" + String(connId) + "," + page.length() + "\r\n", 300);
			SerialWifi.print(page);
			delay(250);

			/* HTML BODY */
			page = F("<body><p><h1>Local Station</h1></p>");
			page += F("<h2>Led Control</h2>");
			page += F("<div><a href=\"?led1=on\"><button type=\"button\" >Turn On</button></a></div>");
			sendModemCommand("AT+CIPSEND=" + String(connId) + "," + page.length() + "\r\n", 300);
			SerialWifi.print(page);
			delay(250);

			page = F("<div><a href=\"?led1=off\"><button type=\"button\">Turn Off</button></a></div>");
			page += F("<h2>Available data</h2><p><h3>Running time: ");
			page += String(millis());
			page += F(" ms</h3></p>");
			page += F("</body></html>\r\n");
			sendModemCommand("AT+CIPSEND=" + String(connId) + "," + page.length() + "\r\n", 300);
			SerialWifi.print(page);
			delay(250);

			sendModemCommand("AT+CIPSEND=" + String(connId) + "," + graph.length() + "\r\n", 300);
			SerialWifi.print(graph);

			sendModemCommand("AT+CIPCLOSE=" + String(connId) + "\r\n", 3000);
		}
	}
}



static void wifiSendData(uint32_t value)
{
	/* Prepara a requisição GET para o PushingBox com o valor desejado*/
	String data = "GET /pushingbox?devid=v701AFD5B48DFA68";
	data +="&value=" + String(value);
	data += " HTTP/1.1\r\nHost: api.pushingbox.com\r\n\r\n";

	/* Conecta ao servidor */
	sendModemCommand("AT+CIPSTART=4,\"TCP\",\"api.pushingbox.com\",80\r\n", 2000);

	/* Descreve o tamanho dos dados para envio */
	sendModemCommand("AT+CIPSEND=4," + String(data.length()) + "\r\n", 2000);
	delay(250);

	/* Envia os dados */
	SerialWifi.print(data);
	Serial.println("Data sent.");
}

static void serverProccessClientCommand()
{
	Serial.println("Checking commands");
	delay(100);
	String clientCmd = "";
	/* Descobre o parâmetros enviado através do HTTP GET */
	while(SerialWifi.available() > 0)
	{
		/* Lê a string recebida pela serial até encontrar um fim de linha.
		 * A requisição é enviada junto com o identificador de conexão
		 */
		String cmd = SerialWifi.readStringUntil('\n');
		Serial.println(cmd);

		/* Se encontrar uma requisição GET */
		if(cmd.indexOf("GET /?") != -1)
		{
			/* A requisição GET vem da seguinte forma:
			 * GET /?param=value HTTP1.1
			 * Se selecionarmos o subtexto que fica entre o ? e o HTTP
			 * temos exatamente o parâmetro
			 */
			clientCmd = cmd.substring(cmd.indexOf('?'), cmd.indexOf("HTTP"));
			Serial.println("Received command = "+ cmd);
			break;
		}
		delay(100);
	}

	/* Toma as ações dependendo dos parâmetros */
	if(clientCmd != "")
	{
		if(clientCmd.indexOf("led1=on") != -1 )
		{
			digitalWrite(13, HIGH);
		}
		if(clientCmd.indexOf("led1=off") != -1 )
		{
			digitalWrite(13, LOW);
		}

	}
}





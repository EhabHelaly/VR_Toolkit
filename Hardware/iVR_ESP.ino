#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUDP.h>

// Wifi 
const char* ssid = "iVRapp";
const char* pass = "vrproject"; 

// Packet data
char S[1024];
int length=0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
IPAddress serverIP(192,168,1,104);
unsigned int port = 80;


void setup(void)
{ 
	// Serial
	Serial.begin(38400);
	Serial.println("\niVR Gadget:");
	Serial.println(  "===========");
	// Wifi
	connect();
	// Udp Socket initialize
	Udp.begin(port);
	// Sync Serial
	Serial.flush();
	while(!(Serial.available() && Serial.read()=='\n'));
}

void loop()
{
	updateTarget();
	sendData();
}
void connect()
{
	WiFi.hostname("VRGadget1");
	WiFi.begin(ssid, pass);
	// Wait for connection
	while (!isConnected())
	{
		WiFi.begin(ssid, pass);
	}
}
int isConnected()
{
	int tries=0;
	Serial.print("\nConnecting ");
	while ( (WiFi.status() != WL_CONNECTED) && tries<20) 
	{
		delay(500);
		Serial.print(".");
		tries++;
	}
	if (WiFi.status() == WL_CONNECTED)
	{
		
	 	/*Serial.println("");
		Serial.print  ("Connected to: ");
		Serial.println(ssid);//*/
		Serial.print  ("IP address  : ");
		Serial.println(WiFi.localIP());			
		return 1;
	}
	return 0;
}
void updateTarget()
{
 	int bytes = Udp.parsePacket();  
 	if ( bytes ) 
 	{
		char R[1024];
		
		Serial.print  ("Packet Size       : ");
	    Serial.println(bytes);
	    Serial.print  ("Packet Source IP  : ");
	    Serial.println(Udp.remoteIP());
	    Serial.print  ("Packet Source Port: ");
	    Serial.println(Udp.remotePort());
	    
	    Udp.read(R,bytes); // read the packet into the buffer
	    
	    serverIP=Udp.remoteIP();
   }
}
void sendData()
{
    while(Serial.available())
    {
      S[length++]=Serial.read();
      if (S[length-1]=='\n' && length>=14)
      {
	    Udp.beginPacket(serverIP, port);
	    Udp.write(S+length-14,14);
	    Udp.endPacket();
        length=0;
      }
    }	
}

// Jens Oosterkamp
// 18-11-2023
// Code to run 8x8 LED matrix panel
// Uses 8xSM74hc595
// Uses LOLIN(WEMOS) D1 mini

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "Maakplek";
const char* password = "";
String newHostname = "PixelLamp";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "row";
const char* PARAM_INPUT_2 = "column";
const char* PARAM_INPUT_3 = "state";

// for ESP D1 mini clone
int latchPin = 4; //Pin connected to ST_CP of 74HC595
int clockPin = 5; //Pin connected to SH_CP of 74HC595
int dataPin = 0;////Pin connected to DS of 74HC595

// Hardware configuration
const int rows = 8;
const int columns = 8;
const int rowMap[rows] = {0,1,3,5,7,6,4,2}; // Mapping between shift registers and LED rows

bool pixelBuffer [rows][columns];

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>

  <head>
    <title>Maakplek Pixel Lamp</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
	
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      .button {
	  background-color: #195B6A; border: none;  color: white; padding: 10px 10px;

      }  
      .button:hover {background-color: #1f2e45}
      .button:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
	  .newspaper {
		-webkit-column-count: 8; /* Chrome, Safari, Opera */
		-moz-column-count: 8; /* Firefox */
		column-count: 8;
		width: 14%;
		margin: auto;
		padding: 10px 10px;
	  }
    </style>
  </head>
  <body>
    <h1>Maakplek Pixel Lamp</h1>
	<div><a href="https://github.com/JensMakerAdventures/MaakplekPixelLamp" target="_blank">Link to the GitHub project!</a></div>
	<br>
	<body onload="createCheckBoxes();">
    <button class="button" onmousedown="allOn();">Turn all LEDS on</button>
	<button class="button" onmousedown="allOff();">Turn all LEDS off</button>
  <script>
   function allOn()
   {
     let checkboxes = document.querySelectorAll('input[name="gridBoxes"]');
     checkboxes.forEach(box => {
        box.checked = true;
      });
	 var xhr = new XMLHttpRequest();
     xhr.open("GET", "/allOn", true);
     xhr.send();
   }

   function allOff()
   {
     let checkboxes = document.querySelectorAll('input[name="gridBoxes"]');
     checkboxes.forEach(box => {
        box.checked = false;
      });
	 var xhr = new XMLHttpRequest();
     xhr.open("GET", "/allOff", true);
     xhr.send();
   }
   
   function createCheckBoxes() 
   {
     for (let row = 0; row < 8; row++) 
	 {
	  var div = document.createElement("div");
	  div.width= "14%";
	  div.margin="auto";
	  div.style.columnCount="8";
	  document.body.appendChild(div);
	  for (let col = 0; col < 8; col++) 
	  {
	    var x = document.createElement("INPUT");
		x.setAttribute("name", "gridBoxes");
	    x.setAttribute("type", "checkbox");
		x.setAttribute("row", row.toString());
		x.setAttribute("column", col.toString());
		x.setAttribute("state", "1");
		x.setAttribute("onmousedown","toggleCheckbox(this)");
		//x.setAttribute("checked", false); // Can be used to initialize state from hardware
	    document.body.appendChild(x);
	  }
	 }
   }

   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
	 var fatLink = "/update?row="+x.getAttribute("row")+"&column=" + x.getAttribute("column") + "&state="+!x.checked;
     xhr.open("GET", fatLink, true);
     xhr.send();
   }
  </script>
  </body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Error 404.");
}

void setPixel(int row, int col, bool state)
{
  pixelBuffer[row][col] = state;
}

void fillScreen() // sets all pixels to 1 (= on)
{
  for (int x = 0; x < 8; x++) 
  {
    for(int y = 0; y < 8; y++)
    {
      pixelBuffer[x][y] = 1;
    }
  }
}

void clearScreen() // sets all pixels to 0 (= off)
{
  for (int x = 0; x < 8; x++) 
  {
    for(int y = 0; y < 8; y++)
    {
      pixelBuffer[x][y] = 0;
    }
  }
}

void setup() 
{
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
  clearScreen();

  // Connect to Wi-Fi
  WiFi.hostname(newHostname.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Receive an HTTP GET request
  server.on("/allOn", HTTP_GET, [] (AsyncWebServerRequest *request) {

    fillScreen();
    request->send(200, "text/plain", "ok");
  });

  // Receive an HTTP GET request
  server.on("/allOff", HTTP_GET, [] (AsyncWebServerRequest *request) {
    clearScreen();
    request->send(200, "text/plain", "ok");
  });


  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    String inputMessage1;
    String inputMessage2;
    String inputMessage3;
    // GET input1 value on <ESP_IP>/update?row=<param1>column=<param2.state=<param3>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2) && request->hasParam(PARAM_INPUT_3)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      Serial.println(inputMessage1);
      Serial.println(inputMessage2);
      Serial.println(inputMessage3);
      if(inputMessage3=="true")
      {
        setPixel(inputMessage1.toInt(), inputMessage2.toInt(), 1);
      }
      else
      {
        setPixel(inputMessage1.toInt(), inputMessage2.toInt(), 0);
      }
      
    }
    request->send(200, "text/plain", "OK");
  });
  
  server.onNotFound(notFound);
  server.begin();
}

byte arrayToByte(bool pixelBuffer[8][8], int row)
{
  byte result = 0;
  for(int i=0; i<columns; i++)
  {
    if(pixelBuffer[row][i])
    {
      result |= (1 << i);
    }
  }
  return result;
} 

void writeScreen()
{
  digitalWrite(latchPin, LOW);
  for(int row = 0; row < rows; row++)
  {
    // A zero means LED turns on, so all bytes are inverted here
    shiftOut(dataPin, clockPin, MSBFIRST, ~arrayToByte(pixelBuffer,rowMap[row]));
  }
  digitalWrite(latchPin, HIGH);
}

void togglePixel(int row, int col)
{
  if(pixelBuffer[row][col] == true)
  {
    pixelBuffer[row][col] = false;
  }
  else
  {
    pixelBuffer[row][col] = true;
  }
}





void setRow(int row, bool state)
{
  for (int i = 0; i < 8; i++)
  {
    pixelBuffer[row][i]=state;
  }
}

void setColumn(int col, bool state)
{
    for (int i = 0; i < 8; i++)
  {
    pixelBuffer[i][col]=state;
  }
}

void loop() 
{
  writeScreen();
  delay(50);
}
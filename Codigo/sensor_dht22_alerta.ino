#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <DHT.h>

// -----------------------------------------------------------------
// Credenciales: NO subas tu WiFi ni tu webhook reales al repo.
// Define estos valores en un archivo local "secrets.h" (ignorado
// por git) con este contenido:
//
//   #define WIFI_SSID     "tu_red_2.4GHz"
//   #define WIFI_PASSWORD "tu_password"
//   #define WEBHOOK_PATH  "/tu-path-secreto-de-make"
//
#include "secrets.h"
// -----------------------------------------------------------------

#define DHTPIN 2
#define DHTTYPE DHT22   // tu sensor es DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = WIFI_SSID;       // debe ser red de 2.4 GHz
const char* password = WIFI_PASSWORD;
const char* host = "hook.us2.make.com";
const char* webhookPath = WEBHOOK_PATH;

const int ledPin = 13;
const float UMBRAL_TEMP = 24.0; // grados Celsius (24 para probar; regresalo a 28 despues)

bool estadoCaliente = false;
float ultimaTemp = 0;
float ultimaHumedad = 0;
unsigned long ultimaLectura = 0;
const unsigned long INTERVALO_LECTURA = 2000;

WiFiServer server(80);
WiFiSSLClient clientSSL;   // conexion segura (https, puerto 443)
WiFiClient clientHTTP;     // conexion normal (http, puerto 80) de respaldo

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // Esperar a que el router le asigne IP
  while (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(500);
    Serial.print("*");
  }
  delay(2000);

  Serial.println();
  Serial.print("Servidor listo en: http://");
  Serial.println(WiFi.localIP());
  server.begin();

  Serial.println("--- Diagnostico de conexion ---");
  Serial.print("Firmware del modulo WiFi: ");
  Serial.println(WiFi.firmwareVersion());

  IPAddress ipMake;
  if (WiFi.hostByName(host, ipMake)) {
    Serial.print("DNS OK: ");
    Serial.println(ipMake);
  } else {
    Serial.println("DNS FALLO");
  }
  Serial.println("-------------------------------");
}

void loop() {
  if (millis() - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = millis();
    leerYEvaluar();
  }

  WiFiClient webClient = server.available();
  if (webClient) {
    while (webClient.connected() && webClient.available()) {
      char c = webClient.read();
      if (c == '\n') break;
    }
    enviarPagina(webClient);
    webClient.stop();
  }
}

void leerYEvaluar() {
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error al leer el sensor DHT22");
    return;
  }

  ultimaTemp = temperatura;
  ultimaHumedad = humedad;

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" *C");

  bool estaCalienteAhora = (temperatura >= UMBRAL_TEMP);
  digitalWrite(ledPin, estaCalienteAhora ? HIGH : LOW);

  if (estaCalienteAhora && !estadoCaliente) {
    enviarAlertaMake(temperatura);
  }

  estadoCaliente = estaCalienteAhora;
}

void enviarPagina(WiFiClient &webClient) {
  webClient.println("HTTP/1.1 200 OK");
  webClient.println("Content-Type: text/html");
  webClient.println("Connection: close");
  webClient.println();
  webClient.println("<!DOCTYPE html><html><head>");
  webClient.println("<meta name='viewport' content=");
  webClient.println("'width=device-width, initial-scale=1'>");
  webClient.println("<meta http-equiv='refresh' content='5'>");
  webClient.println("<style>");
  webClient.println("body{font-family:sans-serif;text-align:center;}");
  webClient.println("body{margin-top:50px;}");
  webClient.println("h1{font-size:28px;}");
  webClient.println(".valor{font-size:48px;font-weight:bold;}");
  webClient.println("</style></head><body>");
  webClient.println("<h1>Monitor del cuarto</h1>");
  webClient.print("<p>Temperatura</p><p class='valor'>");
  webClient.print(ultimaTemp);
  webClient.println(" &deg;C</p>");
  webClient.print("<p>Humedad</p><p class='valor'>");
  webClient.print(ultimaHumedad);
  webClient.println(" %</p>");
  webClient.print("<p>Estado: <b>");
  webClient.print(estadoCaliente ? "CALIENTE (LED encendido)" : "Normal");
  webClient.println("</b></p>");
  webClient.println("</body></html>");
}

// Manda el POST por la conexion que se le pase y muestra la respuesta de Make
bool mandarPOST(Client &c, const char* tipo, float temp) {
  String jsonPayload = "{\"temperatura\":" + String(temp) + "}";

  c.println("POST " + String(webhookPath) + " HTTP/1.1");
  c.println("Host: " + String(host));
  c.println("Content-Type: application/json");
  c.println("Content-Length: " + String(jsonPayload.length()));
  c.println("Connection: close");
  c.println();
  c.print(jsonPayload);

  unsigned long inicio = millis();
  while (c.connected() && !c.available() && millis() - inicio < 8000) {
    delay(10);
  }

  bool ok = false;
  if (c.available()) {
    String respuesta = c.readStringUntil('\n');
    Serial.print("Respuesta de Make (");
    Serial.print(tipo);
    Serial.print("): ");
    Serial.println(respuesta);   // debe decir: HTTP/1.1 200 OK
    ok = respuesta.indexOf("200") > 0;
  } else {
    Serial.print("Make no respondio por ");
    Serial.println(tipo);
  }

  c.stop();
  return ok;
}

void enviarAlertaMake(float temp) {
  Serial.println("Enviando alerta a Make por HTTPS...");
  if (clientSSL.connect(host, 443)) {
    if (mandarPOST(clientSSL, "HTTPS", temp)) return;
  } else {
    Serial.println("HTTPS no conecto");
  }

  Serial.println("Intentando por HTTP (respaldo)...");
  if (clientHTTP.connect(host, 80)) {
    mandarPOST(clientHTTP, "HTTP", temp);
  } else {
    Serial.println("HTTP tampoco conecto");
  }
}

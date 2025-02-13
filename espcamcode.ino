#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

//const char* WIFI_SSID = "Airtel_kall_2649";
//const char* WIFI_PASS = "air40597";

const char* WIFI_SSID = "moto";
const char* WIFI_PASS = "123456789";


//const char* WIFI_SSID = "M";
//const char* WIFI_PASS = "123456789";


WebServer server(80);

static auto hiRes = esp32cam::Resolution::find(800, 600);

void streamMjpeg() {
  WiFiClient client = server.client();
  String response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Connection: keep-alive\r\n\r\n";
  client.print(response);

  while (client.connected()) {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      Serial.println("CAPTURE FAIL");
      break;
    }
    
    String header = "--frame\r\n"
                    "Content-Type: image/jpeg\r\n"
                    "Content-Length: " + String(frame->size()) + "\r\n\r\n";
    client.print(header);
    frame->writeTo(client);
    client.print("\r\n");

    frame.reset();  // Properly releases the frame buffer

    // Optional delay to control frame rate (in milliseconds)
    delay(100);  
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println();
  
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /stream");

  server.on("/stream", streamMjpeg);
  server.begin();
}

void loop() {
  server.handleClient();
}

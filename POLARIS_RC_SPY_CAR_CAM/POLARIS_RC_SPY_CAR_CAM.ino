#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <HardwareSerial.h>
#include "esp_http_server.h"
#include "fb_gfx.h"
#include "camera_index.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#define CAMERA_MODEL_AI_THINKER
#define FLASH_LED 4
#define  ONBOARD_LED   2
#define HORN        14
#include "camera_pins.h"

HardwareSerial espSerial(1);

String receivedData = ""; // To store the received characters



const char* ssid = "POLARIS_RC_SPY_CAR";
const char* password = "TLS1985OP";
char direction   = 'X';
float battery    =  0;

int debugS = 0;
int speedValue = 0;

using namespace websockets;
WebsocketsServer socket_server;

camera_fb_t * fb = NULL;
httpd_handle_t camera_httpd = NULL;
static esp_err_t index_handler(httpd_req_t *req);
void app_httpserver_init ();
void handle_message(WebsocketsClient &client, WebsocketsMessage msg);

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  return httpd_resp_send(req, (const char *)indexHtml,indexHtml_len);
}

httpd_uri_t index_uri = {
  .uri       = "/",
  .method    = HTTP_GET,
  .handler   = index_handler,
  .user_ctx  = NULL
};

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  WiFi.setSleep(false);

  Serial.begin(115200);
  espSerial.begin(115200, SERIAL_8N1, 13, 12); // Initialize Serial1 with TX on pin 12, RX on pin 13
  Serial.println();
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(FLASH_LED,OUTPUT);
  pinMode(HORN,OUTPUT);

  digitalWrite(ONBOARD_LED, LOW);
  digitalWrite(FLASH_LED, LOW);
  digitalWrite(HORN, LOW);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }


  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);


  // WiFi.begin(ssid, password);
  // WiFi.setSleep(false);

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println("");
  // Serial.println("WiFi connected");
  // Serial.print("Camera Ready! Use 'http://");
  // Serial.print(WiFi.localIP());
  // Serial.println("' to connect");

  
  WiFi.softAP(ssid,password);

  app_httpserver_init();
  socket_server.listen(82);

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("' to connect");

 
  delay(10);
}



void loop() {
  auto client = socket_server.accept();
  client.onMessage(handle_message);
  client.send("STREAMING");
  String bat;
  while (client.available()) {
    client.poll();   
    fb = esp_camera_fb_get();
    client.sendBinary((const char *)fb->buf, fb->len);
    if (espSerial.available()) {
    String receivedData = espSerial.readStringUntil('\n'); // Read until newline character
    
    // Find the separator
    int separatorIndex = receivedData.indexOf(',');
    if (separatorIndex != -1) {
      String floatStr = receivedData.substring(0, separatorIndex); // Extract float part
      String charStr = receivedData.substring(separatorIndex + 1); // Extract character part
      
      float battery = floatStr.toFloat(); // Convert float part to float
      char status = charStr.charAt(0); // Get the first character as char
      bat = "BAT: " + String(battery);   

      // Serial.print("Received float: ");
      // Serial.print(battery);
      
      // Serial.print(" Received character: ");
      // Serial.println(status);
    }
  }
    client.send(bat.c_str());
    esp_camera_fb_return(fb);
    fb = NULL;
  }
}





void app_httpserver_init ()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
    Serial.println("httpd_start");
  {
    httpd_register_uri_handler(camera_httpd, &index_uri);
  }
}

void handle_message(WebsocketsClient &client, WebsocketsMessage msg)
{
  if (msg.data() == "LIGHTON") 
  { 
    digitalWrite(FLASH_LED,HIGH);
  }
  if (msg.data() == "LIGHTOFF") 
  { 
    digitalWrite(FLASH_LED,LOW);
  } 
  if(msg.data() == "HON")
  {
    digitalWrite(HORN,HIGH);
  }
  if(msg.data() == "HOFF")
  {
    digitalWrite(HORN,LOW);
  }
  
}





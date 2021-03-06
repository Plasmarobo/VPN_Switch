#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
//NEEDS github.com/schinken/SimpleTimer
#include <SimpleTimer.h>
//#define DEBUG
#ifdef DEBUG
char dbg_str[128];
  #define LOG(x) {mqtt_log(x);Serial.println(x);}
#else
  #define LOG(x) do{}while(0)
#endif

SimpleTimer timer;

void mqtt_callback(char *topic, uint8_t* payload, uint32_t len);

//Hardware Setup
#define SWITCH_PIN 5
#define SW_OFF 0
#define SW_ON 1
#define POWER_SW_PIN 4
#define PWR_OFF 0
#define PWR_ON 1
#define RED_PIN 12
#define GREEN_PIN 13
#define BLUE_PIN 14
uint8_t switch_state;
uint8_t power_sw_state;
bool debounce_sw=false;
bool debounce_pwr=false;
//Display
const uint8_t COLOR_RED[3] = {30,0,0};
const uint8_t COLOR_GREEN[3] = {0,30,0};
const uint8_t COLOR_BLUE[3] = {0,0,30};
const uint8_t COLOR_ORANGE[3] = {30,14,0};
const uint8_t COLOR_BLACK[3] = {0,0,0};

uint8_t color[3];
uint8_t next_color[3];
uint32_t animation_time;
#define PATTERN_FAST 500
#define PATTERN_SLOW 3000
uint16_t animation_rate;
uint8_t animation_state;
uint8_t animation_select;
#define ANIM_STATE_SOLID 0
#define ANIM_STATE_BLINK 1
#define ANIM_STATE_BREATHE 2

void setup_pins() {
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(POWER_SW_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void set_color(const uint8_t c[3]) {
  analogWrite(RED_PIN, c[0]);
  analogWrite(GREEN_PIN, c[1]);
  analogWrite(BLUE_PIN, c[2]);
}

void update_display() {
  uint32_t anim_t = millis() - animation_time;
  uint16_t coeff;
  //Serial.print("T ");
  //Serial.println(anim_t);
  if (power_sw_state == PWR_OFF) {
    set_color(COLOR_BLACK);
    return;
  }
  if (anim_t > animation_rate) {
    animation_time = millis();
    ++animation_state;
  }
  switch(animation_select) {
    case ANIM_STATE_SOLID:
      set_color(color);
      animation_state = 0;
      break;
    case ANIM_STATE_BLINK:
      if (animation_state == 0) {
        set_color(COLOR_BLACK);
      } if (animation_state == 1) {
        set_color(color);
      } else {
        animation_state = 0;
      }
      break;
    case ANIM_STATE_BREATHE:
      switch(animation_state) {
        case 0:
          coeff = 16 + 239 * sin((PI / 2)  * (float)anim_t / (float)animation_rate);
          next_color[0] = ((uint16_t)color[0] * coeff) >> 8;
          next_color[1] = ((uint16_t)color[1] * coeff) >> 8;
          next_color[2] = ((uint16_t)color[2] * coeff) >> 8;
          set_color(next_color);
          break;
        case 1:
          set_color(color);
          break;
        case 2:
          coeff = 255 - 239 * sin((PI / 2)  * (float)anim_t / (float)animation_rate);
          next_color[0] = ((uint16_t)color[0] * coeff) >> 8;
          next_color[1] = ((uint16_t)color[1] * coeff) >> 8;
          next_color[2] = ((uint16_t)color[2] * coeff) >> 8;
          set_color(next_color);
          break;
        case 3:
          next_color[0] = (color[0] * 16) >> 8;
          next_color[1] = (color[1] * 16) >> 8;
          next_color[2] = (color[2] * 16) >> 8;
          set_color(next_color);
          break;
        case 4:
          animation_state = 0;
          animation_time = millis();
          break;
        default:
          break;
      } 
      break;
    default:
      break;
  }
}

//WATCHDOG SETUP
#define MAX_ERRORS 3
#define DELAY 3000

void check_watchdog(uint8_t &errors) {
  if (errors > MAX_ERRORS) {
    LOG("!!");
    LOG("Watchdog: Too many Errors, rebooting...");
    ESP.restart();
  }
}

void pet_watchdog() {
  uint8_t error_count = 0;
  while (!wifi_connect()) {
    ++error_count;
    check_watchdog(error_count);
    LOG("WiFi failure");
    delay(DELAY);
  }
  while (!mqtt_connect()){
    ++error_count;
    check_watchdog(error_count);
    LOG("MQTT failure");
    delay(DELAY);
  }
}

//WIFI SETUP
const String ssid = "";//SET THIS
const String password = "";//SET THIS
WiFiClient wifi;

bool wifi_connect() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    return true;
  }
  return false;
}

//MQTT SETUP
#define MQTT_SERVER "192.168.2.1"
#define MQTT_SERVERPORT 1883
const char* MQTT_STATUS = "vpn/status";
const char* MQTT_CONTROL = "vpn/control";
const char* MQTT_LOG = "vpn/log";

PubSubClient client(MQTT_SERVER, MQTT_SERVERPORT, mqtt_callback, wifi);

bool mqtt_connect() {
  if (client.connected())
  {
    return true;
  }
  if (client.connect("VPNSwitch")){
    client.subscribe(MQTT_STATUS);
    return true;
  }
  return false;
}

void mqtt_log(String payload) {
  if (client.connected()) {
      client.publish(MQTT_LOG, (char*) payload.c_str());
    }
}

bool is_string(const char* t, const char *tt){
  return strcmp(t, tt) == 0; 
}

//HARDWARE setup
void handle_switch() {
  if(!debounce_sw) {
    if (switch_state != digitalRead(SWITCH_PIN)) {
      debounce_sw = true;
      timer.setTimeout(15, [] () {
        debounce_sw = false;
        switch_state = digitalRead(SWITCH_PIN);
        String payload;
        if (switch_state == SW_ON) payload = "ON";
        else if (switch_state == SW_OFF) payload = "OFF";
        LOG(payload);
        if (client.connected()) {
          client.publish(MQTT_CONTROL, (char*) payload.c_str(), true);
          memcpy(color, COLOR_BLUE, sizeof(color));
          animation_select = ANIM_STATE_BLINK;
        }
      });
    }
  }
}

void handle_power() {
  if(!debounce_pwr) {
    if (power_sw_state != digitalRead(POWER_SW_PIN)) {
      debounce_pwr = true;
      timer.setTimeout(15, [](){ power_sw_state = digitalRead(POWER_SW_PIN); debounce_pwr = false;});
    }
  }
}

void mqtt_callback(char *topic, uint8_t* payload, uint32_t len) {
  Serial.print("MQTT: ");
  Serial.println(topic);
  uint8_t *safe_buffer = new uint8_t[len+1];
  memcpy(safe_buffer, payload, len);
  safe_buffer[len] = '\0';
  if (is_string(topic, MQTT_STATUS)) {
    Serial.println((char*)safe_buffer);
    if (is_string((char*)safe_buffer, "ON")) {
      //Check against switch status
      animation_rate = PATTERN_SLOW;
      if(switch_state == SW_OFF) {
        memcpy(color, COLOR_ORANGE, sizeof(color));
        animation_select = ANIM_STATE_BLINK;
      } else {
        memcpy(color, COLOR_GREEN, sizeof(color));
        animation_select = ANIM_STATE_BREATHE;
      }
    } else if(is_string((char*)safe_buffer, "OFF")) {
      animation_rate = PATTERN_FAST;
      if(switch_state == SW_ON) {
        memcpy(color,COLOR_RED, sizeof(color));
        animation_select = ANIM_STATE_BLINK;
      } else {
        memcpy(color, COLOR_RED, sizeof(color));
        animation_select = ANIM_STATE_BREATHE;
      }
    }
  }
}

//OTA setup
void setup_ota() {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("VPNSwitch");

  // No authentication by default
  ArduinoOTA.setPassword((const char *)"gravity");

  ArduinoOTA.onStart([]() {
    LOG("Start");
  });
  ArduinoOTA.onEnd([]() {
    LOG("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {LOG("OTA Auth Failed");}
    else if (error == OTA_BEGIN_ERROR) {LOG("OTA Begin Failed");}
    else if (error == OTA_CONNECT_ERROR) {LOG("OTA Connect Failed");}
    else if (error == OTA_RECEIVE_ERROR) {LOG("OTA Receive Failed");}
    else if (error == OTA_END_ERROR) {LOG("OTA End Failed");}
  });
  ArduinoOTA.begin();
}
  
//==ENTRY==
void connect_all() {
  uint8_t error_count = 0;
  LOG("Connecting Wifi...");
  while (!wifi_connect()) {
    ++error_count;
    check_watchdog(error_count);
    LOG("X");
    delay(DELAY);
  }
  LOG("Ok");
  #ifdef DEBUG
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
  LOG("Connecting MQTT...");
  while (!mqtt_connect()){
    ++error_count;
    check_watchdog(error_count);
    LOG("X");
    delay(DELAY);
  }
  LOG("Ok");
}

void setup() {
  Serial.begin(115200);
  LOG("Boot");
  setup_pins();
  analogWrite(RED_PIN, 128);
  delay(1000);
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 128);
  delay(1000);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 128);
  delay(1000);
  analogWrite(BLUE_PIN, 0);
  delay(500);
  set_color(COLOR_ORANGE);
  delay(500);
  memcpy(color, COLOR_BLUE, sizeof(color));
  LOG("Start fast");
  animation_rate = PATTERN_FAST;
  animation_select = ANIM_STATE_BLINK;
  update_display();
  connect_all();
  setup_ota();
  handle_switch();
  handle_power();
  LOG("System Up");
  #ifdef DEBUG
    timer.setInterval(3000, log_state);
  #endif
}

void log_state() {
  char buf[128];
  sprintf(buf, "Sw %d\n", switch_state);
  LOG(buf);
  //sprintf(buf, "Pw %d\n", power_sw_state);
  //LOG(buf);
  //sprintf(buf, "Anim %d\n", animation_select);
  //LOG(buf);
  //sprintf(buf, "Color #%02x%02x%02x\n", color[0], color[1], color[2]);
  //LOG(buf);
}

void loop() {
  ArduinoOTA.handle();
  timer.run();
  pet_watchdog();
  handle_switch();
  handle_power();
  client.loop();
  update_display();
}


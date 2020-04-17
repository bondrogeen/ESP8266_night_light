#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WS2812FX.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define MQTT_SERVER ""
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_PORT 1883
#define LED_PIN 2 // D2 on ESP8266
#define LED_COUNT 25
#define NEO_PIXEL_TYPE NEO_GRB + NEO_KHZ800
#define DEFAULT_EFFECT FX_MODE_RAINBOW_CYCLE
#define DEFAULT_POWER_ON_STATE true

WiFiClient espClient;
PubSubClient client(espClient);
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_PIXEL_TYPE);

uint8_t red = 0;
uint8_t green = 0;
uint8_t blue = 255;
uint8_t brightness = 127;         // 50%

uint16_t speed = 50;
uint16_t reset_counter = 0;
uint32_t last_time = 0;

boolean wifi_status = false;
boolean state = DEFAULT_POWER_ON_STATE;

const char *will_msg = "Offline";
const char *online_msg = "Online";
const char PREFIX_TOPIC[] = "ESP/";
const char TOPIC_SPEED[] = "/light/speed";
const char TOPIC_EFFECT[] = "/light/effect";
const char TOPIC_BRIGHTNESS[] = "/light/brightness";
const char TOPIC_COLOR[] = "/light/color";
const char TOPIC_STATE[] = "/light/state";
const char TOPIC_LWT[] = "/light/LWT";

char chip_id[7];
uint16_t count = sprintf(chip_id, "%06X", ESP.getChipId());
const size_t chip_id_size = sizeof(chip_id);
char prefix_topic[chip_id_size + sizeof(PREFIX_TOPIC) ];
const size_t prefix_topic_size = sizeof(prefix_topic);

char topic_speed[prefix_topic_size + sizeof(TOPIC_SPEED)];
char topic_effect[prefix_topic_size + sizeof(TOPIC_EFFECT)];
char topic_brightness[prefix_topic_size + sizeof(TOPIC_BRIGHTNESS)];
char topic_color[prefix_topic_size + sizeof(TOPIC_COLOR)];
char topic_state[prefix_topic_size + sizeof(TOPIC_STATE)];
char topic_lwt[prefix_topic_size + sizeof(TOPIC_BRIGHTNESS)];

void generateMqttTopics() {
  strcpy(prefix_topic, PREFIX_TOPIC);
  strcat(prefix_topic, chip_id);

  strcpy(topic_speed, prefix_topic);
  strcat(topic_speed, TOPIC_SPEED);

  strcpy(topic_effect, prefix_topic);
  strcat(topic_effect, TOPIC_EFFECT);

  strcpy(topic_brightness, prefix_topic);
  strcat(topic_brightness, TOPIC_BRIGHTNESS);

  strcpy(topic_lwt, prefix_topic);
  strcat(topic_lwt, TOPIC_LWT);

  strcpy(topic_color, prefix_topic);
  strcat(topic_color, TOPIC_COLOR);

  strcpy(topic_state, prefix_topic);
  strcat(topic_state, TOPIC_STATE);
}

uint32_t rgbToHex(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

uint32_t strToHex(String str) {
  return (uint32_t) strtol( &str[1], NULL, 16);
}

bool isTrue(char *value) {
  return (!strcmp(value, "on") || !strcmp(value, "1") || !strcmp(value, "true"));
}

void setEffect(String effect) {
  if (effect == "static")
    ws2812fx.setMode(FX_MODE_STATIC);
  if (effect == "blink")
    ws2812fx.setMode(FX_MODE_BLINK);
  if (effect == "breath")
    ws2812fx.setMode(FX_MODE_BREATH);
  if (effect == "color wipe")
    ws2812fx.setMode(FX_MODE_COLOR_WIPE);
  if (effect == "color wipe inverted")
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_INV);
  if (effect == "color wipe reverse")
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_REV);
  if (effect == "color wipe reverse inverted")
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_REV_INV);
  if (effect == "color wipe random")
    ws2812fx.setMode(FX_MODE_COLOR_WIPE_RANDOM);
  if (effect == "random color")
    ws2812fx.setMode(FX_MODE_RANDOM_COLOR);
  if (effect == "single dynamic")
    ws2812fx.setMode(FX_MODE_SINGLE_DYNAMIC);
  if (effect == "multi dynamic")
    ws2812fx.setMode(FX_MODE_MULTI_DYNAMIC);
  if (effect == "rainbow")
    ws2812fx.setMode(FX_MODE_RAINBOW);
  if (effect == "rainbow cycle")
    ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  if (effect == "scan")
    ws2812fx.setMode(FX_MODE_SCAN);
  if (effect == "dual scan")
    ws2812fx.setMode(FX_MODE_DUAL_SCAN);
  if (effect == "fade")
    ws2812fx.setMode(FX_MODE_FADE);
  if (effect == "theater chase")
    ws2812fx.setMode(FX_MODE_THEATER_CHASE);
  if (effect == "theater chase rainbow")
    ws2812fx.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
  if (effect == "running lights")
    ws2812fx.setMode(FX_MODE_RUNNING_LIGHTS);
  if (effect == "twinkle")
    ws2812fx.setMode(FX_MODE_TWINKLE);
  if (effect == "twinkle random")
    ws2812fx.setMode(FX_MODE_TWINKLE_RANDOM);
  if (effect == "twinkle fade")
    ws2812fx.setMode(FX_MODE_TWINKLE_FADE);
  if (effect == "twinkle fade random")
    ws2812fx.setMode(FX_MODE_TWINKLE_FADE_RANDOM);
  if (effect == "sparkle")
    ws2812fx.setMode(FX_MODE_SPARKLE);
  if (effect == "flash sparkle")
    ws2812fx.setMode(FX_MODE_FLASH_SPARKLE);
  if (effect == "hyper sparkle")
    ws2812fx.setMode(FX_MODE_HYPER_SPARKLE);
  if (effect == "strobe")
    ws2812fx.setMode(FX_MODE_STROBE);
  if (effect == "strobe rainbow")
    ws2812fx.setMode(FX_MODE_STROBE_RAINBOW);
  if (effect == "multi strobe")
    ws2812fx.setMode(FX_MODE_MULTI_STROBE);
  if (effect == "blink rainbow")
    ws2812fx.setMode(FX_MODE_BLINK_RAINBOW);
  if (effect == "chase white")
    ws2812fx.setMode(FX_MODE_CHASE_WHITE);
  if (effect == "chase color")
    ws2812fx.setMode(FX_MODE_CHASE_COLOR);
  if (effect == "chase random")
    ws2812fx.setMode(FX_MODE_CHASE_RANDOM);
  if (effect == "chase rainbow")
    ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
  if (effect == "chase flash")
    ws2812fx.setMode(FX_MODE_CHASE_FLASH);
  if (effect == "chase random")
    ws2812fx.setMode(FX_MODE_CHASE_FLASH_RANDOM);
  if (effect == "chase rainbow white")
    ws2812fx.setMode(FX_MODE_CHASE_RAINBOW_WHITE);
  if (effect == "chase blackout")
    ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT);
  if (effect == "chase blackout rainbow")
    ws2812fx.setMode(FX_MODE_CHASE_BLACKOUT_RAINBOW);
  if (effect == "color sweep random")
    ws2812fx.setMode(FX_MODE_COLOR_SWEEP_RANDOM);
  if (effect == "running color")
    ws2812fx.setMode(FX_MODE_RUNNING_COLOR);
  if (effect == "running red blue")
    ws2812fx.setMode(FX_MODE_RUNNING_RED_BLUE);
  if (effect == "running random")
    ws2812fx.setMode(FX_MODE_RUNNING_RANDOM);
  if (effect == "larson scanner")
    ws2812fx.setMode(FX_MODE_LARSON_SCANNER);
  if (effect == "comet")
    ws2812fx.setMode(FX_MODE_COMET);
  if (effect == "fireworks")
    ws2812fx.setMode(FX_MODE_FIREWORKS);
  if (effect == "fireworks random")
    ws2812fx.setMode(FX_MODE_FIREWORKS_RANDOM);
  if (effect == "merry christmas")
    ws2812fx.setMode(FX_MODE_MERRY_CHRISTMAS);
  if (effect == "fire flicker")
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
  if (effect == "fire flicker soft")
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER_SOFT);
  if (effect == "fire flicker intense")
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER_INTENSE);
  if (effect == "circus combustus")
    ws2812fx.setMode(FX_MODE_CIRCUS_COMBUSTUS);
  if (effect == "halloween")
    ws2812fx.setMode(FX_MODE_HALLOWEEN);
  if (effect == "bicolor chase")
    ws2812fx.setMode(FX_MODE_BICOLOR_CHASE);
  if (effect == "tricolor chase")
    ws2812fx.setMode(FX_MODE_TRICOLOR_CHASE);
  if (effect == "icu")
    ws2812fx.setMode(FX_MODE_ICU);
}


void process(char *topic, char *message) {
  if (!strcmp(topic, topic_effect)) setEffect(String(message));
  if (!strcmp(topic, topic_speed)) ws2812fx.setSpeed(String(message).toInt());
  if (!strcmp(topic, topic_brightness)) ws2812fx.setBrightness(String(message).toInt());
  if (!strcmp(topic, topic_color)) ws2812fx.setColor(strToHex(String(message)));

  if (!strcmp(topic, topic_state)) {
    if (isTrue(message)) {
      if (!ws2812fx.isRunning()) ws2812fx.start();
    } else {
      ws2812fx.stop();
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(message);

  process(topic, message);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  generateMqttTopics();

  ws2812fx.init();
  ws2812fx.setBrightness(brightness);
  ws2812fx.setColor(rgbToHex(red, green, blue));
  ws2812fx.setSpeed(speed * 100);
  ws2812fx.setMode(DEFAULT_EFFECT);
  ws2812fx.start();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(chip_id);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  Serial.println("Ready");
}

void loop() {
  ws2812fx.service();
  uint32_t now = millis();

  if (WiFi.status() != WL_CONNECTED ) {
    if (now - last_time > 1000) {
      last_time = now;
      wifi_status = true;
      reset_counter++;
      if (reset_counter > 10) {
        Serial.println("");
        Serial.println("Restart");
        delay(1000);
        ESP.restart();
      }
      Serial.print(".");
    }
  } else {
    if (wifi_status) {
      wifi_status = false;
      Serial.println("");
      Serial.print("Connecting to SSID: ");
      Serial.println(WIFI_SSID);
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }
    if (!client.connected()) {
      if (now - last_time > 5000) {
        last_time = now;
        Serial.print("Attempting MQTT connection...");
        if (client.connect(chip_id, MQTT_USER, MQTT_PASSWORD, topic_lwt, 2, 1, will_msg)) {
          Serial.println("connected");
          client.publish(topic_lwt, online_msg, 1); // Send last will message
          client.publish(topic_brightness, "");
          client.publish(topic_effect, "");
          client.publish(topic_speed, "");
          client.publish(topic_color, "");
          client.publish(topic_state, "");
          client.subscribe(topic_brightness);
          client.subscribe(topic_effect);
          client.subscribe(topic_speed);
          client.subscribe(topic_color);
          client.subscribe(topic_state);
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
        }
      }
    } else {
      client.loop(); // Check MQTT
    }
  }
}

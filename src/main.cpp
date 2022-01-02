/**************************************************************************************/
/*  "MrDiy Audio Notifier"                                                            */
/*  Credits to original source : https://gitlab.com/MrDIYca/mrdiy-audio-notifier      */
/*  Modified by Schmurtz for Platformio and ESP32 :                                   */
/*                                                                                    */
/*  Release Notes (yyyy/mm/dd):                                                       */
/*  V0.1 - 2021/12/22 :                                                               */
/*    - Added these comments, pinouts and additionnal DAC settings                    */
/*    - Now compatible with platformio                                                */
/*    - Now compatible wih ESP32                                                      */
/*    - Added support for Google Translate TTS                                        */
/*  V0.2 - 2021/12/29 :                                                               */
/*    - fix playing RTTTL & SAM                                                       */
/*    - added aac playback, useful for many radios                                    */
/*    - added flac playback (working?)                                                */
/*    - added comments to change the SAMvoice (robot, elf , ET...)                    */
/*    - fix errors msg "connect on fd 63, errno: 118, "Host is unreachable""          */
/*      due to mqtt actions before wifi connects                                      */
/*                                                                                    */
/**************************************************************************************/

/* ============================================================================================================

  MrDIY Audio Notifier is a cloud-free media notifier that can play MP3s, stream icecast radios, read text
  and play RTTTL ringtones. It is controller over MQTT.


  + MQTT COMMANDS:  ( 'your_custom_mqtt_topic' is the MQTT prefix defined during the setup)

    - Play an MP3             MQTT topic: "your_custom_mqtt_topic/play"
                              MQTT load: http://url-to-the-mp3-file/file.mp3
                              remark : supports HTTP only, no HTTPS. -> this could be a solution : https://github.com/earlephilhower/ESP8266Audio/pull/410

    - Play an AAC             MQTT topic: "your_custom_mqtt_topic/aac"  (better for ESP32, hard for esp8266)
                              MQTT load: http://url-to-the-aac-file/file.aac

    - Play a Flac             MQTT topic: "your_custom_mqtt_topic/flac"  (better for ESP32, hard for esp8266)
                              MQTT load: http://url-to-the-flac-file/file.flac

    - Play an Icecast Stream  MQTT topic: "your_custom_mqtt_topic/stream"
                              MQTT load: http://url-to-the-icecast-stream/file.mp3, example: http://22203.live.streamtheworld.com/WHTAFM.mp3

    - Play a Ringtone         MQTT topic: "your_custom_mqtt_topic/tone"
                              MQTT load: RTTTL formated text, example: Soap:d=8,o=5,b=125:g,a,c6,p,a,4c6,4p,a,g,e,c,4p,4g,a

    - Say Text                MQTT topic: "your_custom_mqtt_topic/say"
                              MQTT load: Text to be read, example: Hello There. How. Are. You?

    - Stop Playing            MQTT topic: "your_custom_mqtt_topic/stop"

    - Change the Volume       MQTT topic: "your_custom_mqtt_topic/volume"
                              MQTT load: a double between 0.00 and 1.00, example: 0.7

    - Say Text with Google    MQTT topic: "your_custom_mqtt_topic/tts"
                              MQTT load: Text to be read, example: Hello There. How. Are. You?



  + ADDITIONAL INFORMATION:

    - The notifier sends status updates on MQTT topic: "your_custom_mqtt_topic/status" with these values:

                "playing"       either playing an mp3, a ringtone or saying a text
                "idle"          idle and waiting for a command
                "error"         error when receiving a command: example: MP3 file URL can't be loaded

    - The LWT MQTT topic: "your_custom_topic/LWT" with values:
                  "online"
                  "offline"

    - At boot, the notifier plays a 2 second audio chime when it is successfully connected to Wifi & MQTT

============================================================================================================


  + DEPENDENCIES:

    - ESP8266              https://github.com/esp8266/Arduino
    - ESP8266Audio         https://github.com/earlephilhower/ESP8266Audio
    - ESP8266SAM           https://github.com/earlephilhower/ESP8266SAM
    - IotWebConf           https://github.com/prampec/IotWebConf
    - PubSubClient         https://github.com/knolleary/pubsubclient
    - esp8266-google-tts   https://github.com/horihiro/esp8266-google-tts


  Many thanks to all the authors and contributors to the above libraries - you have done an amazing job!


============================================================================================================
  + SETUP:

    Option 1) Connect your ESP to an external amplifier.
    Option 2) Use an extenal DAC (like MAX98357A)

Depending the sound output you will choose below, here how to connect your DAC and your speaker :
______    _____   _____    ____    ___  
|  ____|  / ____| |  __ \  |___ \  |__ \ 
| |__    | (___   | |__) |   __) |    ) |
|  __|    \___ \  |  ___/   |__ <    / / 
| |____   ____) | | |       ___) |  / /_ 
|______| |_____/  |_|      |____/  |____|

+-----------------+-----------------+--------------+--------------------+
|   audio pins    |  External DAC   | Internal DAC |       No DAC       |
+-----------------+-----------------+--------------+--------------------+
| DAC - LCK(LRC)  | GPIO25          | NA           | NA                 |
| DAC - CK(BCLK)  | GPIO26          | NA           | NA                 |
| DAC - I2So(DIN) | GPIO22          | NA           | NA                 |
| speaker L       | NA (on the DAC) | GPIO25       | GPIO22 (L&R mixed) |
| speaker R       | NA (on the DAC) | GPIO26       | GPIO22 (L&R mixed) |
+-----------------+-----------------+--------------+--------------------+

______    _____   _____     ___    ___      __      __  
|  ____|  / ____| |  __ \   / _ \  |__ \    / /     / /  
| |__    | (___   | |__) | | (_) |    ) |  / /_    / /_  
|  __|    \___ \  |  ___/   > _ <    / /  | '_ \  | '_ \ 
| |____   ____) | | |      | (_) |  / /_  | (_) | | (_) |
|______| |_____/  |_|       \___/  |____|  \___/   \___/ 


+-----------------+-----------------+------------------------+
|   audio pins    |  External DAC   |         No DAC         |
+-----------------+-----------------+------------------------+
| DAC - LCK(LRC)  | GPIO2 (D4)      | NA                     |
| DAC - CK(BCLK)  | GPIO15 (D8)     | NA                     |
| DAC - I2So(DIN) | GPIO3 (RX)      | NA                     |
| speaker L       | NA (on the DAC) | GPIO3 (RX) (L&R mixed) |
| speaker R       | NA (on the DAC) | GPIO3 (RX) (L&R mixed) |
+-----------------+-----------------+------------------------+

  For more info, please watch MrDiy instruction video at https://youtu.be/SPa9SMyPU58
  MrDIY.ca & Schmurtz

============================================================================================================== */


#define DEBUG_FLAG // uncomment to enable debug mode & Serial output

// Please select one of these sound output options :
//#define USE_NO_DAC           // uncomment to use no DAC, using software-simulated delta-sigma DAC
//#define USE_EXTERNAL_DAC     // uncomment to use external I2S DAC 
#define USE_INTERNAL_DAC     // uncomment to use the internal DAC of the ESP32 (not available on ESP8266)

#include <SD.h> 
#include "Arduino.h"
#include "boot_sound.h"
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioGeneratorRTTTL.h"
#include "AudioGeneratorAAC.h"
#include <AudioGeneratorFLAC.h>

#ifdef USE_NO_DAC
    #include "AudioOutputI2SNoDAC.h"
#else
    #include "AudioOutputI2S.h"
#endif
#include "ESP8266SAM.h"
#include "IotWebConf.h"
#include "IotWebConfUsing.h"
#include <google-tts.h>

AudioGeneratorMP3 *mp3 = NULL;
AudioGeneratorAAC *aac = NULL;
AudioGeneratorFLAC *flac = NULL;
AudioGeneratorWAV *wav = NULL;
AudioGeneratorRTTTL *rtttl = NULL;
AudioFileSourceHTTPStream *file_http = NULL;
AudioFileSourcePROGMEM *file_progmem = NULL;
AudioFileSourceICYStream *file_icy = NULL;
AudioFileSourceBuffer *buff = NULL;
#ifdef USE_NO_DAC
AudioOutputI2SNoDAC     *out = NULL;
#else
AudioOutputI2S          *out = NULL;
#endif
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
#define port 1883
#define MQTT_MSG_SIZE 256
const char *willTopic = "LWT";
const char *willMessage = "offline";
boolean willRetain = false;
byte willQoS = 0;

int counter = 0;
// AudioRelated ---------------------------
float volume_level = 0.5;
String playing_status;
const int preallocateBufferSize = 4096;   // 4096 for ESP32 could be OK , 2048 for esp8266
void *preallocateBuffer = NULL;

// WifiManager -----------------------------
#ifdef ESP8266
String ChipId = String(ESP.getChipId(), HEX);
#elif ESP32
String ChipId = String((uint32_t)ESP.getEfuseMac(), HEX);
#endif
String thingName = String("MrDIY Notifier - ") + ChipId;
const char wifiInitialApPassword[] = "mrdiy.ca";
char mqttServer[16];
char mqttUserName[32];
char mqttUserPassword[32];
char mqttTopicPrefix[32];
char mqttTopic[MQTT_MSG_SIZE];


DNSServer dnsServer;
WebServer server(80);
IotWebConf iotWebConf(thingName.c_str(), &dnsServer, &server, wifiInitialApPassword);
iotwebconf::ParameterGroup mqttgroup = iotwebconf::ParameterGroup("mqttgroup", "");
iotwebconf::TextParameter mqttServerParam = iotwebconf::TextParameter("MQTT server", "mqttServer", mqttServer, sizeof(mqttServer));
iotwebconf::TextParameter mqttUserNameParam = iotwebconf::TextParameter("MQTT username", "mqttUser", mqttUserName, sizeof(mqttUserName));
iotwebconf::PasswordParameter mqttUserPasswordParam = iotwebconf::PasswordParameter("MQTT password", "mqttPass", mqttUserPassword, sizeof(mqttUserPassword), "password");
iotwebconf::TextParameter mqttTopicParam = iotwebconf::TextParameter("MQTT Topic", "mqttTopic", mqttTopicPrefix, sizeof(mqttTopicPrefix));

//#define LED_Pin           5       // external LED pin

char *mqttFullTopic(const char action[])
{
  strcpy(mqttTopic, mqttTopicPrefix);
  strcat(mqttTopic, "/");
  strcat(mqttTopic, action);
  return mqttTopic;
}

void broadcastStatus(const char topic[], String msg)
{
  if (playing_status != msg)
  {
    char charBuf[msg.length() + 1];
    msg.toCharArray(charBuf, msg.length() + 1);
    mqttClient.publish(mqttFullTopic(topic), charBuf);
    playing_status = msg;
#ifdef DEBUG_FLAG
    Serial.print(F("[MQTT] "));
    Serial.print(mqttTopicPrefix);
    Serial.print(F("/"));
    Serial.print(topic);
    Serial.print(F("\t\t"));
    Serial.println(msg);
#endif
  }
}

void updateLEDBrightness(int brightness_percentage)
{
#ifdef LED_Pin
  analogWrite(LED_Pin, (int)brightness_percentage * 255 / 100);
#endif
}

void mqttReconnect()
{

  if (!mqttClient.connected())
  {

    updateLEDBrightness(0);

    if (mqttClient.connect(thingName.c_str(), mqttUserName, mqttUserPassword, mqttFullTopic(willTopic), willQoS, willRetain, willMessage))
    {
      mqttClient.subscribe(mqttFullTopic("play"));
      mqttClient.subscribe(mqttFullTopic("aac"));
      mqttClient.subscribe(mqttFullTopic("flac"));
      mqttClient.subscribe(mqttFullTopic("stream"));
      mqttClient.subscribe(mqttFullTopic("tone"));
      mqttClient.subscribe(mqttFullTopic("say"));
      mqttClient.subscribe(mqttFullTopic("stop"));
      mqttClient.subscribe(mqttFullTopic("volume"));
      mqttClient.subscribe(mqttFullTopic("tts"));
#ifdef DEBUG_FLAG
      Serial.println(F("Connected to MQTT"));
#endif
      broadcastStatus("LWT", "online");
      broadcastStatus("ThingName", thingName.c_str());
      broadcastStatus("IPAddress", WiFi.localIP().toString());
      broadcastStatus("status", "idle");
      updateLEDBrightness(100);
    }
  }
}

/* ############################### Audio ############################################ */
    const char  RTTLsound[] PROGMEM = "5thSymph:d=16,o=5,b=160:g,g,g,4d#,4p,f,f,f,4d";
void playBootSound()
{
    file_progmem = new AudioFileSourcePROGMEM(boot_sound, sizeof(boot_sound));
    wav = new AudioGeneratorWAV();
    wav->begin(file_progmem, out);

    //  ===== a simpler boot sound : ===== (But little bug with ESP32 for now)
    // file_progmem = new AudioFileSourcePROGMEM(RTTLsound, strlen_P(RTTLsound));
    // rtttl = new AudioGeneratorRTTTL();
    // rtttl->begin(file_progmem, out);
}

void stopPlaying()
{
  if (mp3)
  {
#ifdef DEBUG_FLAG
    Serial.print(F("...#"));
    Serial.println(F("Interrupted!"));
    Serial.println();
#endif
    mp3->stop();
    delete mp3;
    mp3 = NULL;
  }
  if (aac)
  {
#ifdef DEBUG_FLAG
    Serial.print(F("...#"));
    Serial.println(F("Interrupted!"));
    Serial.println();
#endif
    aac->stop();
    delete aac;
    aac = NULL;
  }
  if (flac)
  {
#ifdef DEBUG_FLAG
    Serial.print(F("...#"));
    Serial.println(F("Interrupted!"));
    Serial.println();
#endif
    flac->stop();
    delete flac;
    flac = NULL;
  }
  if (wav)
  {
    wav->stop();
    delete wav;
    wav = NULL;
  }
  if (rtttl)
  {
    rtttl->stop();
    delete rtttl;
    rtttl = NULL;
  }
  if (buff)
  {
    buff->close();
    delete buff;
    buff = NULL;
  }
  if (file_http)
  {
    file_http->close();
    delete file_http;
    file_http = NULL;
  }
  if (file_progmem)
  {
    file_progmem->close();
    delete file_progmem;
    file_progmem = NULL;
  }
  if (file_icy)
  {
    file_icy->close();
    delete file_icy;
    file_icy = NULL;
  }
  
  broadcastStatus("status", "idle");
  updateLEDBrightness(100);
}



// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2)-1]=0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}

/* ################################## MQTT ############################################### */

void onMqttMessage(char *topic, byte *payload, unsigned int mlength)
{
  char newMsg[MQTT_MSG_SIZE];
  if (mlength > 0)
  {
    memset(newMsg, '\0', sizeof(newMsg));
    memcpy(newMsg, payload, mlength);
#ifdef DEBUG_FLAG
    Serial.println();
    Serial.print(F("Requested ["));
    Serial.print(topic);
    Serial.print(F("] "));
    Serial.println(newMsg);
#endif
    // got a new URL to play ------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("play")))
    {
      stopPlaying();
      file_http = new AudioFileSourceHTTPStream();
      if (file_http->open(newMsg))
      {
        broadcastStatus("status", "playing");
        updateLEDBrightness(50); // dim while playing
        buff = new AudioFileSourceBuffer(file_http, preallocateBuffer, preallocateBufferSize);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buff, out);
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }


    // got a new aac URL to play ------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("aac")))
    {
      stopPlaying();
      file_http = new AudioFileSourceHTTPStream();
      file_http->SetReconnect(3, 500);   // useful ?
      if (file_http->open(newMsg))
      {
        broadcastStatus("status", "playing");
        updateLEDBrightness(50); // dim while playing
        buff = new AudioFileSourceBuffer(file_http, preallocateBuffer, preallocateBufferSize);
        aac = new AudioGeneratorAAC();
        aac->begin(buff, out);
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }

    // got a new flac URL to play ------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("flac")))
    {
      stopPlaying();
      file_http = new AudioFileSourceHTTPStream();
      if (file_http->open(newMsg))
      {
        broadcastStatus("status", "playing");
        updateLEDBrightness(50); // dim while playing
        buff = new AudioFileSourceBuffer(file_http, preallocateBuffer, preallocateBufferSize);
        flac = new AudioGeneratorFLAC();
        flac->begin(buff, out);
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }

    // got a new URL to play ------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("stream")))
    {
      stopPlaying();
      file_icy = new AudioFileSourceICYStream();
      if (file_icy->open(newMsg))
      {
        broadcastStatus("status", "playing");
        updateLEDBrightness(50); // dim while playing
        buff = new AudioFileSourceBuffer(file_icy, preallocateBuffer, preallocateBufferSize);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buff, out);
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }

    // got a tone request --------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("tone")))
    {
      stopPlaying();
      broadcastStatus("status", "playing");
      updateLEDBrightness(50); // dim while playing
      file_progmem = new AudioFileSourcePROGMEM(newMsg, sizeof(newMsg));
      rtttl = new AudioGeneratorRTTTL();
      rtttl->begin(file_progmem, out);
      broadcastStatus("status", "idle");
      // an idea from the great project ESParkle to adjust the volume difference between mp3 and RTTTL : https://github.com/CosmicMac/ESParkle#settings
    }

    //got a TTS request ----------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("say")))
    {
      stopPlaying();
      broadcastStatus("status", "playing");
      //updateLEDBrightness(50);                      // dim while playing
      ESP8266SAM *sam = new ESP8266SAM;
      sam->SetVoice(sam->SAMVoice::VOICE_STUFFY);   // Make your choice : VOICE_SAM, VOICE_ELF, VOICE_ROBOT, VOICE_STUFFY, VOICE_OLDLADY, VOICE_ET (from ESP8266SAM.h)
      sam->Say(out, newMsg);
      //stopPlaying();
      out->stop();  // necessary to avoid infinite repeat of the last syllab
      delete sam;
      broadcastStatus("status", "idle");
    }

    //got a Google TTS request ----------------------------------------------------
    // Todo / idea : test if Google TTS if avaible, if not use SAM TTS instead
    if (!strcmp(topic, mqttFullTopic("tts")))
    {
      stopPlaying();

      TTS tts;
      String GoogleUrl = tts.getSpeechUrl(newMsg, "fr");  // here you can change the language of google TTS
      GoogleUrl.replace("https://","http://"); // little trick to keep the library unmodified and avoid to manage certs. 
                                              //alternative : String http = "http" + url.substring(5);
                                              //This could be better : https://github.com/earlephilhower/ESP8266Audio/pull/410
      Serial.println(GoogleUrl);

      // char buffer[GoogleUrl.length() + 1];
      // GoogleUrl.toCharArray(buffer, GoogleUrl.length() + 1);
      file_http = new AudioFileSourceHTTPStream();
      if (file_http->open( (const char *)GoogleUrl.c_str()))
      {
        broadcastStatus("status", "playing");
        buff = new AudioFileSourceBuffer(file_http, preallocateBuffer, preallocateBufferSize);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buff, out);
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }



    // got a volume request, expecting double [0.0,1.0] ---------------------
    if (!strcmp(topic, mqttFullTopic("volume")))
    {
      volume_level = atof(newMsg);
      if (volume_level < 0.0)
        volume_level = 0;
      if (volume_level > 1.0)
        volume_level = 0.7;
      out->SetGain(volume_level);
    }

    // got a stop request  --------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("stop")))
    {
      stopPlaying();
    }
  }
}

/* ############################ WifiManager ############################################# */

void wifiConnected()
{

#ifdef DEBUG_FLAG
  Serial.println();
  Serial.println(F("=================================================================="));
  Serial.println(F("  MrDIY Notifier - Modified By Schmurtz "));
  Serial.println(F("=================================================================="));
  Serial.println();
  Serial.print(F("Connected to Wifi \t["));
  Serial.print(WiFi.localIP());
  Serial.println(F("]"));
#endif
  
  mqttClient.setServer(mqttServer, port);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setBufferSize(MQTT_MSG_SIZE);
  mqttReconnect();

  playBootSound();
}

boolean formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper)
{

  boolean valid = true;
  int l = server.arg(mqttServerParam.getId()).length();
  if (l == 0)
  {
    mqttServerParam.errorMessage = "Please provide an MQTT server";
    valid = false;
  }
  return valid;
}


/* ################################## Setup ############################################# */
void setup()
{

#ifdef DEBUG_FLAG
  Serial.begin(115200);
#endif

#ifdef LED_Pin
  pinMode(LED_Pin, OUTPUT);
  updateLEDBrightness(10);
#endif

  mqttgroup.addItem(&mqttServerParam);
  mqttgroup.addItem(&mqttUserNameParam);
  mqttgroup.addItem(&mqttUserPasswordParam);
  mqttgroup.addItem(&mqttTopicParam);
  iotWebConf.addParameterGroup(&mqttgroup);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setFormValidator(&formValidator);
#ifdef LED_Pin
  iotWebConf.setStatusPin(LED_Pin);
#endif
  iotWebConf.skipApStartup();

  boolean validConfig = iotWebConf.init();
  if (!validConfig)
  {
    mqttServer[0] = '\0';
    mqttUserName[0] = '\0';
    mqttUserPassword[0] = '\0';
  }

  server.on("/", []
            { iotWebConf.handleConfig(); });
  server.onNotFound([]
                    { iotWebConf.handleNotFound(); });




#ifdef USE_INTERNAL_DAC
    out = new AudioOutputI2S(0, 1);  //use the internal DAC : channel 1 (gpio25) , channel 2 (gpio26) 
    Serial.println(F("Using the internal DAC of the ESP32 : \r\n speaker L -> GPIO25 \r\n speaker R -> GPIO26"));
#elif USE_EXTERNAL_DAC
    out = new AudioOutputI2S();
    #ifdef ESP32
      Serial.println(F("Using I2S output on ESP32 : please connect your DAC to pins : ")));
      Serial.println(F("LCK(LRC) -> GPIO25  \r\n BCK(BCLK) -> GPIO26 \r\n I2So(DIN) -> GPIO22"));
    #else    // we are on ESP8266
      Serial.println(F("Using I2S output on ESP8266 : please connect your DAC to pins : "));
      Serial.println(F("LCK(LRC) -> GPIO2 (D4) \r\n BCK(BCLK) -> GPIO15 (D8) \r\n I2So(DIN) -> GPIO3 (RX)"));
    #endif    // end of #ifdef ESP32
#else  // we don't use DAC, using software-simulated delta-sigma DAC
    out = new AudioOutputI2SNoDAC();
    #ifdef ESP32
      Serial.println(F("Using No DAC output on ESP32, audio output on pins \r\n speaker L & R -> GPIO22"));
    #else
      Serial.println(F("Using No DAC output on ESP8266, audio output on pins \r\n speaker L & R -> GPIO3 (RX)"));
    #endif
      Serial.println(F("Don't try and drive the speaker pins can't give enough current to drive even a headphone well and you may end up damaging your device"));
#endif

  out->SetGain(volume_level);
  // out->SetRate(22050);//44100
  // out->SetOversampling(64);
  playBootSound();
}

/* ##################################### Loop ############################################# */

void loop()
{

  iotWebConf.doLoop();
  if (iotWebConf.getState() == iotwebconf::OnLine){
  mqttReconnect();
  mqttClient.loop();
  }
  if (!mp3)
    iotWebConf.doLoop(); // give processor priority to web server
  if (mp3 && !mp3->loop())
    stopPlaying();
  if (wav && !wav->loop())
    stopPlaying();
  if (rtttl && !rtttl->loop())
    stopPlaying();
  if (aac && !aac->loop())
    stopPlaying();
  if (flac && !flac->loop())
    stopPlaying();


#ifdef DEBUG_FLAG
#ifdef ESP32
  if (mp3 && mp3->isRunning())
  {
    static int unsigned long lastms = 0;
    if (millis() - lastms > 1000)
    {
      lastms = millis();
      Serial.print(F("Free: "));
      Serial.print(ESP.getFreeHeap(), DEC);
      Serial.print(F("  ("));
      Serial.print(ESP.getFreeHeap() - ESP.getMaxAllocHeap(), DEC);   // heap_caps_get_largest_free_block() or getMaxAllocHeap() ?
      Serial.print(F(" lost)"));
      // Todo : calculate memory fragmentation on ESP32
      // Serial.print(F("  Fragmentation: "));
      // Serial.print(ESP.getHeapFragmentation(), DEC);
      // Serial.print(F("%"));
      // if (ESP.getHeapFragmentation() > 40)
        Serial.print(F("  ----------- "));
      Serial.println();
    }
  }
#else
  if (mp3 && mp3->isRunning())
  {
    static int unsigned long lastms = 0;
    if (millis() - lastms > 1000)
    {
      lastms = millis();
      Serial.print(F("Free: "));
      Serial.print(ESP.getFreeHeap(), DEC);
      Serial.print(F("  ("));
      Serial.print(ESP.getFreeHeap() - ESP.getMaxFreeBlockSize(), DEC);
      Serial.print(F(" lost)"));
      Serial.print(F("  Fragmentation: "));
      Serial.print(ESP.getHeapFragmentation(), DEC);
      Serial.print(F("%"));
      if (ESP.getHeapFragmentation() > 40)
        Serial.print(F("  ----------- "));
      Serial.println();
    }
  }
#endif
#endif
}

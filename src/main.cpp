/********************************************************************************************/
/*  "MrDiy Audio Notifier"                                                                  */
/*  Credits to original source : https://gitlab.com/MrDIYca/mrdiy-audio-notifier            */
/*  Modified by Schmurtz for Platformio and ESP32 :                                         */
/*                                                                                          */
/*  Release Notes (yyyy/mm/dd):                                                             */
/*  V0.1 - 2021/12/22 :                                                                     */
/*    - Added these comments, pinouts and additionnal DAC settings                          */
/*    - Now compatible with platformio                                                      */
/*    - Now compatible wih ESP32                                                            */
/*    - Added support for Google Translate TTS                                              */
/*  V0.2 - 2021/12/29 :                                                                     */
/*    - fix playing RTTTL & SAM                                                             */
/*    - added aac playback, useful for many radios                                          */
/*    - added flac playback (working?)                                                      */
/*    - added comments to change the SAMvoice (robot, elf , ET...)                          */
/*    - fix errors msg "connect on fd 63, errno: 118, "Host is unreachable""                */
/*      due to mqtt actions before wifi connects                                            */
/*  V0.3 - 2022/01/03 :                                                                     */
/*    - Migrate to IotWebConf 3.2.0   (Thanks to markvader)                                 */
/*  V0.4 - 2022/01/08 :                                                                     */
/*    - NEW: You can set these options directly in web interface                            */
/*      (These settings are saved & restored after a reboot)                                */
/*           * audio output (no DAC, internal or external DAC), reboot required             */
/*           * audio volume                                                                 */
/*           * default TTS sam voice style can be set in web interface                      */
/*           * default TTS google voice language can be set in web interface                */
/*    - NEW: Firmware update with web interface                                             */
/*    - NEW: MQTT command feedback msg when a problems are detected (topic "answer")        */
/*    - FIX : longer MQTT server name are supported                                         */
/*    - Breaking change : MQTT TTS commands renamed: samvoice /googlevoice                  */
/*    - Less different firmwares + Schmurtz_ESP_Flasher : Windows script for easy flashing  */
/*  V0.5 - 2022/02/18 :                                                                     */
/*    - NEW: increase and decrease volume with MQTT commands                                */
/*    - NEW: increase and decrease volume physical buttons on GPIO                          */
/*  V0.6 - 2022/02/22 :                                                                     */
/*    - FIX: Google TTS : a lot better, almost no hang                                      */
/*  V0.61- 2022/02/22 :                                                                     */
/*    - FIX: SAMVOICE TTS on ESP8266                                                        */
/*    - FIX: Restore generated unique name for MQTT                                         */
/*                                                                                          */
/********************************************************************************************/

/* ============================================================================================================

  MrDIY Audio Notifier is a cloud-free media notifier that can play MP3s, aac, stream icecast radios, read text
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
                              MQTT load: http://url-to-the-icecast-stream/file.mp3, example: http://us1.internet-radio.com:8321/stream

    - Play a Ringtone         MQTT topic: "your_custom_mqtt_topic/tone"
                              MQTT load: RTTTL formated text, example: Soap:d=8,o=5,b=125:g,a,c6,p,a,4c6,4p,a,g,e,c,4p,4g,a

    - Stop Playing            MQTT topic: "your_custom_mqtt_topic/stop"

    - Change the Volume       MQTT topic: "your_custom_mqtt_topic/volume"
                              MQTT load: a double between 0.00 and 1.00, example: 0.7

    - Say Text                MQTT topic: "your_custom_mqtt_topic/samvoice"
                              MQTT load: Text to be read, example: Hello There. How. Are. You?

    - Say Text with Google    MQTT topic: "your_custom_mqtt_topic/googlevoice"
                              MQTT load: Text to be read,Language, example: Hello There. How. Are. You?  OR  Bonjour, comment allez vous?,fr-FR
                              List of supported languages : https://github.com/florabtw/google-translate-tts/blob/master/src/voices.js



  + ADDITIONAL INFORMATION:

    - The notifier sends status updates on MQTT topic: "your_custom_mqtt_topic/status" with these values:

                "playing"       either playing an mp3, a ringtone or saying a text
                "idle"          idle and waiting for a command
                "error"         error when receiving a command: example: MP3 file URL can't be loaded

    - The LWT (Last Will & Testament) MQTT topic: "your_custom_topic/LWT" with values:
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
| DAC - BCK(BCLK) | GPIO26          | NA           | NA                 |
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
| DAC - BCK(BCLK) | GPIO15 (D8)     | NA                     |
| DAC - I2So(DIN) | GPIO3 (RX)      | NA                     |
| speaker L       | NA (on the DAC) | GPIO3 (RX) (L&R mixed) |
| speaker R       | NA (on the DAC) | GPIO3 (RX) (L&R mixed) |
+-----------------+-----------------+------------------------+

  For more info, please watch MrDiy instruction video at https://youtu.be/SPa9SMyPU58
  MrDIY.ca & Schmurtz

============================================================================================================== */

#define DEBUG_FLAG // uncomment to enable debug mode & Serial output

// ---------- obsolete part , will be deleted ----------
// Please select one of these sound output options :
//#define USE_NO_DAC           // uncomment to use no DAC, using software-simulated delta-sigma DAC
//#define USE_EXTERNAL_DAC     // uncomment to use external I2S DAC 
//#define USE_INTERNAL_DAC     // uncomment to use the internal DAC of the ESP32 (not available on ESP8266)
// ------------------------------------------------------

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

// #ifdef USE_NO_DAC
    #include "AudioOutputI2SNoDAC.h"
// #else
    #include "AudioOutputI2S.h"
// #endif
#include "ESP8266SAM.h"
#include "IotWebConf.h"
#include "IotWebConfUsing.h"

// UpdateServer includes
#ifdef ESP8266
  # include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
# include <IotWebConfESP32HTTPUpdateServer.h>   // For ESP32 IotWebConf provides a drop-in replacement for UpdateServer.
#endif

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
// #ifdef USE_NO_DAC
AudioOutputI2SNoDAC     *outNoDac = NULL;
// #else
AudioOutputI2S          *out = NULL;
// #endif
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

// ============== declarations for buttons : ==============
typedef struct
{
  byte ButtonGpio = 255;        // 255 means that this button is not initialized
  bool CurrentGPIOstate = 1;    // Digital state of the pin (High or LOW)
  bool PreviousGPIOState = 1;   // Previous digital state of the pin (High or LOW)
  unsigned long time = 0;       // Used to record the time since when the button has been pressed
  bool BoundarySwitcher = 0;    // this boolean is used to debounce the button and avoid repetitions
  byte CurrentButtonstate = 0;  // Current state of the button :
  //                                0 : nothing , 1 = short press begins , 2 = short press released , 3 = long press begins
  //                                4 = long press released , 5 = extra long press begins , 6 = extra long press released , 7 = currently pushed
} button;

button ButtonsDatas[2];
const int DEBOUNCE_TIME = 150;      // debounce time = minimal time between 2 clicks (in ms)
const int DOUBLE_PRESS = 500;       // maximum time between 2 press to obtain a double press event
const int LONG_PRESS = 1500;        // time to press a button to trigger a long press action
const int EXTRA_LONG_PRESS = 3000;  // time to press a button to trigger an extra long press action
bool DblePressActionDone = 0;       // used to debouche simulataneous dual press

const int BTN_VOL_LOWER = 13;
const int BTN_VOL_LOUDER = 14;

// ============ End of declarations for buttons : ============

String playing_status;
const int preallocateBufferSize = 4096;   // 4096 for ESP32 could be OK , 2048 for esp8266
void *preallocateBuffer = NULL;

// WifiManager -----------------------------
#ifdef ESP8266
String ChipId = String(ESP.getChipId(), HEX);
#elif ESP32
String ChipId = String((uint32_t)ESP.getEfuseMac(), HEX);
#endif
String thingName = String("MrDIYNotifier-"+ ChipId);
const char wifiInitialApPassword[] = "mrdiy.ca";
char mqttServer[64];
char mqttUserName[32];
char mqttUserPassword[32];
char mqttTopicPrefix[32];
char mqttTopic[MQTT_MSG_SIZE];


#define STRING_LEN 14
#define NUMBER_LEN 4
// -- Callback methods.
void configSaved();
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
char checkboxParamValue[STRING_LEN];
char soundOutputValue[STRING_LEN];
char floatsoundVolume[4];
char samVoiceValue[8];
char GoogleTTSvoice[6];

#ifdef ESP8266
  static char chooserValues[][STRING_LEN] = { "NoDAC" , "EXTERNAL_DAC" };
  static char chooserNames[][STRING_LEN] = { "No DAC" , "External DAC" };
#elif ESP32
  static char chooserValues[][STRING_LEN] = { "INTERNAL_DAC", "EXTERNAL_DAC", "NoDAC" };
  static char chooserNames[][STRING_LEN] = { "Internal DAC", "External DAC", "No DAC" };
#endif

  static char samVoiceValues[][8] = { "STUFFY" , "SAM", "ELF", "ROBOT",  "OLDLADY", "ET" };
  static char samVoiceNames[][8] = { "STUFFY" , "SAM", "ELF", "ROBOT",  "OLDLADY", "ET" };

DNSServer dnsServer;
WebServer server(80);
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif defined(ESP32)
HTTPUpdateServer httpUpdater;
#endif



IotWebConf iotWebConf(thingName.c_str(), &dnsServer, &server, wifiInitialApPassword);
iotwebconf::ParameterGroup mqttgroup = iotwebconf::ParameterGroup("mqttgroup", "MQTT parameters");
iotwebconf::TextParameter mqttServerParam = iotwebconf::TextParameter("MQTT server", "mqttServer", mqttServer, sizeof(mqttServer));
iotwebconf::TextParameter mqttUserNameParam = iotwebconf::TextParameter("MQTT username", "mqttUser", mqttUserName, sizeof(mqttUserName));
iotwebconf::PasswordParameter mqttUserPasswordParam = iotwebconf::PasswordParameter("MQTT password", "mqttPass", mqttUserPassword, sizeof(mqttUserPassword), "password");
iotwebconf::TextParameter mqttTopicParam = iotwebconf::TextParameter("MQTT Topic", "mqttTopic (without ""/"" at the end)", mqttTopicPrefix, sizeof(mqttTopicPrefix));

iotwebconf::ParameterGroup soundgroup = iotwebconf::ParameterGroup("soundgroup", "Sound parameters");
iotwebconf::SelectParameter soundOutput = IotWebConfSelectParameter("Sound output :", "Sound_output", soundOutputValue, STRING_LEN, (char*)chooserValues, (char*)chooserNames, sizeof(chooserValues) / STRING_LEN, STRING_LEN);
// iotwebconf::TextParameter soundVolume = IotWebConfTextParameter("String param", "stringParam", soundVolume, STRING_LEN);
iotwebconf::NumberParameter soundVolume = IotWebConfNumberParameter("Sound Volume (0 - 1.0)", "Sound_Volume", floatsoundVolume, NUMBER_LEN,  "0.5", "e.g. 0.7", "min='0' max='1' step='0.1'");
iotwebconf::SelectParameter samVoice = IotWebConfSelectParameter("samVoice", "samVoice :", samVoiceValue, 8, (char*)samVoiceValues, (char*)samVoiceNames, sizeof(samVoiceValues) / 8, 8);
iotwebconf::TextParameter GoogleTTSvoiceParam = iotwebconf::TextParameter("Google TTS Voice code (<a href=""https://github.com/florabtw/google-translate-tts/blob/master/src/voices.js"" target=""_blank"">list</a>)", "GoogleTTSvoice", GoogleTTSvoice, sizeof(GoogleTTSvoice));


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
    Serial.print(F("\t"));
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
      mqttClient.subscribe(mqttFullTopic("samvoice"));
      mqttClient.subscribe(mqttFullTopic("stop"));
      mqttClient.subscribe(mqttFullTopic("volume"));
      mqttClient.subscribe(mqttFullTopic("googlevoice"));
#ifdef DEBUG_FLAG
      Serial.println(F("Connected to MQTT"));
#endif
      broadcastStatus("LWT", "online");
      broadcastStatus("SSID", thingName.c_str());
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
    if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0  || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
    {
        wav->begin(file_progmem, out);
    }else{
        wav->begin(file_progmem, outNoDac);
    }
    

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

void SetVolume(char MQTTMsg[MQTT_MSG_SIZE], bool toSave)
{


    const char errorMsg[] = "Wrong value (not a number) : Volume must be a number between 0.0 and 1.0 !";
    bool isnumeric = true;
    float volume_level;
    


    for (int i = 0; i < sizeof(MQTTMsg)/sizeof(MQTTMsg[0]); i++) {
      if (isdigit(MQTTMsg[i]) == 0 && MQTTMsg[i] != '.' && MQTTMsg[i] != '\0')  {  // We check each char of the array , must be a part of a number (or empty)
          isnumeric = false;
      }
     }

    if (isnumeric == true) 
    {
      volume_level = atof(MQTTMsg);
    }else{
        volume_level = atof(floatsoundVolume);
         if  (MQTTMsg[0] == '-') volume_level = volume_level - 0.1;
         if  (MQTTMsg[0] == '+') volume_level = volume_level + 0.1;
    }

    if (volume_level >= 0.0 && volume_level <= 1.0 )  //
    {
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            out->SetGain(volume_level);
        }else{
            outNoDac->SetGain(volume_level);
        }

        // we record the current volume to restore it on the next boot thanks to IotWebConf library
        dtostrf(volume_level,1,1, floatsoundVolume);   // convert float to char array ,1 character minimum, 1 digit
        Serial.print(F("Volume set to : "));
        Serial.println(floatsoundVolume);
        if (toSave) iotWebConf.saveConfig();
    }
    else
    {
        Serial.println(errorMsg);
        broadcastStatus("anwser", errorMsg);
    }

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
    Serial.println(newMsg);  // here we can see that that specifics alphabets like Cyrillic are not supported
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
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            mp3->begin(buff, out);
        }else{
            mp3->begin(buff, outNoDac);
        }
        
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
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            aac->begin(buff, out);
        }else{
            aac->begin(buff, outNoDac);
        }
        
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
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            flac->begin(buff, out);
        }else{
            flac->begin(buff, outNoDac);
        }
        
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
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            mp3->begin(buff, out);
        }else{
            mp3->begin(buff, outNoDac);
        }
        
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
      if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
      {
          rtttl->begin(file_progmem, out);
      }else{
          rtttl->begin(file_progmem, outNoDac);
      }
      
      broadcastStatus("status", "idle");
      // an idea from the great project ESParkle to adjust the volume difference between mp3 and RTTTL : https://github.com/CosmicMac/ESParkle#settings
    }

    //got a TTS request ----------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("samvoice")))
    {
      stopPlaying();
      broadcastStatus("status", "playing");
      //updateLEDBrightness(50);                      // dim while playing
      ESP8266SAM *sam = new ESP8266SAM;
         // Make your choice : VOICE_SAM, VOICE_ELF, VOICE_ROBOT, VOICE_STUFFY, VOICE_OLDLADY, VOICE_ET (from ESP8266SAM.h)

      if (strcmp(samVoiceValue,"STUFFY") == 0) sam->SetVoice(sam->SAMVoice::VOICE_STUFFY);
      if (strcmp(samVoiceValue,"SAM") == 0) sam->SetVoice(sam->SAMVoice::VOICE_SAM);
      if (strcmp(samVoiceValue,"ELF") == 0) sam->SetVoice(sam->SAMVoice::VOICE_ELF);
      if (strcmp(samVoiceValue,"ROBOT") == 0) sam->SetVoice(sam->SAMVoice::VOICE_ROBOT);
      if (strcmp(samVoiceValue,"OLDLADY") == 0) sam->SetVoice(sam->SAMVoice::VOICE_OLDLADY);
      if (strcmp(samVoiceValue,"ET") == 0) sam->SetVoice(sam->SAMVoice::VOICE_ET);


      if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
      {
        sam->Say(out, newMsg);
        Serial.print(soundOutputValue);
        
      }else{
        sam->Say(outNoDac, newMsg); 
        Serial.print(soundOutputValue);
      }
      
      
      //stopPlaying();
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            out->stop();  // necessary to avoid infinite repeat of the last syllab
        }else{
            outNoDac->stop();  // necessary to avoid infinite repeat of the last syllab
        }
      //out->stop();  // necessary to avoid infinite repeat of the last syllab
      delete sam;
      broadcastStatus("status", "idle");
    }

    //got a Google TTS request ----------------------------------------------------
    // Todo / idea : test if Google TTS if avaible, if not use SAM TTS instead
    if (!strcmp(topic, mqttFullTopic("googlevoice")))
    {

      const int LANGMAXLENGTH = 6; 
      stopPlaying();

      TTS tts;

      // ---------  facultative part : check if the language is known by Google TTS, useful to have a warning when a wrong language is used ---------
      String StrnewMsg =  newMsg;
      String SelectedLanguage;
      bool KnownLanguage = false;
      const char allowedLang[68][LANGMAXLENGTH] = { "af-ZA" , "sq" , "ar-AE" , "hy" , "bn-BD" , "bn-IN" , "bs" , "my" , "ca-ES" , "cmn-H" , "hr-HR" ,
        "cs-CZ" , "da-DK" , "nl-NL" , "en-AU" , "en-GB" , "en-US" , "eo" , "et" , "fil-P" , "fi-FI" , "fr-FR" , "fr-CA" , "de-DE" , "el-GR" ,
        "gu" , "hi-IN" , "hu-HU" , "is-IS" , "id-ID" , "it-IT" , "ja-JP" , "kn" , "km" , "ko-KR" , "la" , "lv" , "mk" , "ml" , "mr" , "ne" ,
        "nb-NO" , "pl-PL" , "pt-BR" , "ro-RO" , "ru-RU" , "sr-RS" , "si" , "sk-SK" , "es-MX" , "es-ES" , "sw" , "sv-SE" , "ta" , "te" , "th-TH" ,
        "tr-TR" , "uk-UA" , "ur" , "vi-VN" , "cy" , "fr" , "de" , "en" , "ja" , "ru"};   
         // source of supported languages : https://github.com/florabtw/google-translate-tts/blob/master/src/voices.js
         // Could be converted into a dropdown list in web UI for more simplicity....

        //String StrnewMsg =  String((char *)payload);
        int index = StrnewMsg.lastIndexOf(',');   // take position of the last ","
        if ( StrnewMsg.length() -  index <= LANGMAXLENGTH)     // it could be a language parameter, let's check if it exists in the list...
        {
            SelectedLanguage = StrnewMsg.substring(index + 1);   // a new string which contains what's after the last ","
            Serial.println(SelectedLanguage);
            for (int i = 0; i < (sizeof(allowedLang) / sizeof(allowedLang[1])) -1; i++) 
            {
                if (strcmp(allowedLang[i],SelectedLanguage.c_str())==0)  KnownLanguage = true;  // checking if current language is contained in the list
            }

            if (KnownLanguage == true)
            { 
              StrnewMsg = StrnewMsg.substring(0, index);
              newMsg[mlength-index] = 0; // now we remove the language part from the string 
            }
        }

         if (KnownLanguage == false)
                  {
              char errorMsg[96] = "No language recogniszed in this command, voice configured in settings will be used : ";
              strcat(errorMsg,GoogleTTSvoice);
              broadcastStatus("anwser", errorMsg);
              SelectedLanguage = GoogleTTSvoice;
            }
       // ---------------------------------------------------------------------------------------------------------------------------------------



      String GoogleUrl = tts.getSpeechUrl(StrnewMsg, SelectedLanguage);  // here we change the language of google TTS

      GoogleUrl.replace("https://","http://"); // little trick to keep the library unmodified and avoid to manage certs. 
                                              //alternative : String http = "http" + url.substring(5);
                                              //This could be better : https://github.com/earlephilhower/ESP8266Audio/pull/410
      #ifdef DEBUG_FLAG
        Serial.println(GoogleUrl);
      #endif

      // char buffer[GoogleUrl.length() + 1];
      // GoogleUrl.toCharArray(buffer, GoogleUrl.length() + 1);
      file_icy = new AudioFileSourceICYStream();
      if (file_icy->open( (const char *)GoogleUrl.c_str()))
      {
        broadcastStatus("status", "playing");
        //updateLEDBrightness(50); // dim while playing
        buff = new AudioFileSourceBuffer(file_icy, preallocateBuffer, preallocateBufferSize);
        mp3 = new AudioGeneratorMP3();
        if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0 || strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
        {
            mp3->begin(buff, out);
        }else{
            mp3->begin(buff, outNoDac);
        }
        
      }
      else
      {
        stopPlaying();
        broadcastStatus("status", "error");
        broadcastStatus("status", "idle");
      }
    }



    // got a volume request, expecting double [0.0 - 1.0] ---------------------
    if (!strcmp(topic, mqttFullTopic("volume")))
    {
      SetVolume(newMsg, true);
    }

    // got a stop request  --------------------------------------------------
    if (!strcmp(topic, mqttFullTopic("stop")))
    {
      stopPlaying();
    }
  }
}




/* ###################################### Physical Buttons ########################################### */

void ButtonManager()
{

  for (int i = 0; i < sizeof ButtonsDatas / sizeof ButtonsDatas[0]; i++)   // for each button in ButtonsDatas
  {
    if (ButtonsDatas[i].ButtonGpio < 255)       // if a gpio has been defined
    {

      ButtonsDatas[i].CurrentGPIOstate = digitalRead(ButtonsDatas[i].ButtonGpio);               // we update the GPIO state


      // ==================================================== Case where we have just had a change of state : button pushed =====================================================


      if (ButtonsDatas[i].CurrentGPIOstate == LOW && ButtonsDatas[i].PreviousGPIOState == HIGH) //
      {

        if (millis() - ButtonsDatas[i].time < DEBOUNCE_TIME)  // if during the moment the button is pressed the time of pressing is less than ...
        {
          ButtonsDatas[i].CurrentGPIOstate = HIGH;  // we cancel the action to debounce
          ButtonsDatas[i].CurrentButtonstate = 0;

        }
        else if (millis() - ButtonsDatas[i].time < DOUBLE_PRESS )
        {
          ButtonsDatas[i].time = millis(); // We start counting the time from the moment the button is pressed
          ButtonsDatas[i].CurrentButtonstate = 8;  // like explained above: 1 = short press begins
        }
        else
        {
          ButtonsDatas[i].time = millis(); // We start counting the time from the moment the button is pressed
          ButtonsDatas[i].CurrentButtonstate = 1;  // like explained above: 1 = short press begins
        }
      }


      // ================================================================== Case where the button remains pressed ==================================================================


      if (ButtonsDatas[i].CurrentGPIOstate == LOW && ButtonsDatas[i].PreviousGPIOState == LOW)
      {

        if (millis() - ButtonsDatas[i].time <= LONG_PRESS)  // if during the moment the button is pressed the time of pressing is less than ...
        {
          ButtonsDatas[i].CurrentButtonstate = 7;   // necessary if we want to detect the button only one time when it is pressed
        }


        if (millis() - ButtonsDatas[i].time > LONG_PRESS && millis() - ButtonsDatas[i].time < EXTRA_LONG_PRESS) // if during the moment the button is pressed the time of pressing is more than ...
        {
          if (ButtonsDatas[i].BoundarySwitcher == 0) // on the next boundary we will check if it is = to 1
          {
            ButtonsDatas[i].BoundarySwitcher = 1;    // so it allow to execute that only one time...
            ButtonsDatas[i].CurrentButtonstate = 3;
          }
          else
          {
            ButtonsDatas[i].CurrentButtonstate = 7;   // necessary if we want to detect the button only one time when it is pressed
          }
        }


        if (millis() - ButtonsDatas[i].time >= EXTRA_LONG_PRESS) // si durant le moment ou le bouton est appuyé le temps d'appui est sup à ...
        {
          if (ButtonsDatas[i].BoundarySwitcher == 1) // on the next boundary we will check if it is = to 0
          {
            ButtonsDatas[i].BoundarySwitcher = 0;    // so it allow to execute that only one time...
            ButtonsDatas[i].CurrentButtonstate = 5;
          }
          else
          {
            ButtonsDatas[i].CurrentButtonstate = 7;   // necessary if we want to detect the button only one time when it is pressed
          }
        }
      }


      // ================================================================== In case the button has just been released ==================================================================


      if (ButtonsDatas[i].CurrentGPIOstate == HIGH && ButtonsDatas[i].PreviousGPIOState == LOW)
      {

        if (millis() - ButtonsDatas[i].time <= LONG_PRESS)
        {
          ButtonsDatas[i].CurrentButtonstate = 2;
        }


        if (millis() - ButtonsDatas[i].time > LONG_PRESS && millis() - ButtonsDatas[i].time < EXTRA_LONG_PRESS) // if during the moment the button is pressed the time of pressing is between LONG_PRESS and EXTRA_LONG_PRESS ...
        {
          ButtonsDatas[i].CurrentButtonstate = 4;
        }


        if (millis() - ButtonsDatas[i].time >= EXTRA_LONG_PRESS)            // if during the moment the button is pressed the time of pressing is more than EXTRA_LONG_PRESS ...
        {
          ButtonsDatas[i].CurrentButtonstate = 6;
        }

        ButtonsDatas[i].time = millis();   // To debounce the next push
      }


      // ============================================================================= In case nothing happens =============================================================================



      if (ButtonsDatas[i].CurrentGPIOstate == HIGH && ButtonsDatas[i].PreviousGPIOState == HIGH)
      {
        ButtonsDatas[i].BoundarySwitcher = 0;
        ButtonsDatas[i].CurrentButtonstate = 0;
      }
      // ===================================================================================================================================================================================


      ButtonsDatas[i].PreviousGPIOState = ButtonsDatas[i].CurrentGPIOstate;
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

void configSaved()
{
  Serial.println("Configuration was updated.");
  Serial.println(samVoiceValue);
  Serial.println(floatsoundVolume);
  SetVolume(floatsoundVolume,false);
  

  // could be a way to resume changes in MQTT , or we could use something like this : iotWebConf.getSystemParameterGroup()->debugTo(&Serial);

  // char myConcatenation[120];
  // strcpy(myConcatenation, "Volume level : ");
  // strcat(myConcatenation,floatsoundVolume);
  // strcat(myConcatenation,"\n");
  // Serial.println(myConcatenation);
  // broadcastStatus("anwser", myConcatenation);
  
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

  soundgroup.addItem(&soundOutput);
  soundgroup.addItem(&soundVolume);
  soundgroup.addItem(&samVoice);
  soundgroup.addItem(&GoogleTTSvoiceParam);
  

  iotWebConf.addParameterGroup(&mqttgroup);
  iotWebConf.addParameterGroup(&soundgroup);
  iotWebConf.setWifiConnectionCallback(&wifiConnected);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.setConfigSavedCallback(&configSaved); //callback IotWebConf
  iotWebConf.getThingNameParameter()->label = "Name (for AP,MQTT,DNS)"; // rename the SSID field in web interface "Thing name" -> "SSID"
  iotWebConf.getApPasswordParameter()->label = "Password (for AP and web UI)";
#ifdef LED_Pin
  iotWebConf.setStatusPin(LED_Pin);
#endif
  iotWebConf.skipApStartup();

  // -- Define how to handle updateServer calls.
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });

  
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


Serial.println(soundOutputValue);

if (strcmp(soundOutputValue,"INTERNAL_DAC") == 0) 
{
    out = new AudioOutputI2S(0, 1);  //use the internal DAC : channel 1 (gpio25) , channel 2 (gpio26) 
    out->SetGain(atof(floatsoundVolume));
    Serial.println(F("Using the internal DAC of the ESP32 : \r\n speaker L -> GPIO25 \r\n speaker R -> GPIO26"));
}
else if (strcmp(soundOutputValue,"EXTERNAL_DAC") == 0) 
{
    out = new AudioOutputI2S();
    out->SetGain(atof(floatsoundVolume));
    #ifdef ESP32
      Serial.println(F("Using I2S output on ESP32 : please connect your DAC to pins : "));
      Serial.println(F("LCK(LRC) -> GPIO25  \r\n BCK(BCLK) -> GPIO26 \r\n I2So(DIN) -> GPIO22"));
    #else    // we are on ESP8266
      Serial.println(F("Using I2S output on ESP8266 : please connect your DAC to pins : "));
      Serial.println(F("LCK(LRC) -> GPIO2 (D4) \r\n BCK(BCLK) -> GPIO15 (D8) \r\n I2So(DIN) -> GPIO3 (RX)"));
    #endif    // end of #ifdef ESP32
}else{     // we don't use DAC, using software-simulated delta-sigma DAC
    
    outNoDac = new AudioOutputI2SNoDAC();
    outNoDac->SetGain(atof(floatsoundVolume));
    #ifdef ESP32
      Serial.println(F("Using No DAC output on ESP32, audio output on pins \r\n speaker L & R -> GPIO22"));
    #else
      Serial.println(F("Using No DAC output on ESP8266, audio output on pins \r\n speaker L & R -> GPIO3 (RX)"));
    #endif
      Serial.println(F("Don't try and drive the speaker pins can't give enough current to drive even a headphone well and you may end up damaging your device"));
}

//we manage some buttons :
  pinMode(BTN_VOL_LOWER, INPUT_PULLUP);
  pinMode(BTN_VOL_LOUDER, INPUT_PULLUP);
  ButtonsDatas[0].ButtonGpio = BTN_VOL_LOWER;
  ButtonsDatas[1].ButtonGpio = BTN_VOL_LOUDER;


  


  // attachInterrupt(digitalPinToInterrupt(BTN_VOL_LOWER), manageVolLowerPinInterrupt, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(BTN_VOL_LOUDER), manageVolLouderPinInterrupt, CHANGE);
 
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

ButtonManager(); // we refresh buttons status

// now we manage volume with simple click or double click with buttons :
if (ButtonsDatas[0].CurrentButtonstate == 1 || ButtonsDatas[0].CurrentButtonstate == 8) SetVolume("-",false); 
if (ButtonsDatas[1].CurrentButtonstate == 1 || ButtonsDatas[1].CurrentButtonstate == 8) SetVolume("+",false);

}

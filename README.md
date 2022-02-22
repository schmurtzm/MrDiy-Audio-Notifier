# MrDiy Audio Notifier - Schmurtz Edition
 ----
  MrDiy Audio Notifier is an audio player controlled by MQTT.
  
  It is able to :
- play local TTS (samvoice)
- play Google Translate TTS
- play MP3, AAC and FLAC
- play RTTTL (Nokia ringtones)
- play some web radio


MrDiy Audio Notifier is based on esp8266audio library. This repo uses most of the MrDiy's code with some modifications :

- Ported to platformio (with differents recommanded settings for ESP8266audio)
- Can be compiled for ESP8266 and ESP32
- IotWebConf updated to version v3.x (thanks to Markvader)
- Google Translate TTS (multilingual and better voice quality than the local ESP8266SAM). [List of supported languages](https://github.com/florabtw/google-translate-tts/blob/master/src/voices.js).
- Set voice style of samvoice
- Ability to play AAC (required for many web radio) and flac (not tested)
- New settings in web interface allow to switch easily between no DAC (version used by mr DIY) , external DAC or internal DAC (for ESP32).
- Set volume with physical buttons
- Documentation to connect your ESP to a speaker (see comments at the top of the "main.cpp" file)
- You’ll also find some useful comments for wiring your DAC quickly or to improve the code (sound level of RTTTS, ssl, IotWebConf migration to v3.x...).
- ...

```
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
/*    - FIX: longer MQTT server name are supported                                          */
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
```

 How to flash :
 ----
Precompiled firmwares are available for ESP8266 (Wemos D1 Mini) and ESP32 (esp32doit-devkit-v4).

To flash it, plug your ESP in USB port, download this full repo and then run _Schmurtz_ESP_Flasher\Schmurtz_ESP_Flasher.bat (Windows only).



 How to use :
 ----
 Just after flashing you will find an AP (wifi network) called "MrDIYNotifier".
 The default password to connect to this wifi network is "mrdiy.ca".
 Once connected open web browser and go to http://192.168.4.1 , you will able to configure your wifi parameters, mqtt parameters here and sound parameters (internal DAC, external DAC or no DAC).
 The "AP password" must be changed (8 characters minimum), keep it in mind, it will be usefull to access to the web interface later.
 
 Once connected to your wifi, to go back to configuration opne this url in your favorite browser : http://MrDIYNotifier.local
 The login is "admin" and the password is the "AP password" that you have defined during wifi configuration.

 


 MQTT COMMANDS:  ( 'your_mqtt_topic' is the MQTT prefix defined during the setup)

    - Play an MP3               MQTT topic: "your_mqtt_topic/play"
                                MQTT load: http://url-to-the-mp3-file/file.mp3
                                PS: supports HTTP only, no HTTPS. -> this could be a solution :
                                https://github.com/earlephilhower/ESP8266Audio/pull/410

    - Play an AAC               MQTT topic: "your_mqtt_topic/aac" (OK on ESP32, hard on esp8266)
                                MQTT load: http://url-to-the-aac-file/file.aac

    - Play an Icecast Stream    MQTT topic: "your_mqtt_topic/stream"
                                MQTT load: http://url-to-the-icecast-stream/file.mp3
                                example: http://icecast.radiofrance.fr/fiprock-midfi.mp3

    - Play a Flac               MQTT topic: "your_mqtt_topic/flac"  
                                (better for ESP32, hard for esp8266)
                                MQTT load: http://url-to-the-flac-file/file.flac

    - Play a Ringtone           MQTT topic: "your_mqtt_topic/tone"
                                MQTT load: RTTTL formated text
                                example: Soap:d=8,o=5,b=125:g,a,c6,p,a,4c6,4p,a,g,e,c,4p,4g,a

    - Stop Playing              MQTT topic: "your_mqtt_topic/stop"

    - Set the Volume            MQTT topic: "your_mqtt_topic/volume"
                                MQTT load: a double between 0.00 and 1.00
                                example: 0.7
								
    - increase/decrease Volume  MQTT topic: "your_mqtt_topic/volume"
                                MQTT load: + ou -
                                example: +  -> will increase volume of 0.1
								
    - Say Text                  MQTT topic: "your_mqtt_topic/samvoice"
                                MQTT load: Text to be read
                                example: Hello There. How. Are. You?

    - Say Text with Google      MQTT topic: "your_mqtt_topic/googlevoice"
                                MQTT load: Text to be read,language  (language is facultative)
                                example: Hello There. How are you?
                                         Bonjour, comment allez vous?,fr-FR
                              
For additionnal details, check the comments at the beginning of the "main.cpp".

 How to connect ESP to speaker :
 ----
3 possibilities to have a sound output from the ESP (please read [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) documentation for more information): 
- use an additional I2S DAC module (like PCM5102 for example, MAX98357 board offers a 3W amp to drive your speaker directly)
- use the internal DAC of the ESP32 (not available on ESP8266)
- use a virtual DAC sigma (the sound will be less good than a DAC but enough for speak notifications)

Don't try to drive the speaker directly, the ESP8266 pins can't give enough current to drive even a headphone well and you may end up damaging your device. Instead use a simple transistor 2N2222 or 2N3904 and ~1K resistor:

```
                            2N3904 (NPN)
                            +---------+
                            |         |     +-|
                            | E  B  C |    / S|
                            +-|--|--|-+    | P|
                              |  |  +------+ E|
                              |  |         | A|
ESP8266-GND ------------------+  |  +------+ K| 
                                 |  |      | E|
ESP8266-I2SOUT (Rx) -----/\/\/\--+  |      \ R|
                                    |       +-|
USB 5V -----------------------------+

You may also want to add a 220uF cap from USB 5V to GND just to help filter out any voltage droop during high volume playback.
```

 Known issues :
 ----
 - Playing GoogleTTS hang the ESP at end of playing (for both ESP8266 and ESP32) : [issue #395](https://github.com/earlephilhower/ESP8266Audio/issues/395)  (almost resolved)
 - <strike>Playing RTTTL (nokia tone) never stop playing the last note (ESP32 only) :</strike> [issue #327 resolved](https://github.com/earlephilhower/ESP8266Audio/issues/327)
 
 These issues are related to [ESP8266audio library](https://github.com/earlephilhower/ESP8266Audio) so it has been created on their repo ;)
 
 
 
  Use with Home Assistant :
 ----
 It is possible to use MrDiy Notifier as a media player entity.
 See my post on community forum for it :
 https://community.home-assistant.io/t/turn-an-esp8266-wemosd1mini-into-an-audio-notifier-for-home-assistant-play-mp3-tts-rttl/211499/130?u=schmurtz
 
 ## Thanks
You like this project ? You want to improve this project ? 

Do not hesitate, **Participate**, there are many ways :
- If you don't know aruidno language you can test releases , and post some issues, some tips and tricks for daily use.
- If you're a coder you can fork, edit and publish your modifications with Pull Request on github :)<br/>
- Or you can buy me a coffee to keep me awake during night coding sessions :dizzy_face: !<br/><br/>
[![Buy me a coffee][buymeacoffee-shield]][buymeacoffee]
<br/><br/>

[buymeacoffee-shield]: https://www.buymeacoffee.com/assets/img/guidelines/download-assets-sm-2.svg
[buymeacoffee]: https://www.buymeacoffee.com/schmurtz
 ===========================================================================
 #  Original description from MrDIY 
 
 (some functionalities exposed of the original project are now outdated but it gives a good overview).
 
 https://gitlab.com/MrDIYca/mrdiy-audio-notifier

In this project, I will show you how you can use an ESP8266 module or board like the Wemos D1 Mini to play MP3, TTS and RTTTL. It is controlled over MQTT. You will be able to send it an MQTT message with the URL of the MP3 file and it will play it for it. It is also capable to doing basic TTS and playback RTTTL (aka Nokia) ringtone.
<br><br><br>
**Watch the video**: click on the image below:

[![MrDIY Audio Notifier youtube video](https://img.youtube.com/vi/SPa9SMyPU58/0.jpg)](https://www.youtube.com/watch?v=SPa9SMyPU58)


## Instructions

Please visit <a href="https://www.instructables.com/id/MQTT-Audio-Notifier-for-ESP8266-Play-MP3-TTS-RTTL">Mr DIY Instructables page</a> page for full instructions or watch the video above.

<p>Check out <a href="https://www.youtube.com/channel/UCtfYdcn8F8wfRA2BXp2FPtg">Mr DIY YouTube channel</a>  and follow Mr DIY on <a href="https://twitter.com/MrDIYca">Twitter</a>.

## Thanks
Many thanks to all the authors and contributors to the open source libraries I have used. You have done an amazing job for the community!


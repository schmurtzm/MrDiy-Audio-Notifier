# MrDiy Audio Notifier - Schmurtz Edition
 MrDiy Audio Notifier (based on esp8266audio) with some modifications :
 ----
- Ported to platformio (with differents recommanded settings for ESP8266audio)
- Can be compiled for ESP8266 and ESP32
- Google Translate TTS (so a little less "cloudless" but multilingual and better voice than the included ESP8266SAM)
- ability to play AAC (required for many web radio)
- This code also allows to switch easily between no DAC (version used by mr DIY) , external DAC or internal DAC (for ESP32).
- You’ll also find some useful comments for wiring your DAC quickly or to improve the code (sound level of RTTTS, ssl, IotWebConf migration to v3.x...).

 Some issues for now :
 ----
 - Playing GoogleTTS crash at end (for both ESP8266 and ESP32) : [issue #327](https://github.com/earlephilhower/ESP8266Audio/issues/327)
 - <strike>Playing RTTTL (nokia tone) never stop playing the last note (ESP32 only) :</strike> [issue #395 resolved](https://github.com/earlephilhower/ESP8266Audio/issues/395)
 
 These issues are related to [ESP8266audio library](https://github.com/earlephilhower/ESP8266Audio) so it has been created on their repo ;)
 
 
 
 =============================================================
 #  Original description from MrDIY 
 https://gitlab.com/MrDIYca/mrdiy-audio-notifier

In this project, I will show you how you can use an ESP8266 module or board like the Wemos D1 Mini to play MP3, TTS and RTTL. It is controlled over MQTT. You will be able to send it an MQTT message with the URL of the MP3 file and it will play it for it. It is also capable to doing basic TTS and playback RTTL (aka Nokia) ringtone.
<br><br><br>
**Watch the video**: click on the image below:

[![MrDIY Audio Notifier youtube video](https://img.youtube.com/vi/SPa9SMyPU58/0.jpg)](https://www.youtube.com/watch?v=SPa9SMyPU58)


## Instructions

Please visit <a href="https://www.instructables.com/id/MQTT-Audio-Notifier-for-ESP8266-Play-MP3-TTS-RTTL">my Instructables page</a> page for full instructions or watch the video above.

<p>Don't forget to check out <a href="https://www.youtube.com/channel/UCtfYdcn8F8wfRA2BXp2FPtg">my YouTube channel</a>  and follow me on <a href="https://twitter.com/MrDIYca">Twitter</a>. If you are intersted in supporting my work, please visit <a href="https://www.patreon.com/MrDIYca?fan_landing=true">my Patreon page</a>.</p>


## Thanks
Many thanks to all the authors and contributors to the open source libraries I have used. You have done an amazing job for the community!


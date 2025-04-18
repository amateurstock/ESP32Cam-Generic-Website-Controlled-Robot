# ESP32Cam-Generic-Website-Controlled-Robot
Uses ESP-IDF's http server library, SPIFFS mounting, and camera drivers. Uses Arduino as its framework.

The source files and websites are there for you to copy-paste and modify for your own personal use. I just thought I'd put it up on my own Github just so that no other poor beginner has to go through the same amount of shit I went through getting everything in order. 

The original use for this was a Remote-Controlled Fire Extinguisher robot that provides a camera feed to the website it hosts. I've left a lot of those project components within the files. I've scratched through countless Javascript tutorials getting the joystick right. There are some incomplete code like the fact that it can take WASD keyboard inputs but never use them. That's really up to you if the project calls for it. I'm fairly sure I'm done with what I have here (unless I make up my mind in the future idk).

Uses HTTP GET queries (idk the correct terminology) to send values over to the hosted website. The ESP32 receives these GET requests and parses the query as a sort of input. Much, much earlier into this project, I did use POST, but it was so damn slow because it keeps having to send packed JSON over at a rate of 13.333... POSTs per second. It was only when I started looking into Espressif's implementation was when I switched over the GETs.

I also can't seem to get this to work with AsyncWebServer, something something too much memory used. Also I've already tried using yoursunny's esp32cam library, but its AsyncCam example was notoriously slow (it's apparently a common issue).

I've tried to credit every code snippet I've copied as best as I can. If I've forgotten to credit something else, idk put up an issue or something and I'll try and rectify it. 

My favicon handler implementation only supports PNGs I think. Do edit it to your project's needs.

This was all done in PlatformIO. I've had plenty of troubles using Arduino's IDE to warrant me to switch over to VSCode. 

# Possibility of a cleaner rewrite just for fun :>>

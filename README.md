### Setups

Set up your `arduino-cli` config like:

proxy_type: auto
sketchbook_path: .
arduino_data: .
board_manager:
    additional_urls:
        - https://arduino.esp8266.com/stable/package_esp8266com_index.json
        - https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

You have to set it up to use the ESP8266 

Way easier if you use Python 3 btw

```
arduino-cli core install esp8266:esp8266  
arduino-cli compile -b esp8266:esp8266:espectro blink/ 
```

You can get the port for your ESP device with 

```
ls /dev/tty.*
```

You could grep that to grab the USB serial but I'm too lazy right now

Then updload the artifact with

```
arduino-cli upload --port /dev/cu.usbserial-1420 -b esp8266:esp8266:espectro blink/
```

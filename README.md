[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)
![](https://github.com/pduck27/Smart-Meter-to-MQTT/workflows/BuildAndRelease/badge.svg)
![](https://img.shields.io/github/v/release/pduck27/Smart-Meter-to-MQTT)

# ESP Smartmeter IR Reader to MQTT
This ESP project monitors a smart power electricity meter named "DD3 2R06 ETA ODZ 01" (https://www.ebzgmbh.de/unsere-produkte/) with an IR Receiver "Hitchi TTL IR" (https://wiki.volkszaehler.org/hardware/controllers/ir-schreib-lesekopf-ttl-ausgang). 

![smart meter image](/ressource/unsere-produkte_dd3.jpg)

![ir receiver image](/ressource/s-l400.jpg)

Almost every 30 seconds the ESP tries to receive a full data stack and sends it via MQTT. I also included an optional LED which is switched on for several seconds when a full data stack was read. 

A full data stack send by the DD3 smart meter looks like this. The variable's key always starts with "*Number-Number : ...*" and the value is given in following brackets. The format and the content can be different for other meter models.

    1-0:0.0.0*255(XXXXXXXXXXXXXX) --> Your meter number
    1-0:96.1.0*255(XXXXXXXXXXXXXX) --> Meter number
    1-0:1.8.0*255(000089.46238305*kWh) --> Used energy total
    1-0:2.8.0*255(000006.10581520*kWh) --> Produced energy total
    1-0:16.7.0*255(000335.70*W) --> Current power total
    1-0:36.7.0*255(000130.50*W) --> Current power phase 1
    1-0:56.7.0*255(000160.47*W) --> Current power phase 2
    1-0:76.7.0*255(000044.73*W) --> Current power phase 3
    1-0:96.5.0*255(001C0104) --> Status
    0-0:96.8.0*255(0011A588) --> Time of operation (hex)
    !
    /EBZ5DD32R06ETA_107

Please be aware, most smart meters must be unlocked with a personal pin to send the full data stack.

# Setup
I use an ESP32DevKitv4, but any other ESP module should work as well. The IR receiver you can get on ebay, search for "hitchi ttl ir".
The received data stack is written to the console, so you can check it easily. 

For the first start you should adapt the file *Credentials_sample.h* from the include directory with your data and rename it to *Credentials.h*.

![credentials image](/ressource/shot2.png)


The code only checks for "Used energy total", "Produced energy total" and "Current power total". You can change it to your needs, just read the comments in the code for possible adjustments. I know that the way how I read the values is really "hard coded" but my regex c++ skills are too bad. So if somebody can make it better please push it.

# Licence
All code is licensed under the [MPLv2 License](https://github.com/pduck27/Smart-Meter-to-MQTT/blob/master/LICENSE).
Please recognize additional comments in the code.

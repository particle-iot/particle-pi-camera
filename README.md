#Particle Pi Camera Project

![](https://github.com/spark/particle-pi-camera/blob/master/images/header.jpg)

##Introduction


##Things needed
###Essentials
 - Raspberry Pi (3 preferred)
 - Raspberry Camera V2
 - uSD card
 - PIR sensor
 - Neopixel ring
 - Micro USB cable

###For the build
(If you plan to build the enclosure)
 - Acrylic, MDF or Plywood sheet (3 mm thick)
 - M3x12 screws and nuts (4)
 - M2.5X12 screws and nuts (4)
 - M2x12 screws and nuts (4)
 - Access to Laser cutter
 - Soldering tools and hardware

##Setting up your Raspberry Pi


### Preparing your memory card
You can skip these steps if you already have setup your Pi.

  - Make sure your SD card is FAT32 formatted
  - Install an operating system image on the SD card. 
  - Download here (RASPBIAN JESSIE WITH PIXEL): https://www.raspberrypi.org/downloads/raspbian/
  - Instructions are here: https://www.raspberrypi.org/documentation/installation/installing-images/README.md 

### Setup your WiFi connection 
  
  - Connect to Pi over SSH. I connected the Pi over ethernet to a local router and had my computer and phone connect to the same router as well (over wifi). I then used Fing app on my phont to get Pi’s IP address.
  - Setup the WiFi using the steps given here: https://www.raspberrypi.org/documentation/configuration/wireless/wireless-cli.md
  
I modified the file to be:
```
network={
    ssid="YourWiFiSSDID"
    psk="YourPassword"
    key_mgmt=WPA-PSK
}
```

### Install the particle agent

(More detail on the particle agent can be found here: https://github.com/spark/particle-agent)
  - Install the particle agent using the following in your terminal: 
```
bash <( curl -s https://raw.githubusercontent.com/spark/particle-agent/master/bin/cloud-install )
```

  - At the end of installation, it will ask for your particle account email and password.

  - You may have to manually place the tinker firmware if things don't work in the first attempt. Use the following three commands to place the tinker:
```
sudo service particle-agent stop
sudo cp /usr/share/particle/binaries/tinker /var/lib/particle/devices/yourDeviceID/firmware.bin
sudo service particle-agent start
```

  - Use Particle CLI to verify if the Pi is claimed to your account and connected to the cloud.
```
particle list
```
  - Test the tinker by connecting an LED to pin of the Pi and control it via particle CLI.

### Setup the Pi camera
  - Connect the Pi camera and enable it via raspi-config
https://thepihut.com/blogs/raspberry-pi-tutorials/16021420-how-to-install-use-the-raspberry-pi-camera
  - Verify it by using raspistill command and see if an image is captured
```
raspistill -vf -hf -o /home/pi/picam/selfie.jpg
```
  
### Setup the Dropbox API
  - Follow instructions to install dropbox uploader on adafruit
https://learn.adafruit.com/cloud-cam-connected-raspberry-pi-security-camera/dropbox-sync?view=all
  - Test the dropbox uploader by uploading the above image or any other file on your Pi.
  - Make and test run a shell script to capture an image and then upload it to dropbox.

Learn how to write and run shell scripts on RasPi here: http://www.raspberry-projects.com/pi/pi-operating-systems/raspbian/scripts

I used this shell script. (your directory structure might be different)

```sh
#!/bin/bash

echo "running shell script"
DATE=$(date +"%Y-%m-%d_%H%M")
echo "capturing image"
raspistill -vf -hf -o /home/pi/picam/$DATE.jpg
echo "uploading image"
/usr/local/bin/dropbox_uploader upload /home/pi/picam/$DATE.jpg "camera/"
```

### Compile your user app

  - You can use particle CLI to compile your user app for the Pi
```
particle compile raspberrypi
```
  - To compile and flash
```
particle flash my_pi
```

In order for you to compile the user app locally, you'll need to install docker for your computer. 

  - Install docker for your machine: https://docs.docker.com/engine/installation/
  - Follow instructions on community alpha pi page to be able to compile locally http://community.particle.io/t/alpha-phase-setup-instructions-for-raspberry-pi/26305
  - Try writing a simple app to blink an LED before moving on to a more complicated example.
```cpp
/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "stdarg.h"

PRODUCT_ID(PLATFORM_ID);
PRODUCT_VERSION(3);

#if Wiring_WiFi
STARTUP(System.enable(SYSTEM_FLAG_WIFITESTER_OVER_SERIAL1));
#endif

SYSTEM_MODE(AUTOMATIC);

/* This function is called once at start up ----------------------------------*/
void setup()
{   /* Connect an LED between GPIO4 of the Pi and ground with a 1K resistor*/
    pinMode(4,OUTPUT);          //pi GPIO4   
}

/* This function loops forever --------------------------------------------*/
void loop()
{
  digitalWrite(4,HIGH);
  delay(500);
  digitalWrite(4,LOW);
  delay(500);
}
```

### Connect you Neopixel ring

We used these cool neopixel rings in out project: https://www.adafruit.com/products/2855

  - Connect the positive supply of the ring to +5V on the Pi, GND to GND and input pin of the neopixel ring to GPIO18 of the Pi
  - Use this modified version of the neopixel library to control the ring. (add link here)
  - Use the following app to test the ring

```cpp
#include "application.h"
#include "ws2811.h"

#define TARGET_FREQ  WS2811_TARGET_FREQ
#define GPIO_PIN     18
#define DMA          5
#define STRIP_TYPE   SK6812_STRIP_RGBW    // SK6812RGBW (NOT SK6812RGB)
#define LED_COUNT    16

ws2811_t ledstring = {
    NULL,
    NULL,
    TARGET_FREQ,
    DMA,
    {
        {
            GPIO_PIN,
            0,
            LED_COUNT,
            STRIP_TYPE,
            NULL,
            255,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            NULL,
            0,
            0,
            0,
            0,
            0,
        },
    },
};


void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

void setup()
{
  ws2811_init(&ledstring);
}

void loop() {
  rainbow(20);
}

uint8_t brightness = 32;
uint32_t color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
  return 
    ((((uint32_t)w * brightness) >> 8) << 24) |
    ((((uint32_t)r * brightness) >> 8) << 16) |
    ((((uint32_t)g * brightness) >> 8) << 8) |
    ((((uint32_t)b * brightness) >> 8));
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<LED_COUNT; i++) {
      ledstring.channel[0].leds[i] = Wheel((i+j) & 255);
    }
    ws2811_render(&ledstring);
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
```

### Putting it all together

Now that we have tested all of the hardware and integrations, let's try to put it all together.

```cpp
/*
 ******************************************************************************
  Copyright (c) 2015 Particle Industries, Inc.  All rights reserved.


  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.


  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.


  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "ws2811.h"
#include "stdarg.h"

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

#define TARGET_FREQ  WS2811_TARGET_FREQ
#define GPIO_PIN     18
#define DMA          5
#define STRIP_TYPE   SK6812_STRIP_RGBW    // SK6812RGBW (NOT SK6812RGB)
#define LED_COUNT    32

ws2811_t ledstring = {
    NULL,
    NULL,
    TARGET_FREQ,
    DMA,
    {
        {
            GPIO_PIN,
            0,
            LED_COUNT,
            STRIP_TYPE,
            NULL,
            255,
            0,
            0,
            0,
            0,
        },
        {
            0,
            0,
            0,
            0,
            NULL,
            0,
            0,
            0,
            0,
            0,
        },
    },
};


void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
uint32_t color(uint8_t r, uint8_t g, uint8_t b, uint8_t w);


bool pirState = LOW;

void setup()
{
  ws2811_init(&ledstring);
  pinMode(26,INPUT_PULLDOWN); //pi GPIO17 
  for(uint8_t i=0; i<LED_COUNT; i++) {
      ledstring.channel[0].leds[i] = color(0,100,0,0);
    }
    ws2811_render(&ledstring);
    delay(500);
}


void loop()
{
    //This will run in a loop
    pid_t child_pid;
    int status;

    int value = digitalRead(26);

    if (value == HIGH) {    // check if the input is HIGH
    //digitalWrite(4, HIGH);  // turn LED ON
    for(uint8_t i=0; i<LED_COUNT; i++) {
      ledstring.channel[0].leds[i] = color(250,250,250,250);
    }
    ws2811_render(&ledstring);
    delay(20);

    if (pirState == LOW) {
          // we have just turned on
            child_pid = fork();
            if (child_pid == 0) /* fork() returns 0 for the child process */
            {

                char *const cmd[] = {"",NULL};
                execv("/home/pi/picam/bin/camera-capture",cmd); // image capture shell script

             }
             else /* parent process */
             {
                 wait(&status); /* wait for child to exit, and store child's exit status */
                 exit(0);  /* parent exits */
             }
          pirState = HIGH;
        }
    } else {
    //digitalWrite(4, LOW); // turn LED OFF
    for(uint8_t i=0; i<LED_COUNT; i++) {
      ledstring.channel[0].leds[i] = color(0,0,0,0);
    }
    ws2811_render(&ledstring);
    delay(20);
    if (pirState == HIGH)
      pirState = LOW;
  }

}

uint8_t brightness = 32;
uint32_t color(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
  return 
    ((((uint32_t)w * brightness) >> 8) << 24) |
    ((((uint32_t)r * brightness) >> 8) << 16) |
    ((((uint32_t)g * brightness) >> 8) << 8) |
    ((((uint32_t)b * brightness) >> 8));
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<LED_COUNT; i++) {
      ledstring.channel[0].leds[i] = Wheel((i+j) & 255);
    }
    ws2811_render(&ledstring);
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
```

  - This is the shell script that gets called from the user app:

```sh
#!/bin/bash

echo "running shell script"
DATE=$(date +"%Y-%m-%d_%H%M")
echo "capturing image"
raspistill -vf -hf -o /home/pi/picam/$DATE.jpg
echo "uploading image"
/usr/local/bin/dropbox_uploader upload /home/pi/picam/$DATE.jpg "camera/"
```

##The build

This is an optional enclosure that you can build if you have access to a laser cutter. All of it was cut on a 3mm white acrylic sheet. You can use MDF or plywood instead. (Link to the AI files.) (image of the circuit)

![](https://github.com/spark/particle-pi-camera/blob/master/images/image01.jpg)

  - Attach the Pi to the base plate using M2.5 screws and nuts

![](https://github.com/spark/particle-pi-camera/blob/master/images/image02.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image03.jpg)

  - Attach the Pi camera and the PIR sensor using M2 screws and nuts

![](https://github.com/spark/particle-pi-camera/blob/master/images/image04.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image05.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image06.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image07.jpg)

  - Glue the panels to the base plate using super glue or other fast curing epoxy glues.

![](https://github.com/spark/particle-pi-camera/blob/master/images/image10.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image11.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image12.jpg)

  - Attach the top plate and secure it to the bottom using M3 screws

![](https://github.com/spark/particle-pi-camera/blob/master/images/image13.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image14.jpg)
![](https://github.com/spark/particle-pi-camera/blob/master/images/image15.jpg)

  - Power it up with a USB cable 

![](https://github.com/spark/particle-pi-camera/blob/master/images/image16.jpg)

  - Mount it on a tripod and you are good to go! I glued a GoPro tripod mount bracket to the base of the enclosure.

![](https://github.com/spark/particle-pi-camera/blob/master/images/image17.jpg)







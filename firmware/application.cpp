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
                 //exit(0);  /* parent exits */
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

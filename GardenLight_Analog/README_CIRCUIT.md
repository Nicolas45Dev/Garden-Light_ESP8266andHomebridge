# ESP8266 Circuit

The circuit to build is quite simple. The ESP8266 has diffrent pins that offer PWN Control. In order to control the strip, we can dim the light using the analogWrite function.

![](ESP8266pin.jpg)

This example show you how to  hook up the controller. For the ESP8266, I use the pins 12, 14 and 15. They can all be control with PWN. In case, I use NPN transistors, because they are easier to use than PNP. Make sure
the transistors you are using can handle the current of the strip. My strip uses 500 mA per channel for each unit and I built 7 of them. So, I took three BUT92 transistors, because they can handle 60 amp @ 250V. I can add a lot of lights before having a problem.

![](Circuit.png)


## About the Sketch
In the sketch, there is a function named OpenSide() that controls the transistion between the actual color and the new color to be set. You can adjust the duration of the effect by changing the EffectDuration variable.
Everything else should be quite easy to understand, the code isn't that big. So keep exploring and have fun.

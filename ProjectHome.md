## Intro ##
Given all my previous experience with Geiger Counters (see the [uRadMonitor project](http://uradmonitor.pocketmagic.net)), I have decided to conceive a portable, easy to build, radiation dosimeter. I didn't have much need for yet another dosimeter (with my Radex 1706 or Terra-P), but I know there are several people not affording a commercial unit, but wanting to have their own radiation measurement tool. So here is this project, giving you just that: **an efficient, stable, easy to build construction for all your radiation dosimetry needs**.

## Short description ##
I've started from scratch, designing a complete dosimeter unit around the Atmel Atmega8 microcontroller and a Russian Geiger Muller tube. Here you'll see the STS-1 tube, for high gamma doses, but the dosimeter can be used with any other tubes such as SBM-20, LND-712 or more sensitive ones such as the SBM-19 or the pancake tube SI-14B. Changing the tube requires changing the software, to adjust the dose conversion calculation. This circuit can be used with almost any geiger tube, as even the inverter's output voltage driving the tube is adjustable in the software.

I've been invited to show this dosimeter on a TV Show, on TVR Timisoara (video is in Romanian):
<a href='http://www.youtube.com/watch?feature=player_embedded&v=WXmPR-FNbIg' target='_blank'><img src='http://img.youtube.com/vi/WXmPR-FNbIg/0.jpg' width='425' height=344 /></a>


## Functionality ##
Similar to my uRADMonitor, the microcontroller takes care of everything:<br />
1. Generates a variable duty PWM signal using Timer1, to drive the 400V inverter needed to operate the Geiger tube; The invertor doesn't need a multiplier, as the ferrite's transformer secondary puts out exactly the amount required. The transformer is made on a A22 ferrite core, with 16 turns in the primary and 600 in the secondary.<br />
2. Uses one ADC port to measure the inverter's voltage and adjust the PWM duty cycle for constant output (exactly 400V for stable operation)<br />
3. Counts the time, using Timer0 so we can compute the dose<br />
4. Uses interrupt INT0 to count the pulses produced by the Geiger tube<br />
5. Drives a 2x16 LCD to output the results.<br />
Some other circuits on the Internet come with improper 400V inverters (some people seem not to be able to design a proper inverter), they are redundant (using 555's and additional components, when the microcontroller can take care of EVERYTHING), use the wrong signal detection/counter circuit, or other small defects that result in wrong measurements. Not to mention the controversial aspect of computing dose in sieverts out of counts per minute.<br />
Given all the wrong designs out there, my detector tries to fill in some of the gaps. So here is exactly what you need: a stable design, with several improvements made over time, all packed in this nice construction that you can easily replicate. <br />
[![](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_dosimeter_06-150x150.jpg)](http://www.pocketmagic.net/?p=3342) [![](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_dosimeter_11-150x150.jpg)](http://www.pocketmagic.net/?p=3342) [![](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_dosimeter_17-150x150.jpg)](http://www.pocketmagic.net/?p=3342) [![](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_dosimeter_20-150x150.jpg)](http://www.pocketmagic.net/?p=3342)

## Circuit diagram ##
![![](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_schematics-300x153.png)](http://www.pocketmagic.net/wp-content/uploads/2012/12/geiger_counter_schematics.png)

## License ##
This work is free software, licensed under GPL v2; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. For other licensing options, feel free to contact me.

## Additional resources ##
More details available on my blog: http://www.pocketmagic.net/?p=3342
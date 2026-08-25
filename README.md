# Lock In Amplifier
Measures how much 1kHz light is getting from the LED to the photodiode and ignores everything else.
The trick is that the same 16kHz timer tick both drives the LED and steps the reference. 
The LED flips every 8 ticks, giving 1kHz with 16 samples per cycle. 
Every sample gets multiplied by +1 or -1 against two references 90 degrees apart and piled into two accumulators.

The sketch is in the lockin folder, it runs on a Uno or Nano. 
D8 -> 220 ohm resistor -> LED, then A0 back from the photodiode amp.

## Running it
Choose 115200 baud in the serial monitor, one line a second: X Y R theta
One thing about theta, the sketch increments the phase counter before it writes the LED port, so the light leads the reference by one tick. 
That's 62.5us out of a 1000us cycle, so everything reads 22.5 degrees off. Doesn't affect R at all, which is why I never bothered fixing it
X and Y are the two quadrature components in volts. R is sqrt(X^2+Y^2) and theta is the phase lag the amp adds.
R never reaches zero. It's a magnitude, so it can't go negative, with the beam blocked, X and Y wander around zero but R is the length of that wander.
So your blocked-beam number is a real offset.
It is not a commercial lock-in, it doesn't quote nanovolts, it measures microvolts at best.

## Checking it works
Cover the diode with the LED off, R should be small and sit still. LED running, clear path, R jumps to something obvious. Hand in the beam, back to baseline.
Switch the room lights on and off, R should barely twitch.

## Mistakes that are hard to detect:
- A clipping amp gives a beautiful reading.
- It responds to odd harmonics, the square reference isn't a pure 1kHz.
- Ambient rejection will not be infinite, enough of the ambient light can push the amp towards its rail, which can lead to the clipping case.
- Not shielding the diode!!!!

##Calibration
R is relative. For absolute figures, measure against a known optical density and derive the scale factor.

## Any changes
NCYCLES sets the averaging window in 1kHz cycles, 1000 is a seconds, 250 is 0.25s, livelier display but twice the noise. 
Changing 

## Schematic 
  VBIAS REFERENCE  (3.0 V)

      +9V ----/\/\/\----/\/\/\----+----/\/\/\---- GND
                10k       10k     |      10k
                                  |
                                  +--------> VBIAS
                                  |
                                 ---
                                 --- 100uF
                                  |
                                 GND


  STAGE 1 - TRANSIMPEDANCE  (TL072 section A)

                    +--------/\/\/\--------+
                    |          1M          |
                    +-------- --| |-- -----+
                    |    10pF (often not needed)
                    |                      |
       +9V          |                      |
        |           |    |\                |
     [BPW34]        |    |  \              |
        |           |    |    \            |
        +-----------+----|-     \          |
                         |       >---------+-----> TIA OUT (pin 1)
       cathode to +9V    |     /
       = reverse biased  |   /
                    VBIAS---|+
                         |/    TL072 A


  STAGE 2 - GAIN x11  (TL072 section B)

               1uF
  TIA OUT -----| |-----+----------------+
                       |                |    |\
                     [100k]             |    |  \
                       |                +----|+   \
                     VBIAS                   |     \
                                             |      >----+----> GAIN OUT
                                        +----|-    /     |     (pin 7)
                                        |    |   /       |
                                        |    | /         |
                                        |    |/          |
                                        |  TL072 B       |
                                        |                |
                                        +----/\/\/\------+
                                        |      100k
                                      [10k]
                                        |
                                      VBIAS


  STAGE 3 - LEVEL SHIFT AND ANTI-ALIAS

                                 5V
                                  |
                                [10k]
               10uF               |              220
  GAIN OUT -----| |---------------+-----------/\/\/\-----+------> A0
                                  |                      |
                                [10k]                   ---
                                  |                     --- 100nF
                                 GND                     |
                                                        GND


  MODULATOR

      D8 ----/\/\/\----->|---- GND
               220       LED


  SUPPLY

      +9V ------ TL072 pin 8 ------+
                                   |
                                  ---
                                  --- 100nF   within 1-2 cm of the pin
                                   |
      GND ------ TL072 pin 4 ------+

      9V battery (-), Arduino GND and the GND rail are all one node.
      Arduino 5V feeds the output divider only -- never the +9V rail.
## How it works
Photocurrent fights 1/f noise, thermal drift, ambient light and op-amp offset, the lock in amplifier erases 
the issue by moving the measurement to a frequency of choice. This exact system has three key phases:
- Modulation, where the D8 pin chops the LED at 1kHz, or any other frequency of choice, preferably a prime number.
- Detection, the photodiode feeds 1 MΩ resistor, then a x11 AC-coupled gain stage.
- Demodulation, each sample is multiplied by ±1 against a reference, the reference comes from the same counter driving the LED,
  it gets averaged. The accumulated signal correlates to the amount of light that reaches the photodiode.

## Materials
Qty	Part	Notes
1	TL072CP	dual JFET-input op-amp
1	BPW34	photodiode, reverse-biased
1	LED	any colour
5	10 kΩ	3× VBIAS divider, 2× 2.5 V divider
1	1 MΩ	transimpedance feedback
2	100 kΩ	gain stage
2	220 Ω	LED drive, anti-alias
1	100 µF	VBIAS bypass
1	10 µF	output coupling
1	1 µF	interstage coupling
2	100 nF	decoupling, anti-alias
1	10 pF	feedback — often unnecessary
1	9 V battery	~4 mA draw

## Next steps
After building the instrument, if you enjoyed building it and want to go further, you can use it to do absorbance, turbidity,
thermal time constants, etc., you can even swap the LED for a coil and try to do impedance and proximity sensing. 
You can use a diffraction grating to try and make the system into a spectrometer.

Thank you for your time and patience, I hope you liked this repo and wish you success in your next projects ;)

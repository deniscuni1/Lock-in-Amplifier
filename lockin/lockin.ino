// lock-in amp, ATmega328 @ 16MHz
// D8 -> 220R -> LED,  A0 <- photodiode amp output
//
// Timer1 ticks at 16kHz and the LED flips every 8 ticks, so the
// reference is just the tick counter. Nothing to sync, nothing to trim.

#include <math.h>

#define LED_BIT  PB0      // D8
#define NCYCLES  1000     // averaging window in 1kHz cycles. 250 is livelier while poking at it.

const uint32_t WINDOW = 16UL * NCYCLES;

volatile int32_t  accI, accQ;
volatile uint32_t n;

volatile int32_t  outI, outQ;
volatile uint32_t outN;
volatile bool     ready;

static uint8_t ph;        // 0..15, ISR only


ISR(TIMER1_COMPA_vect)
{
  // Take last tick's conversion and immediately start the next.
  // Waiting on ADSC in here costs 26us and it isn't needed.
  int16_t s = ADC;
  ADCSRA |= _BV(ADSC);

  if (ph < 8)             accI += s; else accI -= s;
  if (ph >= 4 && ph < 12) accQ += s; else accQ -= s;

  ph = (ph + 1) & 15;

  if (ph < 8) PORTB |=  _BV(LED_BIT);
  else        PORTB &= ~_BV(LED_BIT);

  if (++n >= WINDOW) {
    outI = accI;
    outQ = accQ;
    outN = n;
    accI = 0; accQ = 0; n = 0;
    ready = true;
  }
}


void setup()
{
  Serial.begin(115200);

  DDRB  |=  _BV(LED_BIT);
  PORTB &= ~_BV(LED_BIT);

  ADMUX  = _BV(REFS0);                            // AVcc ref, A0
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);   // /32 -> 500kHz, 26us/conv
  ADCSRA |= _BV(ADSC);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = 999;                       // 16e6 / 1000 = 16kHz
  TCCR1B = _BV(WGM12) | _BV(CS10);
  TIMSK1 = _BV(OCIE1A);
  interrupts();

  Serial.println(F("X\tY\tR\ttheta"));
}


void loop()
{
  if (!ready) return;

  int32_t  i, q;
  uint32_t nn;

  noInterrupts();
  i  = outI;
  q  = outQ;
  nn = outN;
  ready = false;
  interrupts();

  // counts -> volts. Multiply first, i is signed and nn isn't.
  float X = i * (5.0f / 1024.0f) / nn;
  float Y = q * (5.0f / 1024.0f) / nn;

  float R = sqrtf(X * X + Y * Y);

  Serial.print(X, 5);  Serial.print('\t');
  Serial.print(Y, 5);  Serial.print('\t');
  Serial.print(R, 5);  Serial.print('\t');
  Serial.println(atan2f(Y, X) * 57.29578f, 1);
}

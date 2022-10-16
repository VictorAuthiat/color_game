#define LED_R 2
#define LED_G 3
#define LED_B 4

#define PIN_R 11
#define PIN_G 10
#define PIN_B 9

#define THRES 200
#define SAMPLES 20

typedef struct touchPad {
  int pin;
  int unpValue;
  int value;
  char state = 0;
  char prevState = 0;
  char toggleState = 0;
};

touchPad touchPadR, touchPadG, touchPadB;

void setup() {
  pinMode(A0, INPUT_PULLUP);
  analogRead(A0);

  pinMode(LED_R, OUTPUT);
  digitalWrite(LED_R, 0);
  pinMode(LED_G, OUTPUT);
  digitalWrite(LED_G, 0);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_B, 0);

  touchPadInit(&touchPadR, A1);
  touchPadInit(&touchPadG, A2);
  touchPadInit(&touchPadB, A3);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  startLeds(3);
}

void loop() {
  touchPadScan(&touchPadR);
  touchPadScan(&touchPadG);
  touchPadScan(&touchPadB);

  digitalWrite(LED_R, touchPadR.toggleState);
  digitalWrite(LED_G, touchPadG.toggleState);
  digitalWrite(LED_B, touchPadB.toggleState);

  writeRgbColor(touchPadR.toggleState,
                touchPadG.toggleState,
                touchPadB.toggleState);

  delay(10);
}

void startLeds(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_R, 1);
    delay(100);
    digitalWrite(LED_G, 1);
    delay(100);
    digitalWrite(LED_B, 1);
    delay(100);
    digitalWrite(LED_B, 0);
    delay(100);
    digitalWrite(LED_G, 0);
    delay(100);
    digitalWrite(LED_R, 0);
    delay(100);
  }
}

void writeRgbColor(int r_value, int g_value, int b_value) {
  int r_level = r_value == 0 ? 0 : 255;
  int g_level = g_value == 0 ? 0 : 255;
  int b_level = b_value == 0 ? 0 : 255;

  analogWrite(PIN_R, r_level);
  analogWrite(PIN_G, g_level);
  analogWrite(PIN_B, b_level);
}

void touchPadInit(touchPad *pad, int pin) {
  pad->pin = pin;
  pad->unpValue = (sampleB(pin) - sampleA(pin));
  DIDR0 |= 1;
  DIDR0 |= 1 << (pin - A0);
}

int sampleA(int sensePin) {
  ADMUX = 0b01000000;

  pinMode(sensePin, OUTPUT);
  digitalWrite(sensePin, 0);
  pinMode(sensePin, INPUT);

  ADMUX = 0b01000000 | sensePin - A0;

  ADCSRA |= 1 << ADSC;
  while ((ADCSRA & (1 << ADSC)) != 0)
    ;

  return ADC;
}

int sampleB(int sensePin) {
  ADMUX = 0b01001111;

  pinMode(sensePin, INPUT_PULLUP);
  pinMode(sensePin, INPUT);

  ADMUX = 0b01000000 | sensePin - A0;

  ADCSRA |= 1 << ADSC;
  while ((ADCSRA & (1 << ADSC)) != 0)
    ;

  return ADC;
}

void touchPadScan(touchPad *pad) {
  static float A, B;

  A = 0;
  B = 0;

  for (int i = 0; i < SAMPLES; i++) {
    A += sampleA(pad->pin);
    B += sampleB(pad->pin);
  }

  A /= SAMPLES;
  B /= SAMPLES;

  pad->value = (B - A);

  if (pad->value > (pad->unpValue + THRES)) {
    pad->state = 1;
  } else {
    pad->state = 0;
    pad->unpValue = ((float)pad->unpValue * 0.9) + ((float)pad->value * 0.1);
  }

  if (pad->state == 1 && pad->prevState == 0) {
    pad->toggleState = !pad->toggleState;
  }

  pad->prevState = pad->state;
}

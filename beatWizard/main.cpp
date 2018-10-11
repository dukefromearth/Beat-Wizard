#include <PololuLedStrip.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<11> ledStrip;
MPU6050 tilt(Wire);

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 100
rgb_color colors[LED_COUNT];

int cmd, note, value;
int degreeOfMotion = 90;
int prevNote = 0, currNote = 1;
int currVelocity = 0, prevVelocity = 1;
int currEffect = 0, prevEffect = 1;
int octave = 3 ;
int numAvailableNotes = 8; //1 octaves
int numOctavesForAvailableNotes = 3;

//octave shifts
enum {octaveUp = 12, octaveDown = -12 };

//note steps
enum {whole = 2, half = 1};

//effect codes
enum {onOff = 144,
    modulate = 1,
    expression = 11,
    breath = 2,
    pan = 10};

int effect[] = {modulate,
    breath,
    expression,
    pan};

int selectedEffect;

//button pins
const byte btnNoteOn = 7;
const byte knobSelectEffect = A1;


//button states
byte btnNoteOnState = 0;
int knobSelectEffectState = 0;

//middleC
int baseNote = 60;

int majorKey[] = {baseNote,
    baseNote + 2,
    baseNote + 4,
    baseNote + 5,
    baseNote + 7,
    baseNote + 9,
    baseNote + 11};



void setup() {
    Serial.begin(115200);
    Wire.begin();
    tilt.begin();
    tilt.calcGyroOffsets(true);
    pinMode(btnNoteOn, INPUT);
    
}

void loop() {
    tilt.update();
    
    btnNoteOnState = digitalRead(btnNoteOn);
    
    
    if(btnNoteOnState == 1){
        //turn off note and effect
        noteOn(144,currNote,0);
    } else {
        holdNoteOn();
        delay(10);
        // Update the colors.
        byte time = millis() >> 2;
        for (uint16_t i = 28; i < LED_COUNT; i++)
        {
            byte x = time - 8 * i;
            colors[i] = rgb_color(x, 255 - x, x);
        }
        
        // Write the colors to the LED strip.
        ledStrip.write(colors, LED_COUNT);
        
        delay(10);
    }
    
    
}


//holds the current note
void holdNoteOn() {
    
    //get note value
    currNote = getPitch(tilt.getAngleX());
    
    //get volume value
    currVelocity = getVolume(tilt.getAngleY());
    
    //get effect number
    selectedEffect = calculateEffect();
    
    //get effect value
    currEffect = getEffect(tilt.getAngleZ());
    
    
    //sends effect message
    if (currEffect != prevEffect){
        lightsOn(selectedEffect,21,27);
        noteOn(176,effect[selectedEffect],currEffect);
        prevEffect = prevEffect;
    }
    
    //sends volume message
    if (currVelocity != prevVelocity){
        int numLights = calculateNumLights(currVelocity);
        clearColors(numLights, 20);
        lightsOn(1,1,numLights);
        //noteOn(176,currNote,0);
        noteOn(176,7,currVelocity);
        prevVelocity = currVelocity;
    }
    
    
    //sends pitch message
    if (currNote != prevNote){
        
        //turn off previous note and effect
        noteOn(144,prevNote,0);
        
        //send pitch and volume
        noteOn(144,currNote,currVelocity);
        
        prevNote = currNote;
    }
}

int calculateNumLights(int velocity){
    for(int i = 0; i < 127; i = i+6){
        if (i > velocity) return i/6;
    }
    return velocity;
}

//based on knob rotation, selects an effect
int calculateEffect() {
    int value = analogRead(knobSelectEffect);
    if (value < 250) value = 0;
    else if (value < 500) value = 1;
    else if (value < 750) value = 2;
    else value = 3;
    return value;
}

int getEffect(int z){
    return convertAngleToValue(z);
}

int getVolume(int y){
    return convertAngleToValue(y);
}


int getPitch(int x) {
    
    //used to limit range of motion
    if (x > degreeOfMotion) x = degreeOfMotion;
    if (x <= -degreeOfMotion) x = -degreeOfMotion;
    //
    //  Serial.print("x:");
    //  Serial.println(x);
    //  Serial.print("degree:");
    //  Serial.println(degreeOfMotion);
    
    //returns a value between 1 and the number of available notes
    int pitch = (abs(x) * (numAvailableNotes)) / degreeOfMotion;
    //  Serial.print("pitch:");
    //  Serial.println(pitch);
    int currOctave = pitch/8;
    //  Serial.print("currOctave:");
    //  Serial.println(currOctave);
    pitch = majorKey[pitch%8] + currOctave*12;
    //  Serial.print("pitch2:");
    //  Serial.println(pitch);
    return pitch;
}

int convertAngleToValue(int angle) {
    
    //used to limit range of motion
    if (angle > degreeOfMotion) angle = degreeOfMotion;
    else if (angle <= -degreeOfMotion) angle = -degreeOfMotion;
    
    int value = (abs(angle) * 127) / degreeOfMotion;
    
    if (angle <= 0 && angle >= -degreeOfMotion) {
        value = 63 + value;
    } else {
        value = 63 - value;
    }
    return value;
}


//sends midi signal to serial
void noteOn(int cmd, int pitch, int value) {
    Serial.write(cmd);
    Serial.write(pitch);
    Serial.write(value);
}


//clears colors from the LED's
void clearColors(int firstLight, int lastLight) {
    rgb_color noColor;
    noColor.red = 0;
    noColor.green = 0;
    noColor.blue = 0;
    // Update the colors buffer.
    for (uint16_t i = firstLight; i < lastLight; i++)
    {
        colors[i] = noColor;
    }
    
    // Write to the LED strip.
    ledStrip.write(colors, LED_COUNT);
}

void lightsOn(int colorValue, int firstLight, int lastLight){
    rgb_color whiteColor;
    whiteColor.red = 255;
    whiteColor.green = 255;
    whiteColor.blue = 255;
    rgb_color redColor;
    redColor.red = 255;
    redColor.green = 0;
    redColor.blue = 0;
    rgb_color orangeColor;
    orangeColor.red = 255;
    orangeColor.green = 127;
    orangeColor.blue = 0;
    rgb_color yellowColor;
    yellowColor.red = 255;
    yellowColor.green = 255;
    yellowColor.blue = 0;
    rgb_color greenColor;
    greenColor.red = 0;
    greenColor.green = 255;
    greenColor.blue = 0;
    rgb_color blueColor;
    blueColor.red = 255;
    blueColor.green = 255;
    blueColor.blue = 0;
    rgb_color indigoColor;
    indigoColor.red = 255;
    indigoColor.green = 127;
    indigoColor.blue = 0;
    rgb_color violetColor;
    violetColor.red = 255;
    violetColor.green = 0;
    violetColor.blue = 0;
    for (uint16_t i = firstLight; i < lastLight; i++)
    {
        if (colorValue == 1){
            colors[i]=redColor;
        } else if (colorValue == 2){
            colors[i]=blueColor;
        } else if (colorValue == 3) {
            colors[i]=greenColor;
        } else {
            colors[i]=yellowColor;
        }
    }
    
    // Write to the LED strip.
    ledStrip.write(colors, LED_COUNT);
    
}


//given a starting point, bursts outward
void rainbowBurstFromPoint(int startingPoint) {
    
    
    int head = startingPoint;
    int tail = startingPoint;
    int currColor = 0;
    
    rgb_color whiteColor;
    whiteColor.red = 255;
    whiteColor.green = 255;
    whiteColor.blue = 255;
    rgb_color redColor;
    redColor.red = 255;
    redColor.green = 0;
    redColor.blue = 0;
    rgb_color orangeColor;
    orangeColor.red = 255;
    orangeColor.green = 127;
    orangeColor.blue = 0;
    rgb_color yellowColor;
    yellowColor.red = 255;
    yellowColor.green = 255;
    yellowColor.blue = 0;
    rgb_color greenColor;
    greenColor.red = 0;
    greenColor.green = 255;
    greenColor.blue = 0;
    rgb_color blueColor;
    blueColor.red = 255;
    blueColor.green = 255;
    blueColor.blue = 0;
    rgb_color indigoColor;
    indigoColor.red = 255;
    indigoColor.green = 127;
    indigoColor.blue = 0;
    rgb_color violetColor;
    violetColor.red = 255;
    violetColor.green = 0;
    violetColor.blue = 0;
    
    
    rgb_color rainbow[] = {
        whiteColor, redColor, orangeColor, yellowColor, greenColor, blueColor, indigoColor, violetColor
    };
    
    //initiateFirstColor
    colors[startingPoint] = whiteColor;
    ledStrip.write(colors, LED_COUNT);
    
    //burst from starting point outward in rainbow pattern until end of lights is reached
    for (int i = 0; i < LED_COUNT; ++i) {
        //reset the color counter
        if (startingPoint + i > LED_COUNT || startingPoint - i <= 0 ) break;
        //advance the tail LED
        colors[startingPoint - i] = rainbow[currColor++];
        //advance the head LED
        colors[startingPoint + i] = rainbow[currColor++];
        //fadeColors();
        ledStrip.write(colors, LED_COUNT);
        delay(50);
    }
    fadeColors();
}

void fadeColors() {
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        if (colors[i].red > 2) colors[i].red = sqrt(colors[i].red);
        else colors[i].red = 0;
        if (colors[i].green > 2) colors[i].green = sqrt(colors[i].green);
        else colors[i].red = 0;
        if (colors[i].blue > 2) colors[i].blue = sqrt(colors[i].blue);
        else colors[i].red = 0;
    }
    
    // Write to the LED strip.
    ledStrip.write(colors, LED_COUNT);
}
 

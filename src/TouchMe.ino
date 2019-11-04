// TouchMe

#define LOGGING true
#define DEBUGGING true
#define INIT_COUNTS false

// Sensor libary
#include <NewPing.h>

#define SONAR_NUM     4 // Number of sensors.
#define MAX_DISTANCE 185 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 66 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(A0, A0, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(A1, A1, MAX_DISTANCE),
  NewPing(A2, A2, MAX_DISTANCE),
  NewPing(A3, A3, MAX_DISTANCE)
};

// Adafruit MP3 shield
// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

#define VOLUME 1    // Speaker volume, lower numbers == louder volume!

Adafruit_VS1053_FilePlayer musicPlayer =
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// These are the indices to the text files
// organized by sensor and cube form top to bottom

int speech[SONAR_NUM][7] = {
  { 0, 3, 3, 0, 6, 6, 0 },
  { 2, 2, 0, 5, 5, 0, 0 },
  { 1, 1, 0, 4, 7, 7, 7 },
  { 91, 99, 99, 99, 99, 99, 99 }
};

long lastPlayed = 0;
#define SILENCE 10000

#include <EEPROM.h>

#define EEPROM_COUNTS_ADDR 0
int counts[100];

// JeeLib power save

#include <Ports.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }


void setup() {
  char debugMessage[64];
  Serial.begin(9600);
  Serial.println(F("TouchMe Setup Started"));

  if (! musicPlayer.begin()) { // initialise the music player
    Serial.println(F("No VS1053 found"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  SD.begin(CARDCS);    // initialise the SD card

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume( VOLUME, VOLUME );

  // initialize counts
  #if INIT_COUNTS
    for (int i = 0 ; i < (100*sizeof(int)) ; i++) {
      EEPROM.write(i, 0);
    }
    Serial.println(F("Counts cleared"));
  #endif

  // get counts
  #if LOGGING
    EEPROM.get( EEPROM_COUNTS_ADDR, counts );
    sprintf( debugMessage, "Counts %d %d %d %d %d %d %d %d %d", counts[1], counts[2], counts[3], counts[4], counts[5], counts[6], counts[7], counts[91], counts[99] );
    Serial.println( debugMessage );
  #endif

  Serial.println(F("TouchMe Setup OK"));

  setupIntervals();
}


void loop() {
  int brightness;
  char debugMessage[64];
  
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  
  // Other code that *DOESN'T* analyze ping results can go here.
}


// Setup ping intervals, needed for startup and after playing
void setupIntervals() {
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
}


// If ping received, set the sensor distance to array.
void echoCheck() {
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}


// Sensor ping cycle complete, do something with the results.
void oneSensorCycle() {
  int cube;
  int idx;
  int eeAddress;
  char debugMessage[64];
  char fileName[12];
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    cube = 0;
    idx = 0;
    if ( cm[i] > 0 ) {
      cube = cm[i] / 28;
      idx = speech[i][cube];
    }
    #if DEBUGGING
      sprintf( debugMessage, "Sensor %d: %d cm -> cube %d, text %d", i, cm[i], cube, idx );
      Serial.println( debugMessage );
    #endif
    if ( idx > 0 ) {
      if ( idx == 99 ) {
        if ( millis() > ( lastPlayed + SILENCE ) ) {
          lastPlayed = millis();
        }
        else {
          #if DEBUGGING
          Serial.println(F("Skipping..."));
          #endif
          continue;
        }
      }
      sprintf( fileName, "text%d.mp3", idx );
      #if DEBUGGING
        Serial.print(F("Playing "));
        Serial.print( fileName );
      #endif
      musicPlayer.playFullFile(fileName);
      // save counts to eeprom
      #if LOGGING
        counts[idx]++;
        eeAddress = EEPROM_COUNTS_ADDR + ( sizeof(int) * idx );
        EEPROM.put(eeAddress, counts[idx]);
        #if DEBUGGING
          Serial.print(F(", count: "));
          Serial.println( counts[idx] );
        #endif
      #endif
    }
  }
  #if DEBUGGING
    Serial.flush();
  #endif
  Sleepy::loseSomeTime( 100 );  
  setupIntervals();
}


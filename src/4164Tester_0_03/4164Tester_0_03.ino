/* 
 *  Noel Llopis 2021
 *  
 *  Based on initial code by Chris Osborn <fozztexx@fozztexx.com>
 *  http://insentricity.com/a.cl/252
 */

 //  Modified for Uno shield  KJ 3 Aug 2021

// #define DEBUG

/////////////////////////////////////
//  Pushbutton

#include <EnableInterrupt.h>

#define PB_PIN 11
volatile bool button_pushed;

void ButtonDetect() { 

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
    {
    button_pushed = true;
    }
  last_interrupt_time = interrupt_time;
  }

////////////////////////////////////////////////
//  OLED Display
// ss_olde library by Larry Bank
#include <ss_oled.h>

// if your system doesn't have enough RAM for a back buffer, comment out
// this line (e.g. ATtiny85)
//#define USE_BACKBUFFER

#ifdef USE_BACKBUFFER
static uint8_t ucBackBuffer[1024];
#else
static uint8_t *ucBackBuffer = NULL;
#endif

// Use -1 for the Wire library default pins
// or specify the pin numbers to use with the Wire library or bit banging on any GPIO pins
// These are the pin numbers for the M5Stack Atom default I2C
#define SDA_PIN -1
#define SCL_PIN -1
// Set this to -1 to disable or the GPIO pin number connected to the reset
// line of your display if it requires an external reset
#define RESET_PIN -1
// let ss_oled figure out the display address
#define OLED_ADDR -1
// rotate the display
#define FLIP180 0
// don't invert the display
#define INVERT 0
// Bit-Bang the I2C bus
#define USE_HW_I2C 1

// Change these if you're using a different OLED display
#define MY_OLED OLED_128x32
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

SSOLED ssoled;


/////////////////////////////////////////////
//  Pins on 4164

#define DIN             8
#define DOUT            A2
#define CAS             A3
#define RAS             10
#define WE              9

#define ADDR_BITS 8
#define COLROW_COUNT (1 << ADDR_BITS)

bool failed;

void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  #endif
  
  pinMode(PB_PIN, INPUT);  // has external pullup wired
  button_pushed = false;
  enableInterrupt(PB_PIN, ButtonDetect, FALLING);
  
  pinMode(DIN, OUTPUT);
  pinMode(DOUT, INPUT);

  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);





  int rc;  // OLED display

  rc = oledInit(&ssoled, MY_OLED, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L); 
  // use standard I2C bus at 400Khz
  if (rc != OLED_NOT_FOUND)
  {
    #ifdef DEBUG
    char *msgs[] = {(char *)"SSD1306 @ 0x3C", (char *)"SSD1306 @ 0x3D", (char *)"SH1106 @ 0x3C", (char *)"SH1106 @ 0x3D"};
    Serial.println(msgs[rc]);
    #endif
    
    oledFill(&ssoled, 0, 1);
    oledSetBackBuffer(&ssoled, ucBackBuffer);
    oledWriteString(&ssoled, 0, 0, 0, (char *)"4164Tester v0.03", FONT_NORMAL, 0, 1);
    oledWriteString(&ssoled, 0, 0, 1, (char *)"USE WITH CAUTION", FONT_NORMAL, 0, 1);
    
    oledWriteString(&ssoled, 0, 0, 2, (char *)"Press S1", FONT_SMALL, 0, 1);
    oledWriteString(&ssoled, 0, 0, 3, (char *)"  to proceed...", FONT_SMALL, 0, 1);
    while (!button_pushed) ;
    button_pushed = false;

    
  } else {
    #ifdef DEBUG
    Serial.println("Display not found");
    #endif
  }
  
}

void loop()
{ 
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 0, 0, (char *)"Before inserting 4164", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 1, (char *)"ensure power is OFF!", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 2, (char *)"  (check LED2)", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 3, (char *)"Press S1 when ready...", FONT_SMALL, 0, 1);
  while (!button_pushed) ;
  button_pushed = false;
  oledFill(&ssoled, 0, 1);
  oledWriteString(&ssoled, 0, 0, 0, (char *)"Insert 4164 w. pin 1,", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 1, (char *)"bottom right.  Then", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 2, (char *)"power ON (check LED2)", FONT_SMALL, 0, 1);
  oledWriteString(&ssoled, 0, 0, 3, (char *)"Press S1 to start...", FONT_SMALL, 0, 1);
  while (!button_pushed) ;
  button_pushed = false;

  oledFill(&ssoled, 0, 1);

  // chip address lines wired to D0..D7  (PORTD)
  DDRD = 0xff;
  
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(WE, HIGH);
  failed = false;
  
  oledWriteString(&ssoled, 0, 0, 0, (char *)"Testing zeroes", FONT_SMALL, 0, 1);
  checkSame(0);

  oledWriteString(&ssoled, 0, 0, 0, (char *)"Testing ones  ", FONT_SMALL, 0, 1);
  checkSame(1);

  oledWriteString(&ssoled, 0, 0, 0, (char *)"Testing 55s   ", FONT_SMALL, 0, 1);
  checkAlternating(0);
 
  oledWriteString(&ssoled, 0, 0, 0, (char *)"Testing AAs   ", FONT_SMALL, 0, 1);
  checkAlternating(1);

  oledWriteString(&ssoled, 0, 0, 0, (char *)"Testing RANDs ", FONT_SMALL, 0, 1);
  checkRandom();
  
  oledWriteString(&ssoled, 0, 0, 2, (char *)"Finished", FONT_SMALL, 0, 1);
  if (!failed) oledWriteString(&ssoled, 0, 0, 3, (char *)"Passed", FONT_SMALL, 0, 1);
  
  while (!button_pushed) ;
  button_pushed = false;

}


static inline void writeToRowCol(int row, int col)
{
  PORTD = row;
  digitalWrite(RAS, LOW);
  PORTD = col;
  digitalWrite(CAS, LOW);

  digitalWrite(WE, LOW);

  digitalWrite(WE, HIGH);
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
}


static inline int readFromRowCol(int row, int col)
{
  PORTD = row;
  digitalWrite(RAS, LOW);
  PORTD = col;
  digitalWrite(CAS, LOW);

  int val = digitalRead(DOUT);
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  return val;
}

void fail() {
  failed = true;
  oledWriteString(&ssoled, 0, 0, 3, (char *)"Failed", FONT_SMALL, 0, 1);
}

void checkSame(int val)
{
  digitalWrite(DIN, val);
 
  for (int col=0; col<COLROW_COUNT; col++)
    for (int row=0; row<COLROW_COUNT; row++)
      writeToRowCol(row, col);

  /* Reverse DIN in case DOUT is floating */
  digitalWrite(DIN, !val);
 
  for (int col=0; col<COLROW_COUNT; col++)
    for (int row=0; row<COLROW_COUNT; row++)
      if (readFromRowCol(row, col) != val)
        fail();

  return;
}

void checkAlternating(int start)
{
  int i = start;
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      digitalWrite(DIN, i);
      i = !i;
      writeToRowCol(row, col);
    }
  }
  
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    { 
      if (readFromRowCol(row, col) != i)
        fail();
  
      i = !i;
    }
  }
  
  return;
}


void checkRandom()
{
  // Generate a somewhat random seed
  const int seed = analogRead(16);

  randomSeed(seed);
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      const int value = (int)random(2);    
      digitalWrite(DIN, value);
      writeToRowCol(row, col);
    }
  }

  // Set the same seed as for the write to know what value to get
  randomSeed(seed);
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      const int value = (int)random(2);    
      if (readFromRowCol(row, col) != value)
        fail();
    }
  }
  
  return;
}

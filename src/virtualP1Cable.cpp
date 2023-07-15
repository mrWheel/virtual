/*
* P1virtualWire
* copyright 2023 by Willem Aandewiel
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
* --> by Dejan Nedelkovski, www.HowToMechatronics.com
*/
/*------------------------------------------------------
*  Chip: "AVR128DB32"
*  Clock Speed: "24 MHz internal"
*  millis()/micros() timer: |TCB2 (recommended)"
*  Reset pin function: "Hardware Reset (recommended)"
*  Startup Time: "8ms"
*  MultiVoltage I/O (MVIO): "Disabled"
*  --
*  Programmer: "jtag2updi"
*-----------------------------------------------------*/

/*
 * PINS NAME     AVR128DB32        AVR128DB28
 * TXD0           PIN_PA0           PIN_PA0
 * RXD0           PIN_PA1           PIN_PA1
 * MOSI           PIN_PA4           PIN_PA4
 * MISO           PIN_PA5           PIN_PA5
 * SCK            PIN_PA6           PIN_PA6
 * PIN_LED        PIN_PA7           PIN_PA7
 * TXD1           PIN_PC0           PIN_PC0
 * RXD1           PIN_PC1           PIN_PC1
 * BAUD_SET       -                 PIN_PC3
 * PIN_MODE       PIN_PF0           PIN_PF0
 * PIN_CE         PIN_PF1           PIN_PD1
 * PIN_CSN        PIN_PF2           PIN_PD2
 * SWITCH1        -                 PIN_PD3
 * SWITCH2        -                 PIN_PD4
 * SWITCH3        -                 PIN_PD5
 * SWITCH4        -                 PIN_PD6
*/
#define _AVR128DB28
#ifdef _AVR128DB28
  #define PIN_CE      PIN_PD1
  #define PIN_CSN     PIN_PD2
  #define PIN_LED     PIN_PA7
  #define BAUD_SET    PIN_PC3
  #define PIN_MODE    PIN_PF0
  #define SWITCH1     PIN_PD3
  #define SWITCH2     PIN_PD4
  #define SWITCH3     PIN_PD5
  #define SWITCH4     PIN_PD6
#else
  #define PIN_CE      PIN_PF1
  #define PIN_CSN     PIN_PF2
  #define PIN_LED     PIN_PA7
  #define BAUD_SET    -1
  #define PIN_MODE    PIN_PF0
  #define SWITCH1     -1
  #define SWITCH2     -1
  #define SWITCH3     -1
  #define SWITCH4     -1
#endif

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//#include "CRC16.h"

// "/KFM5KAIFA-METER\r\n\r\n1-0:1.8.1(000671.578*kWh)\r\n1-0:1.7.0(00.318*kW)\r\n!1E1D\r\n\r\n";
char minTelegram[] =
  "/KFM5KAIFA-METER\r\n"              //-- 18
  "\r\n"                              //--  2
  "1-0:1.8.1(000671.578*kWh)\r\n"     //-- 27
  "1-0:1.7.0(00.318*kW)\r\n"          //-- 22
  //----------------------------------//---> 69
  "!"                                 //---> 70
  "1E1D\r\n\r\n";

#define _MAX_P1BUFFER       10000
#define _PAYLOAD_SIZE          20
#define _MAX_TRANSMIT_ERRORS   50
#define _MAX_LAST_TIME      30000 //-- 10 seconden
#define _MAX_TIMEOUT         2000 //--  2 seconden
#define _REBOOT_TIME      3000000 //-- iedere vijf minuten

RF24 radio(PIN_CE, PIN_CSN); // CE, CSN

const byte address[6] = "00001";

bool      isReceiver;
char      p1Buffer[_MAX_P1BUFFER+15];  // room for "!1234\0\r\n\r\n"
int16_t   p1BuffLen;
int16_t   p1BuffPos = 0;
bool      startTelegram;
int16_t   startTelegramPos;
int16_t   avrgTelegramlen = 50;
char      payloadBuff[_PAYLOAD_SIZE+10];
uint32_t  startTime, waitTimer, rebootTimer;
int32_t   duration=0;
uint32_t  errCount;
char      orgCRC[10];
int16_t   orgCRClen;

//--------------------------------------------------------------
void resetViaSWR() 
{
  Serial.println("Software reset ...");
  Serial.flush();
  delay(500);
  _PROTECTED_WRITE(RSTCTRL.SWRR,1);

} //  resetViaSWR()

//--------------------------------------------------------------
int findChar(const char *haystack, char needle) 
{
  int16_t pos = 0;
  while(haystack[pos] != 0)
  {
    if (haystack[pos] == needle) return pos;
    pos++;
  }
  return -1;

} //  findChar()

//--------------------------------------------------------------
bool readSwitches() 
{
  //---- read switch1 .. switch4 and baud_set
  //---- en doe daar wat mee
  isReceiver = digitalRead(PIN_MODE);
  /*
  x[0] = digitalRead(BAUD_SET);
  x[1] = digitalRead(SWITCH1);
  x[2] = digitalRead(SWITCH2);
  x[3] = digitalRead(SWITCH3);
  x[4] = digitalRead(SWITCH4);
  */

} //  readSwitches()
//--------------------------------------------------------------
void transmitTelegram(char *telegram, int telegramLen) 
{
  bool ok = false;

  errCount         = 0;
  startTelegramPos = 0;

  Serial.println("\r\nTransmit telegram ..\r\n");
  digitalWrite(PIN_LED, HIGH);
  startTime = millis();

  while(startTelegramPos<telegramLen)
  {
    memset(payloadBuff, 0, _PAYLOAD_SIZE+10);
    for(int i=0; i<_PAYLOAD_SIZE; i++)
    {
      payloadBuff[i] = telegram[i+startTelegramPos];
    }
    //Serial.println(payloadBuff);
    //---- ending "0" activates autoACK & Retry
    errCount = 0;
    do
    {
      ok = radio.write(&payloadBuff, _PAYLOAD_SIZE, 0);
      if (!ok) { delay(10); errCount++; }
      else 
      { 
        if (startTelegramPos==0) 
        { delay(50); }  //-- give receiver time to check start
        else          
        { delay(5); }
      }
    }
    while (!ok && (errCount < _MAX_TRANSMIT_ERRORS));
    if (!ok) 
    {
      Serial.println("Max Transmit errors.. Bail out!");
      for(int i=0; i<15; i++)
      {
        digitalWrite(PIN_LED, CHANGE);
        delay(70);
      }
      digitalWrite(PIN_LED, LOW);
      return;
    }
    startTelegramPos += _PAYLOAD_SIZE;
    delay(10);
  }

  memset(payloadBuff, 0, _PAYLOAD_SIZE);
  //---- ending "0" activates autoACK & Retry
  errCount = 0;
  do
  {
    ok = radio.write(&payloadBuff, _PAYLOAD_SIZE, 0);
    if (!ok) { delay(10); errCount++; }
    else     { delay(5); }
  }
  while (!ok && (errCount < _MAX_TRANSMIT_ERRORS));
  Serial.println("Telegram transmitted!");

} //  transmitTelegram()


//--------------------------------------------------------------
void setupReceiver()
{
  Serial.println("I'm a RECEIVER...\r\n");

  radio.openReadingPipe(0, address);
  radio.startListening();

  rebootTimer = millis();

} //  setupReceiver


//--------------------------------------------------------------
void loopReceiver()
{
  bool foundSlash   = false;
  bool foundExcl    = false;
  bool readOneMore  = false;

  if (radio.available()) 
  {
    memset(payloadBuff, 0, _PAYLOAD_SIZE+10);

    uint8_t noBytes = radio.getDynamicPayloadSize();
    radio.read(&payloadBuff, noBytes);

    foundSlash = false;
    int posSlash = findChar(payloadBuff, '/');
    if (posSlash >= 0)
    {
      //Serial.printf("\r\nFound [%c] in [%s] at pos[%d]\r\n", '/', payloadBuff, posSlash);
      foundSlash  = true;
      digitalWrite(PIN_LED, HIGH); 
      memset(p1Buffer, 0, _MAX_P1BUFFER+10);
      p1BuffPos = 0;
      memcpy(&p1Buffer[p1BuffPos], payloadBuff, noBytes);
      p1BuffPos += noBytes;
      startTelegramPos = 0;
      startTime = millis();
      duration = -1;
    }
    
    if (!foundSlash) return;

    readOneMore = false;
    do
    {
      memset(payloadBuff, 0, _PAYLOAD_SIZE+10);
      if (radio.available())
      {
       if (readOneMore) readOneMore = false;

        uint8_t noBytes = radio.getDynamicPayloadSize();
        radio.read(&payloadBuff, noBytes);
        memcpy(&p1Buffer[p1BuffPos], payloadBuff, noBytes);
        p1BuffPos += noBytes;
        foundExcl = false;
        int posExcl = findChar(payloadBuff, '!');
        if (posExcl >= 0)
        { 
          foundExcl = true;
          //Serial.printf("\r\nFound [%c] in [%s] at pos[%d]\r\n", '!', payloadBuff, posExcl);
          digitalWrite(PIN_LED, LOW); 
          duration = millis() - startTime;
          if (posExcl > (noBytes -8)) readOneMore = true;
        }
      }
    }
    while (!foundExcl && !readOneMore);    
  
    startTelegram = false;
    startTelegramPos = -1;
    p1BuffLen = strlen(p1Buffer);
    Serial.printf("Find last [/] in [%d]bytes ...\r\n", p1BuffLen);
    for(int i=p1BuffLen; (i>=0 && !startTelegram); i--)
    {
      if (p1Buffer[i] == '/')
      {
        startTelegram = true;
        startTelegramPos = i;
        Serial.printf("\r\nnStart telegram [/] found at byte[%d] ...\r\n\n", startTelegramPos);
        break;
      }
    }
    if (!startTelegram)
    {
      Serial.println("Not a valid start [/] found.. Bailout\r\n");
      return;
    }

    Serial.println();
    Serial.printf("+++++telegram is [%d]bytes++++++++++++++++++++\r\n", strlen(p1Buffer)-startTelegramPos);
    Serial.print(&p1Buffer[startTelegramPos]);
    //Serial.println("###############################################");
    Serial1.print(&p1Buffer[startTelegramPos]);
  }

  if ((millis() - rebootTimer) > _REBOOT_TIME) { resetViaSWR(); }

} //  loopReceiver


//--------------------------------------------------------------
void setupTransmitter()
{
  memset(p1Buffer, 0, sizeof(p1Buffer));

  Serial.println("I'm a TRANSMITTER...\r\n");
  Serial1.setTimeout(10);

  radio.openWritingPipe(address);
  radio.stopListening();

} //  setupTransmitter


//--------------------------------------------------------------
void loopTransmitter()
{
  bool ok = false;
  bool edited = false;

  if (!Serial1.available()) { return; }

  memset(orgCRC,   0, sizeof(orgCRC));

  memset(p1Buffer, 0, _MAX_P1BUFFER+10);
  //-- set timeout to 15 seconds ....
  Serial1.setTimeout(_MAX_TIMEOUT);
  waitTimer = millis();
  //-- read until "!" ("!" is not in p1Buffer!!!) --
  int len = Serial1.readBytesUntil('!', p1Buffer, (_MAX_P1BUFFER -10));
  Serial.printf("\r\nread [%d]bytes\r\n", len);
  //Serial.println("\r\n===\r\n");
  //-- if timer has expired ..
  if ((millis() - waitTimer) >= _MAX_TIMEOUT)  
  {
    Serial.println("Max waittime expired! BialOut");
    Serial.flush();
    return;
  }
  //          1       2       3                                        len
  //  1...5...0...5...0...5...0                       ..................| 
  //                                                                    v
  // "/KFM5KAIFA-METER\r\n\r\n1-0:1.8.1(0006 . . . 0:1.7.0(00.318*kW)\r\n!1E1D\r\n\r\n";
  //
  if (len < avrgTelegramlen) 
  {
    Serial.printf("Telegram too short [%d]bytes; must be at least [%d]bytes... Bailout\r\n", len, avrgTelegramlen);
    if (avrgTelegramlen > 60) avrgTelegramlen -= 10;
    return;
  }
  //-- track back to first non-control char
  for (int p=0; p>-10; p--)
  {
    //-- test if not a control char
    if ((p1Buffer[len-p] >= ' ') && (p1Buffer[len-p] <= '~'))
    {
      //Serial.printf("\r\n[%d] found [%c]\r\n\n", (len-p), p1Buffer[len-p]);
      len-=p;
      break;
    }
  }
  //Serial.printf("After track Back len in [%d] bytes (last char[%c]\r\n", len, p1Buffer[len]);
  //                                                               len
  //  1...5...1...5...2...5...3                       ..............| 
  //                                                                v
  // "/KFM5KAIFA-METER\r\n\r\n1-0:1.8.1(0006 . . . 0:1.7.0(00.318*kW)\r\n!1E1D\r\n\r\n";
  //
  int posExcl = len;
  p1Buffer[len++] = '!';
  p1Buffer[len++] = '0';
  p1Buffer[len++] = '0';
  p1Buffer[len++] = '0';
  p1Buffer[len++] = '0';
  p1Buffer[len++] = '\r';
  p1Buffer[len++] = '\n';
  p1Buffer[len++] = '\r';
  p1Buffer[len++] = '\n';
  p1Buffer[len++] = '\0';
  p1Buffer[len]   = '\0';

  int orgCRCpos = 0;
  memset(orgCRC, 0, sizeof(orgCRC));

  Serial1.setTimeout(500);
  int orgCRCLen = Serial1.readBytesUntil('\r', orgCRC, sizeof(orgCRC));
  for(int i=0; i<strlen(orgCRC); i++)
  {
    //Serial.printf("Test orgCRC[%d] (%c)\r\n", i, orgCRC[i]);
    if ((orgCRC[i] >= ' ') && (orgCRC[i] <= '~'))
    {
      posExcl++;
      //Serial.printf("p1Buffer[%d] = [%c] ==> ", posExcl, p1Buffer[posExcl]);
      p1Buffer[posExcl] = orgCRC[i];
      //Serial.printf("Changed to [%c] \r\n", p1Buffer[posExcl]);
    }
  }
  
  startTelegram = false;
  startTelegramPos = -1;
  p1BuffLen = strlen(p1Buffer);
  avrgTelegramlen = p1BuffLen * 0.8;
  Serial.printf("Find last [/] .. in [%d]bytes\r\n", p1BuffLen);
  for(int i=p1BuffLen; (i>=0 && !startTelegram); i--)
  {
    //Serial.printf("p1Buffer[%3d] > [%c]\r\n", i, p1Buffer[i]);
    if (p1Buffer[i] == '/')
    {
      startTelegram = true;
      startTelegramPos = i;
      Serial.printf("\r\nnStart telegram [/] found at byte[%d] ...\r\n\n", startTelegramPos);
      break;
    }
  }
  if (!startTelegram)
  {
    Serial.println("Not a valid start [/] found.. Bailout\r\n");
    return;
  }

  //Serial.println("-------111111---------------------------------------------");
  //Serial.print(p1Buffer);
  Serial.println("-------222222---------------------------------------------");
  Serial.print(&p1Buffer[startTelegramPos]);
  Serial.println("----------------------------------------------------------");
  p1BuffLen = strlen(p1Buffer) - startTelegramPos;
  Serial.print("p1Telegram is ["); Serial.print(p1BuffLen); Serial.println("]bytes long");
  Serial.flush();

  transmitTelegram(&p1Buffer[startTelegramPos], p1BuffLen);

  digitalWrite(PIN_LED, LOW);

} //  loopTransmitter


//==============================================================
void setup() 
{
  pinMode(PIN_LED,  OUTPUT);
  pinMode(BAUD_SET, INPUT_PULLUP);
  pinMode(SWITCH1,  INPUT_PULLUP);
  pinMode(SWITCH2,  INPUT_PULLUP);
  pinMode(SWITCH3,  INPUT_PULLUP);
  pinMode(SWITCH4,  INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("\n\n\nAnd than it begins ....");
  Serial1.begin(115200);

  readSwitches();
  isReceiver = digitalRead(PIN_MODE);
  
  for (int i=0; i<10; i++)  
  {
    digitalWrite(PIN_LED, CHANGE);
    Serial.print(">>>");
    delay(200);
  }
  Serial.println();
  //--- now setup radio's ----
  radio.begin();
  //-- Max power 
  radio.setPALevel( RF24_PA_MAX ) ; 
  // Min speed (for better range I presume)
  radio.setDataRate( RF24_250KBPS ) ; 
  //-- 8 bits CRC
  radio.setCRCLength( RF24_CRC_8 ) ; 
  //-- Disable dynamic payloads 
  //-- radio.write_register(DYNPD,0); 
  radio.enableDynamicPayloads();
  //-- save on transmission time by setting the radio to only transmit the
  //-- number of bytes we need to transmit
  radio.setPayloadSize(_PAYLOAD_SIZE);  
  //-- increase the delay between retries & # of retries 
  radio.setRetries(10,10);  //-- 15 - 15 is the max!
  radio.setChannel(0x61);

  if (isReceiver)
  {
    Serial1.println("I'm a Receiver!");
    setupReceiver();
  }
  else  
  {
    Serial1.println("I'm a Transmitter!");
    setupTransmitter();
  }
  
} //  setup()


//==============================================================
void loop() 
{
  if (isReceiver) loopReceiver();
  else            loopTransmitter();
}

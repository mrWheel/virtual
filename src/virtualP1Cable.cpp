/*
* virual P1 Cable
* copyright 2023 by Willem Aandewiel
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
* --> by Dejan Nedelkovski, www.HowToMechatronics.com
*/
/*------------------------------------------------------
*  Chip: "AVR128DB32" / "AVR128DB28"
*  Clock Speed: "24 MHz internal"
*  millis()/micros() timer: |TCB2 (recommended)"
*  Reset pin function: "Hardware Reset (recommended)"
*  Startup Time: "8ms"
*  MultiVoltage I/O (MVIO): "Disabled"
*  --
*  Programmer: "jtag2updi"
*-----------------------------------------------------*/

#define _FW_VERSION "1.0 (23-07-2023)"

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
#if defined( __AVR_AVR128DB32__ )
  #define PIN_CE      PIN_PF1
  #define PIN_CSN     PIN_PF2
#elif defined( __AVR_AVR128DB28__ )
  #define PIN_CE      PIN_PD1
  #define PIN_CSN     PIN_PD2
#else
  #error No Processor selected!
#endif
#define PIN_LED     PIN_PA7
#define BAUD_SET    PIN_PC3
#define PIN_MODE    PIN_PF0
#define SWITCH1     PIN_PD3
#define SWITCH2     PIN_PD4
#define SWITCH3     PIN_PD5
#define SWITCH4     PIN_PD6

#define SET_BIT(x, pos) (x |= (1U << pos))
#define CLR_BIT(x, pos) (x &= (~(1U<< pos)))

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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

byte      pipeName[6] = {};

bool      isReceiver;
byte      channelNr = 0x61;
int       powerMode = RF24_PA_MIN;
char      p1Buffer[_MAX_P1BUFFER+15];  // room for "!1234\0\r\n\r\n"
int16_t   p1BuffLen;
int16_t   p1BuffPos = 0;
bool      startTelegram;
int16_t   startTelegramPos;
int16_t   avrgTelegramlen = 500;
char      payloadBuff[_PAYLOAD_SIZE+10];
uint32_t  startTime, waitTimer, rebootTimer;
uint32_t  errCount;
char      orgCRC[10];
int16_t   orgCRClen;

//--------------------------------------------------------------
void resetViaSWR() 
{
  Serial.println("\r\nSoftware reset ...\r\n");
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
  Serial.println("Read the Switches ...");

  if (digitalRead(SWITCH1))
  {
    powerMode = RF24_PA_MAX;
    Serial.println("RF24 Power set to MAX");
  }
  else  
  {
    powerMode = RF24_PA_LOW;
    Serial.println("RF24 Power set to LOW");
  }
  //x[0] = digitalRead(BAUD_SET);
  /*
  **    |      SWITCH     | CHANNEL
  **    |  1  |   2 |   3 |
  **    +-----------------+---------
  **    | off | off | off | x54 - x65
  **    | On  | off | off | x14 - x25
  **    | off | On  | off | x44 - x55
  **    | On  | On  | off | x04 - x15
  **    | off | off | On  | x50 - x61
  **    | On  | off | On  | x10 - x21
  **    | off | On  | On  | x40 - x51
  **    | On  | On  | On  | x00 - x11
  **
  */
  CLR_BIT(channelNr,0);
  CLR_BIT(channelNr,1);
  if (digitalRead(SWITCH2))
        SET_BIT(channelNr,2);
  else  CLR_BIT(channelNr,2);
  CLR_BIT(channelNr,3);
  if (digitalRead(SWITCH3))
        SET_BIT(channelNr,4);
  else  CLR_BIT(channelNr,4);
  CLR_BIT(channelNr,5);
  if (digitalRead(SWITCH4))
        SET_BIT(channelNr,6);
  else  CLR_BIT(channelNr,6);
  CLR_BIT(channelNr,7);
  channelNr += 0x11;

} //  readSwitches()


//--------------------------------------------------------------
void transmitTelegram(char *telegram, int telegramLen) 
{
  bool transmitOk = false;

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
    Serial.print(payloadBuff);
    //---- ending "0" activates autoACK & Retry
    errCount = 0;
    do
    {
      transmitOk = radio.write(&payloadBuff, _PAYLOAD_SIZE, 0);
      if (!transmitOk) { delay(10); errCount++; }
      else 
      { 
        if (startTelegramPos==0) 
        { delay(50); }  //-- give receiver time to check start
        else          
        { delay(5); }
      }
    }
    while (!transmitOk && (errCount < _MAX_TRANSMIT_ERRORS));
    if (!transmitOk) 
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

  //-- send End Of Telegram token
  memset(payloadBuff, 0, _PAYLOAD_SIZE);
  memcpy(&payloadBuff[0], "*EOT*\r\n", 7);

  //---- ending "0" activates autoACK & Retry
  errCount = 0;
  do
  {
    transmitOk = radio.write(&payloadBuff, _PAYLOAD_SIZE, 0);
    Serial.print(payloadBuff);
    if (!transmitOk)  { delay(10); errCount++; }
    else              { delay(5); }
  }
  while (!transmitOk && (errCount < _MAX_TRANSMIT_ERRORS));
  Serial.printf("Telegram transmitted in %d milliseconds!\r\n", (millis()-startTime));

} //  transmitTelegram()


//--------------------------------------------------------------
void setupReceiver()
{
  Serial.printf("I'm a RECEIVER for pipe [%s]...\r\n\n", pipeName);

  radio.openReadingPipe(0, pipeName);
  radio.startListening();

  rebootTimer = millis() + _REBOOT_TIME;

} //  setupReceiver


//--------------------------------------------------------------
void loopReceiver()
{
  bool foundSlash   = false;
  bool foundEOT     = false;

  if (radio.available()) 
  {
    memset(payloadBuff, 0, _PAYLOAD_SIZE+10);

    uint8_t bytesRead = radio.getDynamicPayloadSize();
    radio.read(&payloadBuff, bytesRead);

    foundSlash = false;
    int posSlash = findChar(payloadBuff, '/');
    if (posSlash >= 0)
    {
      foundSlash  = true;
      digitalWrite(PIN_LED, HIGH); 
      memset(p1Buffer, 0, _MAX_P1BUFFER+10);
      p1BuffPos = 0;
      memcpy(&p1Buffer[p1BuffPos], payloadBuff, bytesRead);
      p1BuffPos += bytesRead;
      startTelegramPos = 0;
      startTime = millis();
    }
    
    if (!foundSlash) return;

    foundEOT = false;
    while(!foundEOT && (p1BuffPos < (_MAX_P1BUFFER -15)) && (millis() < rebootTimer) )
    {
      memset(payloadBuff, 0, _PAYLOAD_SIZE+10);
      if (radio.available())
      {
        uint8_t bytesRead = radio.getDynamicPayloadSize();
        radio.read(&payloadBuff, bytesRead);
        //-- did we receive a EOT-token?
        if (    payloadBuff[0] == '*'
              && payloadBuff[1] == 'E'
               && payloadBuff[2] == 'O'
                && payloadBuff[3] == 'T'
                 && payloadBuff[4] == '*'
            ) 
        {
          Serial.println("\r\nfound EOT token!");
          foundEOT = true;
          digitalWrite(PIN_LED, LOW); 
        }
        else
        {
          memcpy(&p1Buffer[p1BuffPos], payloadBuff, bytesRead);
          p1BuffPos += bytesRead;
        }
      }
    }

    Serial.println();
    Serial.printf("+++++telegram is [%d]bytes++++++++++++++++++++\r\n", strlen(p1Buffer)-startTelegramPos);
    Serial.print(&p1Buffer[startTelegramPos]);
    Serial1.print(&p1Buffer[startTelegramPos]);
    //-- reset rebootTimer --
    rebootTimer = millis() + _REBOOT_TIME;
    Serial.printf("Telegram received in %d milliseconds!\r\n", (millis() - startTime));
  }

  if ( millis() >  rebootTimer)  { resetViaSWR(); }


} //  loopReceiver


//--------------------------------------------------------------
void setupTransmitter()
{
  memset(p1Buffer, 0, sizeof(p1Buffer));

  Serial.printf("I'm a TRANSMITTER for pipe [%s]...\r\n\n", pipeName);
  Serial1.setTimeout(10);

  radio.openWritingPipe(pipeName);
  radio.stopListening();

} //  setupTransmitter


//--------------------------------------------------------------
void loopTransmitter()
{
  bool transmitOk = false;

  if (!Serial1.available()) { return; }

  memset(orgCRC,   0, sizeof(orgCRC));

  memset(p1Buffer, 0, _MAX_P1BUFFER+10);
  //-- set timeout to 15 seconds ....
  Serial1.setTimeout(_MAX_TIMEOUT);
  waitTimer = millis();
  //-- read until "!" ("!" is not in p1Buffer!!!) --
  int len = Serial1.readBytesUntil('!', p1Buffer, (_MAX_P1BUFFER -15));
  Serial.printf("\r\nread [%d]bytes\r\n", len);

  if (len > (_MAX_P1BUFFER - 20))
  {
    Serial.println("Something went wrong .. (no '!' found in input)");
    Serial.println(p1Buffer);
    Serial.println("\r\n==========");
    Serial.println(".. Bailout! \r\n");
    Serial.flush();
    return;
  }

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
  //-- track back to first non-control char
  for (int p=0; p>-10; p--)
  {
    //-- test if not a control char
    if ((p1Buffer[len-p] >= ' ') && (p1Buffer[len-p] <= '~'))
    {
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
  p1Buffer[len++] = '!';

  int orgCRCpos = 0;
  memset(orgCRC, 0, sizeof(orgCRC));

  Serial1.setTimeout(500);
  int orgCRCLen = Serial1.readBytesUntil('\r', orgCRC, sizeof(orgCRC));
  for(int i=0; i<strlen(orgCRC); i++)
  {
    if ((orgCRC[i] >= ' ') && (orgCRC[i] <= '~'))
    {
      p1Buffer[len++] = orgCRC[i];
    }
  }
  p1Buffer[len++] = '\r';
  p1Buffer[len++] = '\n';
  p1Buffer[len++] = '\r';
  p1Buffer[len++] = '\n';
  p1Buffer[len++] = '\0';

  //-- telegram length does not vary much so it is
  //-- a good idear to test it
  if (len < avrgTelegramlen) 
  {
    Serial.printf("Telegram too short [%d]bytes; must be at least [%d]bytes... Bailout\r\n", len, avrgTelegramlen);
    if (avrgTelegramlen > 60) avrgTelegramlen -= 10;
    return;
  }

  startTelegram = false;
  startTelegramPos = -1;
  p1BuffLen = strlen(p1Buffer);
  avrgTelegramlen = p1BuffLen * 0.8;
  Serial.printf("Find last [/] .. in [%d]bytes\r\n", p1BuffLen);
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

  Serial.println("------------------------------------------------------");
  Serial.print(&p1Buffer[startTelegramPos]);

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
  Serial.printf("Firmware version v%s\r\n", _FW_VERSION);
  //-- P1 data 
  Serial1.begin(115200);

  readSwitches();

  snprintf(pipeName, 6, "P1-%02x", channelNr);

  isReceiver = digitalRead(PIN_MODE);
  
  for (int i=0; i<10; i++)  
  {
    digitalWrite(PIN_LED, CHANGE);
    delay(200);
  }
  Serial.println();
  //--- now setup radio's ----
  if (!radio.begin())
  {
    Serial.println("\r\nNo NRF24L01 tranceiver found!\r");
    while(true)
    {
      digitalWrite(PIN_LED, CHANGE);
      delay(100);
    }
  }
  //-- Set Transmitter Power 
  radio.setPALevel( powerMode );
  //-- Min speed (for better range I presume)
  radio.setDataRate( RF24_250KBPS ) ; 
  //-- 8 bits CRC
  radio.setCRCLength( RF24_CRC_8 ) ; 
  //-- Enable dynamic payloads 
  radio.enableDynamicPayloads();
  //-- save on transmission time by setting the radio to only transmit the
  //-- number of bytes we need to transmit
  radio.setPayloadSize(_PAYLOAD_SIZE);  
  //-- increase the delay between retries and # of retries 
  radio.setRetries(10,10);  //-- 15 - 15 is the max!

  //-- set Channel
  Serial.printf("Set RF24 channel to [0x%x]/[%d]\r\n", channelNr, channelNr);
  radio.setChannel(channelNr);

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

## Display layout:
### Pages
The RAM divides the display into 4 pages, PAGE0-PAGE3. Each page is a 8x128 matrix. It has 128 columns and 8 rows (if you hold the ChipKIT such that the buttons lie horisontally). 1 pixel is 1 bit.

So if you had an array of bytes to store your info, byte `i` is column `i % 128` of page `k`, where k solves `i = k*128 + r` for some positive integer `r` (I T H I N K).

There are different [addressing modes](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf) for accessing individual pixels on the screen. In general, there is a page pointer and a column pointer (recall that one column is the column of a page = 1 byte = 8 bits):
- Page addressing mode. The column pointer scans a page from left to right, 1 column (aka byte of information) at a time. When reaching the end of a page, the column pointer wraps around to the beginning of the same page. To move to the next page, you have to manually increase the page pointer (by sending issuing a command to the display -- how to do so is described below, in the next section).
- Horisontal addressing mode. Similar to page addressing mode, but it automatically increments the page counter upon reaching the end of a page.
- Vertical addressing mode. It reads column `i` of each page before moving on to the next column. So it would expect col0(page0), followed by col0(page1), followed by col0(page2), followed by col0(page3), followed by col1(page0) etcetc..

The "biggest thing" that the addressing mode changes (afaik) is the way you send your data. It might be noice to choose the addressing mode that fits how you want to store your screen data in the game/application (array of arrays, byte array, whatever). I think lab 3 used page addressing mode.

### Writing bytes to the GDDRAM (the display RAM I presume)
You change pixels to the display RAM by switching to data mode on the display and putting a byte (representing some pixels) up for sending
(it is sent using the SPI -- serial peripheral interface; there is a spi_send_recv function from lab3 i believe, which uses stuff described later).

The byte is then translated into 1 column of the current page, D0 being the lsb, D7 being the msb. How do we know which column gets converted? We know by the column address pointer, which column we are currently at. After you write 1 byte, the pointer increments by 1 to go to the next pointer.

To perform commands/configure the display, switch to command mode on the display and put a byte (representing a command) up for sending. This command then configures the display (see [the same link](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf) for the full command documentation, like how to choose the starting column/how to increment page pointers/change addressing mode).

# Some Terminology & other stuff -- most of this is not too important
## SPI2 (This probably isn't too important)
SPI: Synchronous serial port. Pin 10 (SS), pin 11 (MOSI), pin 12 (MISO), pin 13 (SCK). This uses SPI2 (SS2, SDI2,
SDO2, SCK2) on the PIC32 microcontroller. These signals also appear on connector J8.

## SPI2BUF
[page 111](http://ww1.microchip.com/downloads/en/DeviceDoc/61143H.pdf). Stores data in a buffer that will be sent once both master and slave are ready.

## SPI2STAT
[table 4-12](http://ww1.microchip.com/downloads/en/DeviceDoc/61143H.pdf). Contains information that is used to update display.

## Example code from manual
Some code from the basic IO shield OLED display:

```C
void
OledHostInit()
{
    unsigned int tcfg;
    /* Initialize SPI port 2.
    */
    SPI2CON = 0;
    SPI2BRG = 15; //8Mhz, with 80Mhz PB clock
    SPI2STATbits.SPIROV = 0;
    SPI2CONbits.CKP = 1;
    SPI2CONbits.MSTEN = 1;
    SPI2CONbits.ON = 1;
    /* Make pins RF4, RF5, and RF6 be outputs.
    */
    PORTSetBits(IOPORT_F, bitVddCtrl|bitVbatCtrl|bitDataCmd);
    PORTSetPinsDigitalOut(prtDataCmd, bitDataCmd); //Data/Command# select
    PORTSetPinsDigitalOut(prtVddCtrl, bitVddCtrl); //VDD power control
    (1=off)
    PORTSetPinsDigitalOut(prtVbatCtrl, bitVbatCtrl); //VBAT power control
    (1=off)
    /* Make the RG9 pin be an output. On the Basic I/O Shield, this pin
    ** is tied to reset.
    */
    PORTSetBits(prtReset, bitReset);
    PORTSetPinsDigitalOut(prtReset, bitReset);
}

void
OledDspInit()
{
    /* We're going to be sending commands, so clear the Data/Cmd bit
    */
    PORTClearBits(prtDataCmd, bitDataCmd);
    /* Start by turning VDD on and wait a while for the power to come up.
    */
    PORTClearBits(prtVddCtrl, bitVddCtrl);
    DelayMs(1);
    /* Display off command
    */
    Spi2PutByte(0xAE);
    /* Bring Reset low and then high
    */
    PORTClearBits(prtReset, bitReset);
    DelayMs(1);
    PORTSetBits(prtReset, bitReset);
    /* Send the Set Charge Pump and Set Pre-Charge Period commands
    */
    Spi2PutByte(0x8D);
    Spi2PutByte(0x14);
    Spi2PutByte(0xD9);
    Spi2PutByte(0xF1);
    /* Turn on VCC and wait 100ms
    */
    PORTClearBits(prtVbatCtrl, bitVbatCtrl);
    DelayMs(100);
    /* Send the commands to invert the display. This puts the display origin
    ** in the upper left corner.
    */
    Spi2PutByte(0xA1); //remap columns
    Spi2PutByte(0xC8); //remap the rows
    /* Send the commands to select sequential COM configuration. This makes the
    ** display memory non-interleaved.
    */
    Spi2PutByte(0xDA); //set COM configuration command
    Spi2PutByte(0x20); //sequential COM, left/right remap enabled
    /* Send Display On command
    */
    Spi2PutByte(0xAF);
}

void
OledUpdate()
{
    int ipag;
    int icol;
    BYTE * pb;
    pb = rgbOledBmp;
    for (ipag = 0; ipag < cpagOledMax; ipag++) {
    PORTClearBits(prtDataCmd, bitDataCmd);
    /* Set the page address
    */
    Spi2PutByte(0x22); //Set page command
    Spi2PutByte(ipag); //page number
    /* Start at the left column
    */
    Spi2PutByte(0x00); //set low nybble of column
    Spi2PutByte(0x10); //set high nybble of column
    PORTSetBits(prtDataCmd, bitDataCmd);
    /* Copy this memory page of display data.
    */
    OledPutBuffer(ccolOledMax, pb);
    pb += ccolOledMax;
    }
}

void
OledPutBuffer(int cb, BYTE * rgbTx)
{
    int ib;
    BYTE bTmp;
    /* Write/Read the data
    */
    for (ib = 0; ib < cb; ib++) {
    /* Wait for transmitter to be ready
    */
    while (SPI2STATbits.SPITBE == 0);
    /* Write the next transmit byte.
    */
    SPI2BUF = *rgbTx++;
    /* Wait for receive byte.
    */
    while (SPI2STATbits.SPIRBF == 0);
    bTmp = SPI2BUF;
    }
}

BYTE
Spi2PutByte(BYTE bVal)
{
    BYTE bRx;
    /* Wait for transmitter to be ready
    */
    while (SPI2STATbits.SPITBE == 0);
    /* Write the next transmit byte.
    */
    SPI2BUF = bVal;
    /* Wait for receive byte.
    */
    while (SPI2STATbits.SPIRBF == 0);
    /* Put the received byte in the buffer.
    */
    bRx = SPI2BUF;
    return bRx;
}
```

Some bits in SPI2STAT:

* SPIRBF: SPI2 "has receiver received the byte (1 if it has received)"
* SPITBE: SPI2 "is transmitter ready (1 if ready) to transmit"

## For initializing:
### Port F
Remember to set PORTF output signals for the display when you initiate!

### SPI2CON
Configurations.

### SPI2BRG
Baud-Rate Generator.

### Terminology
* SCLK: Serial Clock (output from master).
* MOSI: Master Output Slave Input, or Master Out Slave In (data output from master).
* MISO: Master Input Slave Output, or Master In Slave Out (data output from slave).
* SDIO: Serial Data I/O (bidirectional I/O)
* SS: Slave Select (often active low, output from master).

Pin 10 (SS), pin 11 (MOSI), pin 12 (MISO), pin 13 (SCK). This uses SPI2 (SS2, SDI2,
SDO2, SCK2) on the PIC32 microcontroller.

Jumpers JP5 and JP7 are used to select whether the Uno32 operates as a master (transmit on MOSI, receive on
MISO) or a slave (transmit on MISO, receive on MOSI) device. The shorting blocks on JP5 and JP7 are normally
placed in the master position for the Uno32 to function as an SPI master.

Jumper JP4 is used to select PWM output or the SPI SS function on pin 10. The shorting block on JP4 should be in
the RD4 position to select PWM output. It should be in the RG9 position to select the SPI SS function. JP4 will
normally be in the RD4 position. In general, the only time it needs to be in the RG9 position is when the Uno32
board is being used as an SPI slave device.

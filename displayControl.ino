//the opcodes for the MAX7221 and MAX7219
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

LedControl::LedControl(int dataPin, int clkPin, int csPin1,int csPin2, int numDevices) {
    SPI_MOSI=dataPin;
    SPI_CLK=clkPin;
    SPI_CS1=csPin1;
    SPI_CS2=csPin2;
    if(numDevices<=0 || numDevices>8 ) { numDevices=8; };
    maxDevices=numDevices;
    pinMode(SPI_MOSI,OUTPUT);
    pinMode(SPI_CLK,OUTPUT);
    pinMode(SPI_CS1,OUTPUT);
    digitalWrite(SPI_CS1,HIGH);
    pinMode(SPI_CS2,OUTPUT);
    digitalWrite(SPI_CS2,HIGH);
    SPI_MOSI=dataPin;
    for(int i=0;i<64;i++) 
        status[i]=0x00;
    for(int i=0;i<maxDevices;i++) {
        spiTransfer(i,OP_DISPLAYTEST,0);
        //scanlimit is set to max on startup
        setScanLimit(i,7);
        //decode is done in source
        spiTransfer(i,OP_DECODEMODE,0);
        clearDisplay(i);
        //we go into shutdown-mode on startup
        shutdown(i,true);
    }
}

int LedControl::getDeviceCount() { return maxDevices; }

void LedControl::shutdown(int addr, bool b) {
    if(addr<0 || addr>=maxDevices) { return; }
    if(b) { spiTransfer(addr, OP_SHUTDOWN,0); }
    else { spiTransfer(addr, OP_SHUTDOWN,1); }
}

void LedControl::setScanLimit(int addr, int limit) {
    if(addr<0 || addr>=maxDevices) { return; }
    if(limit>=0 && limit<8) { spiTransfer(addr, OP_SCANLIMIT,limit); }
}

void LedControl::setIntensity(int addr, int intensity) {
    if(addr<0 || addr>=maxDevices) { return; }
    if(intensity>=0 && intensity<16) { spiTransfer(addr, OP_INTENSITY,intensity); }
}

void LedControl::clearDisplay(int addr) {
    int offset;
    if(addr<0 || addr>=maxDevices) { return; }
    offset=addr*8;
    for(int i=0;i<8;i++) {
        status[offset+i]=0;
        spiTransfer(addr, i+1,status[offset+i]);
    }
}

void LedControl::setLed(int addr, int row, int column, boolean state) {
    int offset;
    byte val=0x00;

    if(addr<0 || addr>=maxDevices) { return; }
    if(row<0 || row>7 || column<0 || column>7) { return; }
    offset=addr*8;
    val=B10000000 >> column;
    if(state) { status[offset+row]=status[offset+row]|val; }
    else {
        val=~val;
        status[offset+row]=status[offset+row]&val;
    }
    spiTransfer(addr, row+1,status[offset+row]);
}

void LedControl::setRow(int addr, int row, byte value) {
    int offset;
    if(addr<0 || addr>=maxDevices) { return; }
    if(row<0 || row>7) { return; }
    offset=addr*8;
    status[offset+row]=value;
    spiTransfer(addr, row+1,status[offset+row]);
}

void LedControl::setColumn(int addr, int col, byte value) {
    byte val;
    if(addr<0 || addr>=maxDevices) { return; }
        return;
    if(col<0 || col>7) { return; }
    for(int row=0;row<8;row++) {
        val=value >> (7-row);
        val=val & 0x01;
        setLed(addr,row,col,val);
    }
}

void LedControl::spiTransfer(int addr, volatile byte opcode, volatile byte data) {
    //Create an array with the data to shift out
    int module_CS = SPI_CS1;
    if(addr>3) { module_CS=SPI_CS2; addr=addr-4; }
    int offset=addr*2;
    int maxbytes=maxDevices*2;

    for(int i=0;i<maxbytes;i++) { spidata[i]=(byte)0; }
    //put our device data into the array
    spidata[offset+1]=opcode;
    spidata[offset]=data;
    //enable the line 
    digitalWrite(module_CS,LOW);
	delayMicroseconds(10);
    //Now shift out the data 
    for(int i=maxbytes;i>0;i--) { shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]); }
	//for(int i=maxbytes;i>0;i--) { myShiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]); }
    //latch the data onto the display
    digitalWrite(module_CS,HIGH);
}    

// version alternative à shiftOut à utiliser si le ESP32 est trop rapide pour l'afficheur
void LedControl::myShiftOut(int dataPin, int clockPin, int bitOrder, int val) {
  int i;
  digitalWrite(clockPin, LOW);
  for (i = 0; i < 8; i++)  {
    if (bitOrder == LSBFIRST) {
      digitalWrite(dataPin, val & 1);
      val >>= 1;
    } else {
      digitalWrite(dataPin, (val & 128) != 0);
      val <<= 1;
    } 
    delayMicroseconds(10); 
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(10); 
    digitalWrite(clockPin, LOW);    
} }



void setup() 
{
    Serial.begin(9600);

    // configure signal input pins 
    pinMode(5,INPUT);  // IO request (active=0) 
    pinMode(6,INPUT);  // RWB (read=1) 

    // configure bidirectional data pins to default non-pullup tri-state
    pinMode(A0,INPUT);   // D0
    pinMode(A1,INPUT);   // D1
    pinMode(A2,INPUT);   // D2
    pinMode(A3,INPUT);   // D3
    pinMode(A4,INPUT);   // D4
    pinMode(A5,INPUT);   // D5
    pinMode(3,INPUT);    // D6
    pinMode(4,INPUT);    // D7

    // configure signal output pints
    digitalWrite(7,HIGH);
    pinMode(7,OUTPUT);   // IO ACK
}

void loop() 
{
    byte data;

    int tries;
    for (tries=0; tries<100000; tries++)
    {
        // wait until the 65c02 requests IO
        while ((PIND & 0x20) == 0)  { }; 

        // read access
        if (PIND & 0x40) 
        {
            // perform io operation
            data = readIO();
                      
            // distribute the byte to port C and port D (keep ack inactive yet)
            PORTC = data & 0x3F;
            PORTD = ((data>>3) & 0x18) | 0x80;
            DDRC = 0x3F;
            DDRD = 0x98;
            
            // add small delay to let outgoing signals get through the resistors
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
            DDRD = 0x98;
                
            noInterrupts();

            // pulse the ACK for exactly one clock 
            PORTD = ((data>>3) & 0x18);
            PORTD = 0x98;

            // bring bidirectional ports back to idle state
            DDRC = 0x00;
            DDRD = 0x80;
            PORTC = 0x00;
            
            interrupts();
        }
        // write access
        else
        {
            // small delay to let inputs stablize
            PORTD = 0x80;
            PORTD = 0x80;
            PORTD = 0x80;
            PORTD = 0x80;
           
            // collect data from port C and D
            data = (PINC & 0x3F) | ((PIND & 0x18) << 3);

            // perform io operation
            writeIO(data);

            // pulse the ACK for exactly one clock 
            noInterrupts();
            
            PORTD = 0x00;
            PORTD = 0x80;
            
            interrupts();
        }
    }

    // stop program
    for (;;);
}


byte romcounter = 0;
const PROGMEM byte rom[] = 
{   
    // First data the 65c02 reads from IO space are NOP instructions. 
    // When interpreted as the reset vector, the program counter will
    // be set to EAEA, and quite some additional NOPs are executed 
    0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 
    0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
    // because I do not know which byte the 65c02 takes for the 
    // opcode, and which byte is just an idle fetch, I insert
    // a specific command that is crafted to bring the 65c02 
    // onto the right track.
    // this is either LDA zp,x  (3 data fetches)
    // or a           LDA zp    (2 data fetches)
    0xB5, 0XA5, 0x00,        
    // build this copy loop at address $0000:
    //      0000 AD FF FF   LDA $FFFF
    //      0003 8D 00 20   STA $0200
    //      0006 E6 04      INC $04
    //      0008 D0 F6      BNE -10
    //      000A E6 05      INC $05
    //      000C 10 F2      BPL -14
    //      000E 4C 00 70   JMP $7000
    0xA9, 0xAD, 0x85, 0x00,   // LDA imm, STA zp
    0xA9, 0xFF, 0x85, 0x01,   // LDA imm, STA zp
    0xA9, 0xFF, 0x85, 0x02,   // LDA imm, STA zp
    0xA9, 0x8D, 0x85, 0x03,   // LDA imm, STA zp
    0xA9, 0x00, 0x85, 0x04,   // LDA imm, STA zp
    0xA9, 0x02, 0x85, 0x05,   // LDA imm, STA zp
    0xA9, 0xE6, 0x85, 0x06,   // LDA imm, STA zp
    0xA9, 0x04, 0x85, 0x07,   // LDA imm, STA zp
    0xA9, 0xD0, 0x85, 0x08,   // LDA imm, STA zp
    0xA9, 0xF6, 0x85, 0x09,   // LDA imm, STA zp
    0xA9, 0xE6, 0x85, 0x0A,   // LDA imm, STA zp
    0xA9, 0x05, 0x85, 0x0B,   // LDA imm, STA zp
    0xA9, 0x10, 0x85, 0x0C,   // LDA imm, STA zp
    0xA9, 0xF2, 0x85, 0x0D,   // LDA imm, STA zp
    0xA9, 0x4C, 0x85, 0x0E,   // LDA imm, STA zp
    0xA9, 0x00, 0x85, 0x0F,   // LDA imm, STA zp
    0xA9, 0x02, 0x85, 0x10,   // LDA imm, STA zp
    // start the copy loop
    0x4C, 0x00, 0x00,         // JMP $0000

    // this is the actual program that will go to $0200
    0xA2, 0x00,                             // 0200  LDX #0
    0xA0, 0x0D,                             // 0202  LDY #13
    0xBD, 0x10, 0x02,                       // 0204  LDA $0210,X
    0x8D, 0xFF, 0xFF,                       // 0207  STA $FFFF
    0xE8,                                   // 020A  INX
    0x88,                                   // 020B  DEY
    0xD0, 0xF6,                             // 020C  BNE -10
    0xF0, 0xFE,                             // 020E  BEQ -2
    72,69,76,76,79,32,87,79,82,76,68,33,10, // 0210  "HELLO WORLD!\n"
    
    // fill rest of memory with NOPs
    0xEA
};



byte readIO()
{
    byte value;

//    delay(1);
    
    value = pgm_read_byte(&rom[romcounter]); 
    
    if (romcounter<sizeof(rom)-1) 
    { romcounter++;
    }
//    Serial.print("READ ");
//    Serial.print(value,HEX);    
//    Serial.println();

    return value;
}

void writeIO( byte value)
{
//    Serial.print("WRITE ");
//    Serial.print(value, HEX);
//    Serial.println();
    Serial.write(value);
}

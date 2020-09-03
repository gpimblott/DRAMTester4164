#define setPort(value, bit, bitvalue) bitWrite(value,bit,bitvalue);

#define HIGH_MEM   10
#define LOW_MEM    11
#define GREEN_LED  12
#define RED_LED    13

#define BIT0       18
#define BIT1       2
#define BIT2       19
#define BIT3       6
#define BIT4       5
#define BIT5       4
#define BIT6       7
#define BIT7       3

#define RAS         17
#define CAS         9
#define D           15
#define Q           8
#define WRITE       16

#define RAS_PORT    PORTC
#define RAS_BIT     3
#define CAS_PORT    PORTB
#define CAS_BIT     1
#define WRITE_PORT  PORTC
#define WRITE_BIT   2
#define D_PORT      PORTC
#define D_BIT       1
#define Q_PORT      PORTB
#define Q_BIT       0

bool upperMem = true;
bool lowerMem = true;

/**
   Setup the pin input/output states
*/
void setup()
{
  Serial.begin(9600);

  // Setup the config switches and LED's
  pinMode(HIGH_MEM, INPUT);
  pinMode(LOW_MEM, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Setup pin modes
  // Address lines
  pinMode(BIT0, OUTPUT);
  pinMode(BIT1, OUTPUT);
  pinMode(BIT2, OUTPUT);
  pinMode(BIT3, OUTPUT);
  pinMode(BIT4, OUTPUT);
  pinMode(BIT5, OUTPUT);
  pinMode(BIT6, OUTPUT);
  pinMode(BIT7, OUTPUT);

  // Data lines
  pinMode(D, OUTPUT);
  pinMode(Q, INPUT);

  // Row/Column select
  pinMode(RAS, OUTPUT);
  pinMode(CAS, OUTPUT);
  pinMode(WRITE, OUTPUT);

  // Read the jumper values
  lowerMem = !digitalRead( LOW_MEM );
  upperMem = !digitalRead( HIGH_MEM );

  if (!lowerMem && !upperMem ) {
    Serial.println("Testing all addresses");
  } else {
    Serial.println("Testing " + String(lowerMem ? "LOWER" : "UPPER") + " 32K");
  }

  // set the intial states for RAS, CAS and WRITE_ENABLE
  digitalWrite(WRITE, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);

  // Read to go blink to LEDS to indicate we are starting
  blinkLeds( 2 , true, true);

  noInterrupts();
}


// Keep track of the state of the tests
int runNum = 0;
boolean failed = false;

/**
    Keep looping through the tests until it fails
*/
void loop()
{
  int testType = runNum % 3;
  int expectedBits = 0;

  interrupts();
  if ( failed ) {
    blinkLeds( testType + 1, true, false );
    delay(1000);
    return;
  };

  // Set LED's to indicate test start
  setLEDs( false, true );

  Serial.println("Starting test " + String(runNum) + " (" + String(testType) + ")");

  for (int  row = 0; row <= 255; row++) {
    //Serial.println("Testing row " + String(row));

    // do the actual test by writing to the row based on the type of test
    writeBits(row , testType );
    int numberOfBits = readBits(row);

    switch ( testType ) {
      case 0 :
        expectedBits = 256;
        break;
      case 1 :
        expectedBits = 0;
        break;
      case 2 :
        expectedBits = 128;
        break;
    }

    if (numberOfBits != expectedBits) {
      failed = true;
      Serial.println("ERROR: row " + String(row) + " number of bits was: " + String(numberOfBits) + ", but should be " + String(expectedBits));
      return;
    }
  }

  // Flash green
  blinkLeds( 1 , false, true);

  Serial.println("Test finished");
  runNum++;
}

/**
   Write the

   RAS         17 PC3
   CAS         9  PB1
   D           15 PC1
   Q           8  PB0
   WRITE       16 PC2
*/
void writeBits(int row, int testType) {

  // Pull RAS and CAS HIGH
  setPort( RAS_PORT , RAS_BIT , HIGH);
  setPort( CAS_PORT , CAS_BIT , HIGH );

  // Pull write enable HIGH
  setPort( WRITE_PORT, WRITE_BIT, HIGH );

  // Loop though all the columns writing the test data
  for (int i = 0; i <= 255; i++) {

    // Calculate the value we are writing for this test
    byte value = 0;
    switch ( testType ) {
      case 0 :
        value = HIGH;
        break;
      case 1 :
        value = LOW;
        break;
      case 2 :
        value = bitRead( i, 0);
        break;
    }

    // write the row index to the address lines
    writeRowAddress( row );

    // Pull Write LOW (Enables write)
    setPort( WRITE_PORT, WRITE_BIT, LOW );

    // Write the value to the D pin
    setPort(D_PORT, D_BIT, value );

    // Set column index
    writeColumnAddress( i );

    // Pull RAS and CAS HIGH to indicate that it's all ready
    setPort( RAS_PORT, RAS_BIT, HIGH);
    setPort( CAS_PORT, CAS_BIT, HIGH );
  }
}

/**
   Read the number of set bits on the specified row
*/
int readBits(int row) {

  // Bit counter
  int numberOfBits = 0;

  // Pull RAS, CAS and Write HIGH
  setPort( RAS_PORT, RAS_BIT, HIGH);
  setPort( CAS_PORT, CAS_BIT, HIGH);
  setPort( WRITE_PORT, WRITE_BIT, HIGH);

  // Loop though all the columns
  for (int i = 0; i <= 255; i++) {
    // Set row address
    writeRowAddress( row );

    // Set column address
    writeColumnAddress( i );

    // Read the stored bit and add to bit counter
    numberOfBits += digitalRead(Q);//(PINB & 1);

    // Pull RAS and CAS HIGH
    setPort(RAS_PORT, RAS_BIT, HIGH);
    setPort(CAS_PORT, CAS_BIT, HIGH);
  }

  return numberOfBits;
}

/**
   Write the address for the row using the definitions that we have setup
   I am writing to ports directly rather than using digitalWrite for performance (it's much faster)

   BIT0       18 PC4
   BIT1       2  PD2
   BIT2       19 PC5
   BIT3       6  PD6
   BIT4       5  PD5
   BIT5       4  PD4
   BIT6       7  PD7
   BIT7       3  PD3
*/
void writeRowAddress( int row ) {
  setPort(PORTC, 4 , bitRead(row, 0) ); //PC4
  setPort(PORTD, 2 , bitRead(row, 1) ); //PD2
  setPort(PORTC, 5 , bitRead(row, 2) ); //PC5
  setPort(PORTD, 6 , bitRead(row, 3) ); //PD6
  setPort(PORTD, 5 , bitRead(row, 4) ); //PD5
  setPort(PORTD, 4 , bitRead(row, 5) ); //PD4
  setPort(PORTD, 7 , bitRead(row, 6) ); //PD7
  setPort(PORTD, 3 , bitRead(row, 7) ); //PD3

  // Pull RAS LOW to indicate that the row address is ready
  setPort(RAS_PORT, RAS_BIT, LOW);
}

/**
   Write the column address using the setup definitions
   Note: For spectrum Upper ram only either the top 32K or bottom 32K is ok as they used the chips as 32K
   If this is the case we are overwriting bit7 to be either high or low depending on the board jumpers

   BIT0       18 PC4
   BIT1       2  PD2
   BIT2       19 PC5
   BIT3       6  PD6
   BIT4       5  PD5
   BIT5       4  PD4
   BIT6       7  PD7
   BIT7       3  PD3
*/
void writeColumnAddress( int column ) {
  setPort(PORTC, 4 , bitRead(column, 0) ); //PC4
  setPort(PORTD, 2 , bitRead(column, 1) ); //PD2
  setPort(PORTC, 5 , bitRead(column, 2) ); //PC5
  setPort(PORTD, 6 , bitRead(column, 3) ); //PD6
  setPort(PORTD, 5 , bitRead(column, 4) ); //PD5
  setPort(PORTD, 4 , bitRead(column, 5) ); //PD4
  setPort(PORTD, 7 , bitRead(column, 6) ); //PD7

  if ( !lowerMem && !upperMem ) {
    setPort( PORTD, 3, bitRead(column, 7) ); //PD3
  } else {
    setPort(PORTD, 3 , upperMem ? HIGH : LOW ); //PD3
  }

  // Pull CAS LOW to indicate that the column address is ready
  setPort( CAS_PORT , CAS_BIT , LOW );
}

/**
   Blink the red and/or green LED's the specified number of times as a visual indicator
*/
void blinkLeds(int numTimes, bool red, bool green)
{

  for (int i = 0; i < numTimes; i++) {
    setLEDs( red, green );
    delay(500);
    setLEDs( false, false );
    delay(500);
  }
}

void setLEDs( bool red, bool green) {
  digitalWrite(GREEN_LED, green ? HIGH : LOW);
  digitalWrite(RED_LED, red ? HIGH : LOW);
}

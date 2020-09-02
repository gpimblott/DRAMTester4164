#define setPort(value, bit, bitvalue) bitWrite(value,bit,bitvalue);

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

byte bit7CAS = HIGH;

/**
   Setup the pin input/output states
*/
void setup()
{
  Serial.begin(9600);

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

  digitalWrite(WRITE, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(CAS, HIGH);

  noInterrupts();
}

unsigned long currentMillis = 0;
unsigned long scrollMillis = 0;

int runNum = 0;
boolean failed = false;

/**
   Keep looping through the tests until it fails
*/
void loop()
{
  interrupts();
  if ( failed ) return;

  int testType = runNum % 3;
  int expectedBits = 128;

  Serial.println("Starting test " + String(runNum) + " (" + String(testType) + ")");

  for (int  row = 0; row <= 255; row++) {
    //Serial.println("Testing row " + String(row));

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

  Serial.println("Test finished");
  runNum++;
}

/**
  #define RAS         17 PC3
  #define CAS         9  PB1
  #define D           15 PC1
  #define Q           8  PB0
  #define WRITE       16 PC2
*/
void writeBits(int row, int testType) {

  // Pull RAS and CAS HIGH ???
  setPort( RAS_PORT , RAS_BIT , HIGH);
  setPort( CAS_PORT , CAS_BIT , HIGH );
  //digitalWrite(RAS, HIGH);
  //digitalWrite(CAS, HIGH);

  //digitalWrite(WRITE, LOW);
  setPort( WRITE_PORT, WRITE_BIT, HIGH );

  // Loop though all the columns
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
    // Set row address
    writeRowAddress( row );

    // Pull Write LOW (Enables write)
    //digitalWrite(WRITE, LOW);
    setPort( WRITE_PORT, WRITE_BIT, LOW );

   // Write the value
    //digitalWrite(D, value);
    setPort(D_PORT, D_BIT, value );

    // Set column index
    writeColumnAddress( i, bit7CAS );

    // Pull RAS and CAS HIGH
    setPort( RAS_PORT, RAS_BIT, HIGH);
    setPort( CAS_PORT, CAS_BIT, HIGH );
    //digitalWrite(RAS, HIGH);
    //digitalWrite(CAS, HIGH);
  }
}

/**

*/
int readBits(int row) {

  // Bit counter
  int numberOfBits = 0;

  // Pull RAS, CAS and Write HIGH
  setPort( RAS_PORT, RAS_BIT, HIGH);
  setPort( CAS_PORT, CAS_BIT, HIGH);
  setPort( WRITE_PORT, WRITE_BIT, HIGH);
  //digitalWrite(RAS, HIGH);
  //digitalWrite(CAS, HIGH);
  //digitalWrite(WRITE, HIGH);

  // Loop though all the columns
  for (int i = 0; i <= 255; i++) {
    // Set row address
    writeRowAddress( row );

    // Set column address
    writeColumnAddress( i , bit7CAS );

    // Read the stored bit and add to bit counter
    numberOfBits += digitalRead(Q);//(PINB & 1);

    // Pull RAS and CAS HIGH
    setPort(RAS_PORT, RAS_BIT, HIGH);
    setPort(CAS_PORT, CAS_BIT, HIGH);
    //digitalWrite(RAS, HIGH);
    //digitalWrite(CAS, HIGH);
  }

  return numberOfBits;
}

//#define BIT0       18 PC4
//#define BIT1       2  PD2
//#define BIT2       19 PC5
//#define BIT3       6  PD6
//#define BIT4       5  PD5
//#define BIT5       4  PD4
//#define BIT6       7  PD7
//#define BIT7       3  PD3
void writeRowAddress( int row ) {
  setPort(PORTC, 4 , bitRead(row, 0) );
  setPort(PORTD, 2 , bitRead(row, 1) );
  setPort(PORTC, 5 , bitRead(row, 2) );
  setPort(PORTD, 6 , bitRead(row, 3) );
  setPort(PORTD, 5 , bitRead(row, 4) );
  setPort(PORTD, 4 , bitRead(row, 5) );
  setPort(PORTD, 7 , bitRead(row, 6) );
  setPort(PORTD, 3 , bitRead(row, 7) );

  // Pull RAS LOW
  setPort(RAS_PORT, RAS_BIT, LOW);
  //digitalWrite(RAS, LOW);
}

void writeColumnAddress( int column , byte bit7 ) {
  setPort(PORTC, 4 , bitRead(column, 0) );
  setPort(PORTD, 2 , bitRead(column, 1) );
  setPort(PORTC, 5 , bitRead(column, 2) );
  setPort(PORTD, 6 , bitRead(column, 3) );
  setPort(PORTD, 5 , bitRead(column, 4) );
  setPort(PORTD, 4 , bitRead(column, 5) );
  setPort(PORTD, 7 , bitRead(column, 6) );
  setPort(PORTD, 3 , bit7 );

  // Pull CAS LOW
  //digitalWrite(CAS, LOW);
  setPort( CAS_PORT , CAS_BIT , LOW );

}

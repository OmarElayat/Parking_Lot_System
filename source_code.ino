#include <LiquidCrystal.h>
#include <Keypad.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 137, 10);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


#define Password_Length 8 
const int capacity = 5;
const int opto_in = 52;
const int opto_out = 50;
const int gate_motor=53;
//const int motor1_in = 46;
const int direction_in = 47;
const int direction_out = 45; 
const int LED_green = 51;
const int LED_red = 49;
int x=0;
int free_slots = 0;
int opto_state_in = 0;
int opto_state_out = 0;
int count = 0;
int opto_in_flag = 1;
int opto_out_flag = 1;
bool open_gate_flag;

//////////////keypad definitions/////////////////
int signalPin = 53;

char Data[Password_Length]; 
char Master[Password_Length] = "1234567"; 
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '4', '7', '#'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '*'},
  {'!', '@', '$', '%'}
};
byte rowPins[ROWS] = {36,34,32,30}; //Rows 0 to 3

byte colPins[COLS]= {28,26,24,22}; //Columns 0 to 3

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


//////////////////LCD definitions///////////////////
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

unsigned long Timer =0;
void openGate() {
  //digitalWrite(motor1_in,HIGH);
  digitalWrite(direction_in,HIGH);
  digitalWrite(direction_out,LOW);
  //delay(5000);
  //digitalWrite(direction_in,LOW);
  //digitalWrite(direction_out,LOW);
 }
void disable_motor_signal(){
   digitalWrite(direction_in,LOW);
  digitalWrite(direction_out,LOW);
}
 void closeGate() {
  //digitalWrite(motor1_in ,HIGH);
  digitalWrite(direction_out,HIGH);
  digitalWrite(direction_in,LOW);
  //delay(5000);
 }
void setup()
{
  pinMode(opto_in, INPUT);
  pinMode(opto_out, INPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(gate_motor, OUTPUT);
 // pinMode(motor1_in, OUTPUT);
  pinMode(direction_in, OUTPUT);
  pinMode(direction_out, OUTPUT);
  pinMode(LED_red, OUTPUT);
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  /////////////////Ethernet Server Setup ///////////////////////////////////
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}
void loop()
{
  //////////////LEDs Contorl/////////////
  if(count < capacity){
   digitalWrite(LED_green,HIGH);
   digitalWrite(LED_red,LOW);
  }else{
   digitalWrite(LED_green,LOW);
   digitalWrite(LED_red,HIGH);
  }
  //////////////Gate Contorl/////////////
  digitalWrite(gate_motor,LOW);
  //////////////Optoisolators control/////////////
  opto_state_in = digitalRead(opto_in);
  while (opto_state_in == HIGH)
  {
    //Serial.print("in");
     digitalWrite(gate_motor,HIGH);
     openGate();
     open_gate_flag = true;
    //delay(5000);
    opto_state_out = digitalRead(opto_out);
    if((opto_state_out == HIGH) && opto_in_flag && count){
      count--;
      opto_in_flag = 0;
      Serial.print(count);
    }
    opto_state_in = digitalRead(opto_in);
  } 
  disable_motor_signal();
  opto_in_flag = 1;
  opto_state_out = digitalRead(opto_out);
  while (opto_state_out == HIGH)
  {
    if( (count >= capacity) && opto_out_flag){
      lcd.setCursor(0,0);
      lcd.print("Sorry, No Room");
      Serial.print("Sorry, No Room"); //put on lcd
      opto_in_flag = 0;
    }
    //Serial.print("out");


    
    if((count != capacity)){
        if(!x){
          lcd.clear();
          x++;
        }
         lcd.setCursor(0,0);
         lcd.print("Pw:");
        customKey = customKeypad.getKey();
      if (customKey){
        Data[data_count] = customKey; 
        lcd.setCursor(data_count+3,0); 
        //lcd.print(Data[data_count]); 
        lcd.print('*');
        data_count++; 
      }

      if(data_count == Password_Length-1){
      if(!strcmp(Data, Master)){
        lcd.setCursor(0,1);
        lcd.print("Correct     ");
        digitalWrite(gate_motor, HIGH);
        openGate();
        open_gate_flag = true;

      }
      else{
        lcd.setCursor(0,1);
        lcd.print("Incorrect ");
        delay(1000);
        lcd.setCursor(0,1);
        lcd.print("Try Again");
        lcd.setCursor(3,0); 
        lcd.print("        ");
      }
    
      clearData();  
    }
    }
    
    //delay(5000);
    opto_state_in = digitalRead(opto_in);
    if((opto_state_in == HIGH) && opto_out_flag && (count != capacity)){
   
      count++;
      opto_out_flag = 0;
      //Serial.print(count);
    }
    opto_state_out = digitalRead(opto_out);
  }
  x = 0;
  opto_out_flag = 1;
  disable_motor_signal();
  if(!opto_state_in && !opto_state_out && open_gate_flag){
  //  Serial.print(count);
  closeGate();
  delay(5000);
  open_gate_flag = false;
  }
 int val; // variable to store the value coming from the analog pin
 double data =0; // variable to store the temperature value coming from the conversion formula
 val=analogRead(1); // read the analog in value:
 data = (double) (val * 330)/1024; // temperature conversion formula
 free_slots = capacity-count;
 if((millis() - Timer) > 500){ // output a temperature and car count values per 500ms
 Timer = millis();
 // print the results to the lcd
 lcd.setCursor(0,0);
 lcd.print("Free Slots:   ");
 lcd.setCursor(12,0);
 lcd.print(free_slots);
 lcd.setCursor(0,1);
 lcd.print("Temp.: ");
 lcd.setCursor(7,1);
 lcd.print(data);
 lcd.setCursor(13,1);
 lcd.print("C");
 }

////////////////////Ethernet Server////////////////////////////////

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
            client.print("Free Slots : ");
            client.print(free_slots);
            client.println("<br />");
            client.print("The Gate is = ");
            if (free_slots == 0){
             client.print("Close");
            } else {
             client.print("Open");
            }
            client.println("<br />");
            client.print("Temperature : ");
            client.print(data);
            client.println("<br />");
 
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

 
}

void clearData(){
  while(data_count !=0){
    Data[data_count--] = 0; 
  }
}

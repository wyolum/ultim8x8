#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
const int n_node = 7;

uint8_t myRadioNum = 0; // 0, 1, 2, 3, 4, 5, 6

RF24 radio(9, 10);

// Radio pipe addresses for the 7 nodes to communicate.
byte addresses[][6] = {"0Node", "1Node","2Node","3Node","4Node","5Node","6Node"};


// A single byte to keep track of the data being sent back and forth
byte count = 1; 

void setup(){
  Serial.begin(115200);
  radio.begin();
  radio.enableAckPayload();                     // Allow optional ack payloads
  radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
  
  for(byte radioNum=0; radioNum < n_node; radioNum++){
    if(radioNum != myRadioNum){
      radio.openReadingPipe(radioNum,addresses[radioNum]);      // Open a reading pipe on address 0, pipe 1
    }
  }
  radio.openWritingPipe(addresses[(3) % 6]);
  if(myRadioNum == 0){
    while(1){
      byte msg = 123;
      if(radio.write(&msg,1)){
	Serial.println("Woot!");
      }
      else{
	Serial.println("boo!");
      }
      delay(1000);
    }
  }
  radio.startListening();                       // Start listening  
}

void loop(){
  unsigned long time = micros();                              // Record the current microsecond count   
  byte gotByte;                                               // Initialize a variable for the incoming response
  
  if(myRadioNum == 0){
    Serial.print("Now sending ");                             // Use a simple byte count as payload
    Serial.println(count);
    for(byte radioNum=1; radioNum < n_node; radioNum++){
      radio.stopListening();                                  // First, stop listening so we can talk.      
      radio.openWritingPipe(addresses[radioNum]);
      if(radio.write(&count,1)){                         // Send the count variable to the other radios
	if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
	  Serial.print(F("Got blank response. round-trip delay: "));
	  Serial.print(micros()-time);
	  Serial.println(F(" microseconds"));     
	}
	else{      
	  while(radio.available()){                    // If an ack with payload was received
	    radio.read(&gotByte, 1);                  // Read it, and display the response time
	    unsigned long timer = micros();
	    
	    Serial.print(F("Got response "));
	    Serial.print(gotByte);
	    Serial.print(F(" round-trip delay: "));
	    Serial.print(timer-time);
	    Serial.println(F(" microseconds"));
	  }
	}
      }
      else{
	Serial.print(radioNum);
	Serial.println(F(", send failed."));         // If no ack response, sending failed
      }
    }
    delay(1000);
    count++;                                  // Increment the count variable
  }
  else{
    byte pipeNo, gotByte;                          // Declare variables for the pipe and the byte received
    while(radio.available(&pipeNo)){               // Read all available payloads
      radio.read(&gotByte, 1 );                   
                                                   // Since this is a call-response. Respond directly with an ack payload.
      gotByte += 1;                                // Ack payloads are much more efficient than switching to transmit mode to respond to a call
      radio.writeAckPayload(pipeNo,&gotByte, 1 );  // This can be commented out to send empty payloads.
      Serial.print(F("Loaded next response "));
      Serial.println(gotByte);  
    }
  }
}

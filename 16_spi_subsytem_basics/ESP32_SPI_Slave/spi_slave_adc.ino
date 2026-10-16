#include <Arduino.h>
#include <ESP32SPISlave.h>

#define LED_PIN         22
#define BUFF_LEN        1

volatile bool     state     = false;

ESP32SPISlave slave;

// SPI buffers (single- byte transaction)
uint8_t tx_buffer = 0x34;
uint8_t rx_buffer = 0x00;

void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  slave.setDataMode(SPI_MODE0);
  slave.setQueueSize(1);
  if(!slave.begin(VSPI)) {
    Serial.println("ESP32SPISlave begin() Failed!!");
    while(1) delay(1000);
  }
}

void loop(){

  if(state == true)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  delay(50);

  size_t got = slave.transfer(&tx_buffer, &rx_buffer, BUFF_LEN, 0); // 0 = wait forever 
  if(got > 0){
    Serial.printf("Reciver 0x%x\n", rx_buffer);
    state = !state;
  }
}
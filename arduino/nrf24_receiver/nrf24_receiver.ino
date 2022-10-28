#include <SPI.h>
#include <RF24.h>

#define RF24_ADDR "69420"
#define RF24_CHAN 69

RF24 radio(7, 8);

// #define BUTTONS
// #define MAG
// #define ACCEL
#define AUDIO

typedef struct {
    uint8_t btn0 : 1;
    uint8_t btn1 : 1;
    uint8_t btn2 : 1;
    uint8_t rsvd : 4;
    uint8_t mag  : 1; // 0=accel, 1=mag
    int16_t x; // little endian!
    int16_t y;
    int16_t z;
    uint8_t audio[22]; // 22kHz audio
} __attribute__((packed)) app_packet_S;

void setup() {
    Serial.begin(1000000);

    SPI.setClockDivider(0);
    if (!radio.begin()) {
        Serial.println("hardware not responding!");
    }
    radio.setPALevel(RF24_PA_MAX);
    radio.setChannel(RF24_CHAN);
    radio.setDataRate(RF24_1MBPS);
    radio.enableDynamicPayloads();
    
    radio.openReadingPipe(0, (uint8_t*) RF24_ADDR);
    radio.startListening();
}

void loop() {
    uint8_t pipe;
    if (radio.available(&pipe)) {
        app_packet_S data;
        radio.read(&data, sizeof(data)); // assume always right size

#ifdef BUTTONS
        Serial.print(data.btn0); Serial.print(" ");
        Serial.print(data.btn1); Serial.print(" ");
        Serial.println(data.btn2);
#endif

#ifdef MAG
        if (data.mag) {
            Serial.print(data.x); Serial.print(" ");
            Serial.print(data.y); Serial.print(" ");
            Serial.println(data.z);
        }
#endif

#ifdef ACCEL
        if (!data.mag) {
            Serial.print(data.x); Serial.print(" ");
            Serial.print(data.y); Serial.print(" ");
            Serial.println(data.z);
        }
#endif

#ifdef AUDIO
        // suitable for raw retrieval using pyserial
        Serial.write(data.audio, sizeof(data.audio));
#endif
    }
}

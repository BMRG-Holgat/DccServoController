#include "dcc_handler.h"
#include "servo_controller.h"
#include "config.h"
#include "utils/dcc_debug_logger.h"

// External functions from main.cpp
extern void triggerDccSignal();
extern void addDccLogMessage(const String& message);

// Global DCC objects
NmraDcc Dcc;
DCC_MSG Packet;

CVPair FactoryDefaultCVs[] = {
    {CV_ACCESSORY_DECODER_ADDRESS_LSB, 1},
    {CV_ACCESSORY_DECODER_ADDRESS_MSB, 0},
}; 

uint8_t FactoryDefaultCVIndex = 0;

void initializeDCC() {
    // DCC setup: External Interrupt, Pin, and enable Pull-Up 
    // For the circuit, DCC is active low through a pulldown diode and a series 1k res, hence need the pull-up
    // For ESP32, we use GPIO 2 for DCC input
    Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, true); // Pin 2, enable pull-up

    // Call the main DCC Init function to enable the DCC Receiver
    Dcc.init(MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0);
    Serial.println("DCC Init Done");
}

void processDCC() {
    Dcc.process();

    if (FactoryDefaultCVIndex && Dcc.isSetCVReady()) {
        FactoryDefaultCVIndex--; // Decrement first as initially it is the size of the array 
        Dcc.setCV(FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
    }
}

// DCC callback functions
void notifyDccAccTurnoutBoard(uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower) {
    Serial.print("notifyDccAccTurnoutBoard: ");
    Serial.print(BoardAddr, DEC);
    Serial.print(',');
    Serial.print(OutputPair, DEC);
    Serial.print(',');
    Serial.print(Direction, DEC);
    Serial.print(',');
    Serial.println(OutputPower, HEX);
    // Does nothing more, we are not using it
}

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
    // Check if this address matches any of our configured servos
    bool isOurAddress = false;
    for (auto &sv : virtualservo) {
        if (Addr == sv.address && sv.address != 0) {
            isOurAddress = true;
            break;
        }
    }
    
    // Only trigger signal indication for our configured addresses
    if (isOurAddress) {
        triggerDccSignal();
    }
    
    // Debug output if enabled
    if (dccDebugLogger.isDebugEnabled()) {
        String debugMsg = "Addr=" + String(Addr) + ", Dir=" + String(Direction) + 
                         ", Pwr=" + String(OutputPower, HEX) + (isOurAddress ? " [MATCH]" : " [ignore]");
        Serial.print("DCC RX: ");
        Serial.println(debugMsg);
        addDccLogMessage(debugMsg);
    }

    if (!isOurAddress) return;  // Only process packets for our addresses

    VIRTUALSERVO *targetVirtualServo = nullptr;

    // Act on the data, first locate the appropriate servo slot
    for (auto &sv : virtualservo) {
        if (Addr == sv.address) {
            targetVirtualServo = &sv;
            
            if (targetVirtualServo == nullptr) continue;
            
            // Take action. 0 is closed, 1 thrown
            if (Direction == 0) {
                targetVirtualServo->state = SERVO_TO_CLOSED;
            } else {
                targetVirtualServo->state = SERVO_TO_THROWN;
            }
            
            if (dccDebugLogger.isDebugEnabled()) {
                String servoMsg = "Servo action: Pin " + String(targetVirtualServo->pin) + 
                                " -> " + String(Direction == 0 ? "CLOSED" : "THROWN");
                Serial.println(servoMsg);
                addDccLogMessage(servoMsg);
            }
        }
    }
}

#ifdef NOTIFY_DCC_MSG
void notifyDccMsg(DCC_MSG *Msg) {
    Serial.print("notifyDccMsg: ");
    for (uint8_t i = 0; i < Msg->Size; i++) {
        Serial.print(Msg->Data[i], HEX);
        Serial.write(' ');
    }
    Serial.println();
}
#endif

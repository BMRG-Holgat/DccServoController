#ifndef DCC_HANDLER_H
#define DCC_HANDLER_H

#include <NmraDcc.h>

// DCC Configuration Values
struct CVPair {
    uint16_t CV;
    uint8_t Value;
};

// Global DCC objects
extern NmraDcc Dcc;
extern DCC_MSG Packet;
extern CVPair FactoryDefaultCVs[];
extern uint8_t FactoryDefaultCVIndex;

// Function declarations
void initializeDCC();
void processDCC();

// DCC callback functions
void notifyDccAccTurnoutBoard(uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower);
void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower);

#ifdef NOTIFY_DCC_MSG
void notifyDccMsg(DCC_MSG *Msg);
#endif

#endif // DCC_HANDLER_H

#ifndef _IDRIVE_CAN_
#define _IDRIVE_CAN_

#define IDRIVE_BUS_ALIVE_TX_INT 1000000

//Use these with the CAN library in lieu of its built in values. They're the same, but it is easier to read.
#define IDRIVE_KCAN1_BUSRATE 100000
#define IDRIVE_KCAN2_BUSRATE 500000

#define IDRIVE_ID_ENCODER_INITREQUEST 0x273
#define IDRIVE_ID_BUS_ALIVE 0x560
#define IDRIVE_ID_ILLUMI_LEVEL 0x202

#define IDRIVE_ID_MAIN_DATA 0x267
#define IDRIVE_ID_ENCODER_DATA 0x264
#define IDRIVE_ID_ENCODER_INITREPLY 0x277
#define IDRIVE_ID_TOUCHPAD_DATA 0x0BF

#define IDRIVE_CAT_BUTTON 0xC0
#define IDRIVE_CAT_STK_BTN 0xDE
#define IDRIVE_CAT_STK_MOVE 0xDD

#define IDRIVE_VAL_OK 0
// These are all moved over 1 bit from their CAN values to make room for the above 'OK' button.
#define IDRIVE_VAL_MENU 1
#define IDRIVE_VAL_BACK 2
#define IDRIVE_VAL_OPTION 3
#define IDRIVE_VAL_RADIO 4
#define IDRIVE_VAL_MEDIA 5
#define IDRIVE_VAL_NAV 6
#define IDRIVE_VAL_TEL 7

#define IDRIVE_VAL_UP 4
#define IDRIVE_VAL_RIGHT 5
#define IDRIVE_VAL_DOWN 6
#define IDRIVE_VAL_LEFT 7

#define IDRIVE_VAL_TOUCH_0FINGER 0x11
#define IDRIVE_VAL_TOUCH_1FINGER 0x10
#define IDRIVE_VAL_TOUCH_2FINGER 0x00
#define IDRIVE_VAL_TOUCH_3FINGER 0x1F
#define IDRIVE_VAL_TOUCH_4FINGER 0x0F

#define IDRIVE_INSTATE_RELEASE 0
#define IDRIVE_INSTATE_SHORTPRESS 1
#define IDRIVE_INSTATE_LONGPRESS 2

#define IDRIVE_ENCODER_DISABLED 6
#define IDRIVE_ENCODER_ENABLED 1

#define IDRIVE_DIAG_STATUS 0x5E7
#define IDRIVE_DIAG_NETWORKERROR 0x4E7
#define IDRIVE_DIAG_TIMEOUT 0x567
#define IDRIVE_DIAG_ERROR 0x667

// Define byte-positions for things we care about.
#define IDRIVE_POS_STATUS_ENCODER 4
#define IDRIVE_POS_ENCODER_VALUE 3
#define IDRIVE_POS_MAIN_INPUT_SELECT 4
#define IDRIVE_POS_MAIN_BTN_SELECT 5
#define IDRIVE_POS_MAIN_BTN_STATE 3
#define IDRIVE_POS_MAIN_JOY_STATE 3

#include <Arduino.h>
#if defined(__AVR__)
  #include <avr_can.h>
#elif defined (__SAM3X8E__)
  #include <due_can.h>
#else 
  #error "Unsupported platform"
#endif
#include <CircularBuffer.h>

class iDriveController {
  private:
    uint8_t buttonStatus = 0;
    uint8_t stickStatus = 0;
    CircularBuffer<CAN_FRAME, 8>* txBuffer;
    void (*buttonCallback)(uint8_t);
    void (*encoderCallback)(int8_t);
    void (*joyCallback)(uint8_t);

  public:
    uint8_t encoderPosition = 0;

    uint8_t getButtonStatus();
    uint8_t getStickStatus();

    bool processMessage(CAN_FRAME &frame);
    bool checkMessage(uint8_t canID);
    void initEncoder();
    void busAlive();
    void setBacklight(uint8_t BacklightLevel);

    void setButtonCallback(void (*buttonCallback)(uint8_t));
    void setEncoderCallback(void (*encoderCallback)(int8_t));
    void setJoyCallback(void (*joyCallback)(uint8_t));

    iDriveController(CircularBuffer<CAN_FRAME, 8> *txBuffer);
};


#endif

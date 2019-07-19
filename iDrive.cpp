#include "iDrive.h"

uint8_t iDriveController::getButtonStatus() {
  return this->buttonStatus;
};
uint8_t iDriveController::getStickStatus() {
  return this->stickStatus;
};

bool iDriveController::processMessage(CAN_FRAME &frame) {
  switch (frame.id) {
    // Bus error handling (as transmitted by the controller)
    case IDRIVE_DIAG_STATUS:
      {
        if (frame.data.byte[IDRIVE_POS_STATUS_ENCODER] == 6) {
          this->initEncoder();
        }
        break;
      }
    case IDRIVE_DIAG_NETWORKERROR:
      {
        break;
      }
    case IDRIVE_DIAG_TIMEOUT:
      {
        break;
      }
    case IDRIVE_DIAG_ERROR:
      {
        break;
      }

    // Encoder init reply packet. Don't really need to do anything with it
    case IDRIVE_ID_ENCODER_INITREPLY:
      {
        break;
      }

    //Data packets
    case IDRIVE_ID_MAIN_DATA:
      {
        switch (frame.data.byte[IDRIVE_POS_MAIN_INPUT_SELECT]) {
          case IDRIVE_CAT_BUTTON:
            {
              // Everything is moved over by 1 bit to make room for the 'OK' button on the stick
              if (frame.data.byte[IDRIVE_POS_MAIN_BTN_STATE] != 0) {
                this->buttonStatus |= (frame.data.byte[IDRIVE_POS_MAIN_BTN_SELECT] << 1);
              } else {
                buttonStatus &= ~(frame.data.byte[IDRIVE_POS_MAIN_BTN_SELECT] << 1);
              }
              this->buttonCallback(this->buttonStatus);
              break;
            }
          case IDRIVE_CAT_STK_MOVE:
            {
              if (frame.data.byte[IDRIVE_POS_MAIN_JOY_STATE] & 1 || frame.data.byte[IDRIVE_POS_MAIN_JOY_STATE] & 2) {
                //If bits 0 or 1 are set to indicate the stick is off-centre.
                this->stickStatus |= (frame.data.byte[IDRIVE_POS_MAIN_JOY_STATE]);

              } else {
                //The stick is released.
                this->stickStatus = 0; //Wow, no logic needed. Just zero it!
              }
              this->joyCallback(this->stickStatus);
              break;
            }
          case IDRIVE_CAT_STK_BTN:
            {
              if (frame.data.byte[IDRIVE_POS_MAIN_BTN_STATE] != 0) {
                this->buttonStatus |= 1;
              } else {
                buttonStatus &= ~1;
              }
              this->buttonCallback(this->buttonStatus);
              break;
            }
        }
        break;
      }
    case IDRIVE_ID_ENCODER_DATA:
      {
        noInterrupts();
        if (this->encoderPosition < 127 && frame.data.byte[IDRIVE_POS_ENCODER_VALUE] > 127) {
          this->encoderCallback(-((255 - frame.data.byte[IDRIVE_POS_ENCODER_VALUE]) + this->encoderPosition)); //Negative move with overrun
        } else if (this-> encoderPosition > frame.data.byte[IDRIVE_POS_ENCODER_VALUE]) {
          this->encoderCallback(-(this->encoderPosition - frame.data.byte[IDRIVE_POS_ENCODER_VALUE])); //Negative move without overrun
        } else {
          this->encoderCallback((frame.data.byte[IDRIVE_POS_ENCODER_VALUE] - this->encoderPosition)); // Positive move
        }
        this->encoderPosition = frame.data.byte[IDRIVE_POS_ENCODER_VALUE];
        interrupts();
        break;
      }
    case IDRIVE_ID_TOUCHPAD_DATA:
      {
        break;
      }

    default:
      {
        break;
      }
  }
};
void iDriveController::initEncoder() {
  CAN_FRAME encoderInitFrame;
  encoderInitFrame.id = 0x273;
  encoderInitFrame.extended = 0;
  encoderInitFrame.length = 8;
  encoderInitFrame.data.low = 0xf000e11d;
  encoderInitFrame.data.high = 0x04de7fff;

  this->txBuffer->push(encoderInitFrame);
};

void iDriveController::busAlive() {
  CAN_FRAME busAliveFrame;
  busAliveFrame.id = 0x560;
  busAliveFrame.extended = 0;
  busAliveFrame.length = 8;
  busAliveFrame.data.low = 0x00000000;
  busAliveFrame.data.high = 0x60002f57;

  this->txBuffer->push(busAliveFrame);

}

void iDriveController::setBacklight(uint8_t BacklightLevel) {

  //BMW logic... don't ask.
  if (BacklightLevel > 253) {
    BacklightLevel = 253;
  } else if (BacklightLevel == 0) {
    BacklightLevel = 255;
  }

  CAN_FRAME illumiStatusFrame;
  illumiStatusFrame.id = 0x202;
  illumiStatusFrame.extended = 0;
  illumiStatusFrame.length = 2;
  illumiStatusFrame.data.byte[1] = 0x0;
  illumiStatusFrame.data.byte[0] = BacklightLevel;

  this->txBuffer->push(illumiStatusFrame);
}


iDriveController::iDriveController(CircularBuffer<CAN_FRAME, 8> *txBuffer) {
  this->txBuffer = txBuffer;
};

void iDriveController::setButtonCallback(void (*buttonCallback)(uint8_t)) {
  this->buttonCallback = buttonCallback;
};

void iDriveController::setJoyCallback(void (*joyCallback)(uint8_t)) {
  this->joyCallback = joyCallback;
};

void iDriveController::setEncoderCallback(void (*encoderCallback)(int8_t)) {
  this->encoderCallback = encoderCallback;
};

#include <avr/pgmspace.h>

#define TRUE 1
#define FALSE 0
#define INIT_DEFAULT_DELAY 5
#define FLASH_SIZE 135

const PROGMEM uint8_t eeprom  [135] = {0xDF, 0X32, 0X00, 0x00, 0xDE, 0XB8, 0X0B, 0x00, 0xAF, 0xe0, 0xAF, 0xe2, 0x17, 0xAE, 0x00, 0X08, 0X06, 0X0B, 0X12, 0X2C, 0xAF, 0xE1, 0X34, 0xAF, 0xE1, 0X17, 0X0B, 0X0C, 0X16, 0X2C, 0X0C, 0X16, 0X2C, 0X04, 0X2C, 0X07, 0X08, 0X10, 0X12, 0xAF, 0xE1, 0X34, 0x00, 0xDE, 0XFA, 0X00, 0x00, 0x28, 0xAE, 0xA7, 0X64, 0X00, 0x00, 0xDE, 0XB8, 0X0B, 0x00, 0xAF, 0xe1, 0x1e, 0xAE, 0x00, 0xAF, 0xe2, 0x3d, 0xAE, 0x00, 0xAF, 0xe0, 0x06, 0xAE, 0x00, 0xAF, 0xe3, 0x15, 0xAE, 0x00, 0x76, 0xAE, 0x00, 0x51, 0xAE, 0x00, 0x50, 0xAE, 0x00, 0x4f, 0xAE, 0x00, 0x52, 0xAE, 0x00, 0x48, 0xAE, 0x00, 0x39, 0xAE, 0x00, 0x4c, 0xAE, 0x00, 0x4d, 0xAE, 0x00, 0x29, 0xAE, 0x00, 0x4a, 0xAE, 0x00, 0x49, 0xAE, 0x00, 0x53, 0xAE, 0x00, 0x4b, 0xAE, 0x00, 0x4e, 0xAE, 0x00, 0x46, 0xAE, 0x00, 0x47, 0xAE, 0x00, 0x2c, 0xAE, 0x00, 0x2b, 0xAE, 0x00, 0xFF};

/*
const PROGMEM uint8_t eeprom [64] = { 
	0xDF, 0X32, 0X00, 0x00, 0xAF, 0xe3, 0x15, 0xAE, \
	0x00, 0xDE, 0XF4, 0X01, 0x00, 0X11, 0X12, 0X17, \
	0X08, 0X13, 0X04, 0X07, 0x00, 0xDE, 0XF4, 0X01, \
	0x00, 0x28, 0xAE, 0x00, 0xDE, 0XEE, 0X02, 0x00, \
	0xAF, 0xE1, 0X0B, 0X08, 0X0F, 0X0F, 0X12, 0X2C, \
	0xAF, 0xE1, 0X1A, 0X12, 0X15, 0X0F, 0X07, 0xAF, \
	0xE1, 0X1E, 0xAF, 0xE1, 0X1E, 0xAF, 0xE1, 0X1E, \
	0x00, 0x28, 0xAE, 0x00, 0xFF, 0x00, 0x00, 0x00 };
*/

uint16_t EEprom_Addr = 0; // EEprom Address
uint16_t Last_EEprom_Addr = 0; // EEprom Address
uint16_t Replay_Counter = 0; 
uint16_t Default_Delay = 0;
uint16_t Time_to_Delay = 0;
uint8_t Hold_Next_Key = 0; // Hold Key flag
uint8_t Replaying = 0;
uint8_t c;
uint8_t End_of_Script;



//------------------------------------------------------------------------------

// delay while updating until we are finished delaying
void digiKeyboard_delay(uint16_t tdelay) {
	delay(tdelay);
}

//sendKeyPress: sends a key press only, with modifiers - no release
//to release the key, send again with keyPress=0
void sendKeyPress(byte keyPress, byte modifiers) {

	Serial.print ("[");
	Serial.print (keyPress,HEX);
	Serial.print (",");
	Serial.print (modifiers,HEX);
	Serial.print ("] ");

};


//sendKeyPress: sends a key press only - no release
//to release the key, send again with keyPress=0
void digiKeyboard_sendKeyPress(byte keyPress) {
	sendKeyPress(keyPress, 0);
};


//------------------------------------------------------------------------------

uint8_t Ext_EEpromRead (uint16_t address) {
	if (address < FLASH_SIZE) {
		return pgm_read_byte_near(eeprom + address);  
	} 
	else 
	return 0;
}

uint16_t Ext_EEpromRead16 (uint16_t address) {
	if (address < FLASH_SIZE-1) {
		return pgm_read_word_near(eeprom + address);  
	} 
	else 
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
        Serial.println("Iniciando:");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void loop() {

        EEprom_Addr=0;
	// Ugly Token Player
	while ( (EEprom_Addr < FLASH_SIZE) && !End_of_Script )  {
		c = Ext_EEpromRead( EEprom_Addr);
		switch (c) {
		case 0xff:    
			End_of_Script = TRUE ; 
                        Serial.println("Script End");
			break;

		case 0x00:    // end of command
			EEprom_Addr++;      // advance address
			Last_EEprom_Addr = EEprom_Addr; 
                        Serial.println(" Command End");
			break;             // save this address for eventual return
			
		case 0xdf: // Set Default Delay 

			Default_Delay = Ext_EEpromRead16 (EEprom_Addr +1);// read value
			if (Default_Delay==0)                             // check range
			Default_Delay = INIT_DEFAULT_DELAY;               //
			//Last_EEprom_Addr = EEprom_Addr;                 // advance address
			EEprom_Addr +=3; //
                        Serial.print("Set Def Delay: ");
                        Serial.print(Default_Delay);
                        break;

		case 0xde: // Delay
			Time_to_Delay = Ext_EEpromRead16 (EEprom_Addr +1);
			if (Time_to_Delay==0)                              // check range
			Time_to_Delay = INIT_DEFAULT_DELAY;            //
			//Last_EEprom_Addr = EEprom_Addr;                    // advance address
			EEprom_Addr +=3;                                   //
                        Serial.print("Delay: ");
                        Serial.print(Time_to_Delay);
                        break;

//		case 0xa5:    // start of block
//			EEprom_Addr++;      // advance address
//			Last_Block_EEprom_Addr = EEprom_Addr; 
//			break; 			
//			
//		case 0xA6: // Replay last block
//			if (Replaying_block) {
//				EEprom_Addr = Last_Block_EEprom_Addr;
//				Replay_Block_Counter--;
//				if (Replay_Block_Counter ==0) {  // replay is over!
//					EEprom_Addr += 3 ; // skip to the next command
//					Replaying_Block = FALSE;
//				}
//			} 
//			else {
//				Replaying_Block = TRUE;
//				Replay_BLock_Counter = Ext_EEpromRead16 (EEprom_Addr +1);
//				if (Replay_Block_Counter==0)                              // check range
//				Replay_Block_Counter = 1;            //
//				EEprom_Addr = Last_Block_EEprom_Addr;                    // rewind address                
//			}			
			
			
		case 0xa7: // Replay last command
			if (Replaying) {
				Replay_Counter--;
				if (Replay_Counter ==0) {  // replay is over!
					EEprom_Addr += 3 ; // skip to the next command
					Replaying = FALSE;
				} else
				        EEprom_Addr = Last_EEprom_Addr;

                                Serial.print("Replaying: ");
                                Serial.println(Replay_Counter);
			} 
			else {
				Replaying = TRUE;
				Replay_Counter = Ext_EEpromRead16 (EEprom_Addr +1);
				if (Replay_Counter==0)                              // check range
				Replay_Counter = 1;            //
				EEprom_Addr = Last_EEprom_Addr;                    // rewind address                
                                Serial.print("Replay Start: ");
                                Serial.println(Replay_Counter);
			}
                        break;

		case 0xae: // release all keys
                        Serial.print("Release");
			digiKeyboard_sendKeyPress(0);
			EEprom_Addr++;      // advance address	
			break ;                       

		case 0xaf: // Hold next key
			Hold_Next_Key = 1; 
			EEprom_Addr++;      // advance address	
                        Serial.print("Hold");
			break ; 
			
			
		case 0xa5:  // unused codes
		case 0xa6:
		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:		
			EEprom_Addr++;      // advance address
			break; 
		default:  // Send the character
			digiKeyboard_sendKeyPress(c);



			if (Hold_Next_Key)                                     // If next key should remain hold
			Hold_Next_Key = 0;                                 // then do nothing
			else {                                                 //
				digiKeyboard_sendKeyPress(0);                      // otherwise, send a release of
				digiKeyboard_delay(Default_Delay);                   // all key presses
			}    

			EEprom_Addr++;                                         // advance to the next character

		}
	}
	for (;;);  // do nothing after playing the script
}



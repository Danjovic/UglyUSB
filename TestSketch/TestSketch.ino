// 
// Ugly Token player Test Sketch 
// It runs on a standard Arduino and is used for debugging/development of the Ugly Token Player
//
#include <avr/pgmspace.h>

#define TRUE 1
#define FALSE 0
#define INIT_DEFAULT_DELAY 5
#define FLASH_SIZE 70

const PROGMEM uint8_t eeprom  [70] = {0xDE, 0XB8, 0X0B, 0x00, 0xA5, 0x00, 0xAF, 0xe3, 0x15, 0xAE, 0x00, 0xDE, 0XF4, 0X01, 0x00, 0X11, 0X12, 0X17, 0X08, 0X13, 0X04, 0X07, 0x00, 0xDE, 0XF4, 0X01, 0x00, 0x28, 0xAE, 0x00, 0xDE, 0XEE, 0X02, 0x00, 0xAF, 0xE1, 0X0B, 0X08, 0X0F, 0X0F, 0X12, 0X2C, 0xAF, 0xE1, 0X1A, 0X12, 0X15, 0X0F, 0X07, 0xAF, 0xE1, 0X1E, 0xAF, 0xE1, 0X1E, 0xAF, 0xE1, 0X1E, 0xA7, 0X03, 0X00, 0x00, 0xA6, 0X0A, 0X00, 0x00, 0x28, 0xAE, 0x00, 0xFF};


uint16_t EEprom_Addr = 0; // EEprom Address
uint16_t Last_EEprom_Addr = 0; // EEprom Address
uint16_t Last_Block_EEprom_Addr = 0; // EEprom Address
uint16_t Replay_Counter = 0; 
uint16_t Replay_Block_Counter = 0; 
uint16_t Default_Delay = 0;
uint16_t Time_to_Delay = 0;
uint8_t Hold_Next_Key = 0; // Hold Key flag
uint8_t Replaying = 0;
uint8_t Replaying_Block = 0;
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

		case 0xa5:    // start of block
			EEprom_Addr++;      // advance address
			Last_Block_EEprom_Addr = EEprom_Addr; 
                        Serial.print("Start of block ");
                        Serial.print(EEprom_Addr);
			break; 			
			
		case 0xa6: // Replay last block
			if (Replaying_Block) {
				Replay_Block_Counter--;
				if (Replay_Block_Counter ==0) {  // replay is over!
					EEprom_Addr += 3 ; // skip to the next command
					Replaying_Block = FALSE;
				} else
				        EEprom_Addr = Last_Block_EEprom_Addr;
                                Serial.print("Replaying block: ");
                                Serial.println(Replay_Block_Counter);
			} 
			else {
				Replaying_Block = TRUE;
				Replay_Block_Counter = Ext_EEpromRead16 (EEprom_Addr +1);
				if (Replay_Block_Counter==0)                              // check range
				    Replay_Block_Counter = 1;            //
				EEprom_Addr = Last_Block_EEprom_Addr;    // rewind address      
                                Serial.print("Replaying block Start: ");
                                Serial.print(EEprom_Addr);
                                Serial.println(Replay_Block_Counter);
			}			
			break;
			
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
                                Serial.print("\nReplay Start: ");
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
			
			
		case 0xa8: // unused codes
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



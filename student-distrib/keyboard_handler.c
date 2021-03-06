 #include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "keyboard_handler.h"
#include "terminal_driver.h"

#define KEYBOARD_ENTRY 0x21
#define  CAPS_LOCK_PRESSED    0x3A  /* Scancodes for actions */
#define  CAPS_LOCK_RELEASED   0xBA
#define  RIGHT_SHIFT_PRESSED  0x36
#define  RIGHT_SHIFT_RELEASED 0xB6
#define  LEFT_SHIFT_PRESSED   0x2A
#define  LEFT_SHIFT_RELEASED  0xAA
#define  CTRL_PRESSED         0x1D
#define CTRL_RELEASED         0x9D
#define ENTER_PRESSED         0x1C
#define CARRIAGE_RETURN       0x0A
#define  ALT_PRESSED          0x38
#define  ALT_RELEASED         0xB8
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D



#define TAB_OPCODE            0x0F

#define OPCODE_L              0x26
#define ESCAPE_OPCODE         0x01

#define  LOWER_CASE           0
#define  UPPER_CASE           1


#define BEGINNING_OF_RELEASE 0x81
#define END_OF_RELEASE 0xD8
#define BEGINNING_OF_PRINTABLE 0x01
#define END_OF_PRINTABLE 0x39
#define DELETE_OPCODE 0x0E
#define DELETE_ASCII  0x08
#define KEYBOARD_PORT 0x60

#define LOWER_BOUNDS_SYMBOLS 0x01
#define UPPER_BOUNDS_SYMBOLS 0x0D
#define LEFT_BRACKET         0x1A
#define RIGHT_BRACKET        0x1B
#define LOWER_BOUND_SYMBOLS2 0x27
#define UPPER_BOUND_SYMBOLS2 0x2B
#define LOWER_BOUND_SYMBOLS3 0x33
#define UPPER_BOUND_SYMBOLS3 0x35



//uint8_t keyboard_mapping[2][57];  /*  Keyboard array that converts the scancode to ascii value */
int caps_lock_flag = 0;
int caps_lock_counter = 0;
int right_shift_flag = 0;
int left_shift_flag = 0;
int control_flag = 0;
int alt_flag = 0;



/* This function should be called when the key is pressed and keyboard interrupt is sent to PIC. 
   Prints out the character from port 0x60 */
void keyboard_interrupt_handler(){
unsigned long flags;

cli();

/*used to check if input opcode is letter or number*/
int is_letter(unsigned int c);

sti();

cli_and_save(flags);
    unsigned int c = inb(KEYBOARD_PORT);
	//printf("c: %x\n",c);
	
       

	if(c==ENTER_PRESSED){
	      add_to_buffer(CARRIAGE_RETURN);
	}
	else if(c==TAB_OPCODE){
	  /*do nothing for tab just capture value*/
	}
	else if(c==ESCAPE_OPCODE){
          /*do nothing for escape just capture value*/
	}
	
	else if(c == ALT_PRESSED){
		alt_flag = 1;
	}
	
	else if(c == ALT_RELEASED){
		alt_flag = 0;
	}

	else if( c == CAPS_LOCK_PRESSED){
	  if( caps_lock_flag == 1)
	  {
             /* caps lock still pressing, do nothing */
	  
	  }
			
       	  else if(caps_lock_counter == 0){
            /* first time pressing caps_lock */
	    caps_lock_flag = 1;
            caps_lock_counter = 1;
	  }       
	  else{
            /* second time pressing caps lock */
	    caps_lock_counter = 0;
	    caps_lock_flag = 1;
          }
	}

	else if(c == CAPS_LOCK_RELEASED){
	    /*caps lock released reset flag*/
	    caps_lock_flag = 0;
	}

        /*right shift pressed change flag*/
	else if( c == RIGHT_SHIFT_PRESSED)
		right_shift_flag = 1;

        /*when released reset flag to 0*/
	else if( c == RIGHT_SHIFT_RELEASED)
		right_shift_flag = 0;

        /*left shift pressed change flag*/
	else if( c == LEFT_SHIFT_PRESSED)
		left_shift_flag = 1;

        /*when released reset flag to 0*/
	else if( c == LEFT_SHIFT_RELEASED)
		left_shift_flag = 0;

        
	else if(c==CTRL_PRESSED){
              control_flag = 1;
	}
	else if(c==CTRL_RELEASED){
              control_flag = 0;
	}
        else if(c>= BEGINNING_OF_RELEASE && c<=END_OF_RELEASE){
            /*key release trigger do nothing*/
	}

	else if(c>=BEGINNING_OF_PRINTABLE && c <=END_OF_PRINTABLE && control_flag!=1 && alt_flag!=1){
            /*printable character pressed---check case status and add to buf*/
	    
	    if(caps_lock_counter==0){
		if(right_shift_flag | left_shift_flag){
		    add_to_buffer(keyboard_mapping_capital[c]);
		}

		else{
		    add_to_buffer(keyboard_mapping_lowercase[c]);
		    }
            }
	    /*caps lock is pressed*/
	    else{
                 if(!(is_letter(c))){
                    if(right_shift_flag | left_shift_flag){
						add_to_buffer(keyboard_mapping_capital[c]);
					}
		   
					else{
						add_to_buffer(keyboard_mapping_lowercase[c]);
					}
				}    
                else{
					if(right_shift_flag | left_shift_flag){
                        add_to_buffer(keyboard_mapping_lowercase[c]);
					}

		 
					else{
					add_to_buffer(keyboard_mapping_capital[c]);
					}
				}
	    }
	}
	/* triggers clear when control and "L" pressed */
	else if(c == OPCODE_L && control_flag==1){
		clear();
		clear_buffer();
	}
	/* triggers terminal switch if F1, F2, or F3 key is pressed when the alt key is pressed */
	else if(alt_flag==1){
		if(c == F1){
			/* switch to terminal 0 */
			term_visible_switch(0);
		}
		else if(c == F2){
			/* switch to terminal 1 */
			term_visible_switch(1);
		}
		else if(c == F3){
			/* switch to terminal 2 */
			term_visible_switch(2);
		}
	}
	else{
	/*character not recognized do nothing*/
	}
    
    send_eoi(1);  //ends interrupt on IRQ1 for keyboard

restore_flags(flags);  //end of interrupt end of critical section
}



/*  Referenced from OSDev PS2 Keyboard Scan Code Set 1. More keys to be added in the future. 
    Initializes the array that maps the scancode to ascii value */

void keyboard_initialization(){
//printf("made it to initializaiton of keyboard\n");


	idt[KEYBOARD_ENTRY].seg_selector = KERNEL_CS;
	idt[KEYBOARD_ENTRY].reserved4   = 0x0; 
	idt[KEYBOARD_ENTRY].reserved3   = 0x0;  /* referenced from x86 ISA Manual */
	idt[KEYBOARD_ENTRY].reserved2   = 0x1;
	idt[KEYBOARD_ENTRY].reserved1   = 0x1;
	idt[KEYBOARD_ENTRY].size        = 0x1;  /* 32 bit */
	idt[KEYBOARD_ENTRY].reserved0   = 0x0;
	idt[KEYBOARD_ENTRY].dpl         = 0x0;  /* kernel priviledged mode */
	idt[KEYBOARD_ENTRY].present     = 0x1;
        SET_IDT_ENTRY(idt[KEYBOARD_ENTRY], &keyboard_interrupt_handler);
        enable_irq(1); // enables keyboard interrupt on IRQ
	
       


}


/*is_letter
 *DESCRIPTION: takes in letter tells if letter character is pressed
 *or number
 *INPUT: c---opcode from keyboard
 *OUTPUT: 0 if symbol other then letter is input
 *        1 if symbol is a letter
 *SIDEFFECTS: NONE
 */
int is_letter(unsigned int c){
    //printf("Character: %c",c);
    if(c>=LOWER_BOUNDS_SYMBOLS && c<=UPPER_BOUNDS_SYMBOLS) return 0;
    if(c==LEFT_BRACKET || c==RIGHT_BRACKET) return 0;
    if(c>=LOWER_BOUND_SYMBOLS2&& c<=UPPER_BOUND_SYMBOLS2)return 0;
    if(c>=LOWER_BOUND_SYMBOLS3 && c<=UPPER_BOUND_SYMBOLS3)return 0;
    return 1;

};

/*Referenced from OSDev PS2 Keyboard Scan Code Set 1. More keys to be added in the future.
   Initializes the array that maps the scancode to ascii value */

/*lowercase map of characters from press to ascii*/


unsigned char keyboard_mapping_lowercase[MAPPING_SIZE] = {
        0x00,0x1B,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x2D,
	0x3D,0x08,0x09,0x71,0x77,0x65,0x72,0x74,0x79,0x75,0x69,0x6F,0x70,
	0x5B,0x5D,0x13,0x00,0x61,0x73,0x64,0x66,0x67,0x68,0x6A,0x6B,0x6C,
	0x3B,0x27,0x60,0x00,0x5C,0x7A,0x78,0x63,0x76,0x62,0x6E,0x6D,0x2C,
	0x2E,0x2F,0x00,0x2A,0x00,0x20,



};

/*uppercase map of characters from press to ascii*/

unsigned char keyboard_mapping_capital[MAPPING_SIZE] = {
        0x00,0x1B,0x21,0x40,0x23,0x24,0x25,0x5E,0x26,0x2A,0x28,0x29,0x5F,
	0x2B,0x08,0x09,0x51,0x57,0x45,0x52,0x54,0x59,0x55,0x49,0x4F,0x50,
	0x7B,0x7D,0x13,0x00,0x41,0x53,0x44,0x46,0x47,0x48,0x4A,0x4B,0x4C,
	0x3A,0x22,0x7E,0x00,0x7C,0x5A,0x58,0x43,0x56,0x42,0x4E,0x4D,0x3C,
	0x3E,0x3F,0x00,0x2A,0x00,0x20,0x00

};


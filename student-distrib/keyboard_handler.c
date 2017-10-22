#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "keyboard_handler.h"

#define KEYBOARD_ENTRY 0x21
#define  CAPS_LOCK_PRESSED    0x3A  /* Scancodes for actions */
#define  CAPS_LOCK_RELEASED   0xBA
#define  RIGHT_SHIFT_PRESSED  0x36
#define  RIGHT_SHIFT_RELEASED 0xB6
#define  LEFT_SHIFT_PRESSED   0x2A
#define  LEFT_SHIFT_RELEASED  0xAA
#define  LOWER_CASE           0
#define  UPPER_CASE           1


uint8_t keyboard_mapping[2][57];  /*  Keyboard array that converts the scancode to ascii value */
int caps_lock_flag = 0;
int caps_lock_counter = 0;
int right_shift_flag = 0;
int left_shift_flag = 0;

/* This function should be called when the key is pressed and keyboard interrupt is sent to PIC. 
   Prints out the character from port 0x60 */
void keyboard_interrupt_handler(){
unsigned long flags;
//cli_and_save(flags);
cli();
    unsigned int c = inb(0x60);
	
sti();
        
cli_and_save(flags);
        if((c >= 0x81) && (c <= 0xD8)){
            
	}
	else if( c == CAPS_LOCK_PRESSED) {

		if( caps_lock_flag == 1)/* caps lock still pressing, do nothing */
		{
		}
			

		else if(caps_lock_counter == 0) /* first time pressing caps_lock */
		{
			caps_lock_flag = 1;
			caps_lock_counter = 1;
		}       
		else{                           /* second time pressing caps lock */
			caps_lock_counter = 0;
			caps_lock_flag = 1;
		}
	}	

	else if( c == CAPS_LOCK_RELEASED) 
	{
		caps_lock_flag = 0;
	}

	else if( c == RIGHT_SHIFT_PRESSED)
		right_shift_flag = 1;
	else if( c == RIGHT_SHIFT_RELEASED)
		right_shift_flag = 0;
	else if( c == LEFT_SHIFT_PRESSED)
		left_shift_flag = 1;
	else if( c == LEFT_SHIFT_RELEASED)
		left_shift_flag = 0;
	else if(c>=0x01 && c <=0x39){

		if(caps_lock_counter | right_shift_flag | left_shift_flag)
			printf("%c", keyboard_mapping[UPPER_CASE][c-1]);
		else
			printf("%c", keyboard_mapping[LOWER_CASE][c-1]);

	}
	else{
	}
//	restore_flags(flags);
//sti();
    send_eoi(1);  //ends interrupt on IRQ1 for keyboard

restore_flags(flags);
}

//keyboard_mapping [] = 


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
        lidt(idt_desc_ptr);

	keyboard_mapping[LOWER_CASE][0] = 0x1B; /* Escape pressed */
	keyboard_mapping[LOWER_CASE][1] = 0x31; /* 1 pressed */
	keyboard_mapping[LOWER_CASE][2] = 0x32; /* 2 pressed */
	keyboard_mapping[LOWER_CASE][3] = 0x33; /* 3 pressed */
	keyboard_mapping[LOWER_CASE][4] = 0x34; /* 4 pressed */
	keyboard_mapping[LOWER_CASE][5] = 0x35; /* 5pressed */
	keyboard_mapping[LOWER_CASE][6] = 0x36; /* 6 pressed */
	keyboard_mapping[LOWER_CASE][7] = 0x37; /* 7 pressed */
	keyboard_mapping[LOWER_CASE][8] = 0x38; /* 8 pressed */
	keyboard_mapping[LOWER_CASE][9] = 0x39;  /* 9 pressed */
	keyboard_mapping[LOWER_CASE][10] = 0x30; /* 0 pressed */
	keyboard_mapping[LOWER_CASE][11] = 0x2D; /* - pressed */
	keyboard_mapping[LOWER_CASE][12] = 0x3D; /* = pressed */
	//keyboard_mapping[13] =       /* backspace pressed */
	//keyboard_mapping[14] =       /* tab pressed */
	keyboard_mapping[LOWER_CASE][15] = 0x71; /* q pressed */
	keyboard_mapping[LOWER_CASE][16] = 0x77; /* w pressed */
	keyboard_mapping[LOWER_CASE][17] = 0x65; /* e pressed */
	keyboard_mapping[LOWER_CASE][18] = 0x72; /* r ressed */
	keyboard_mapping[LOWER_CASE][19] = 0x74; /* t pressed */
	keyboard_mapping[LOWER_CASE][20] = 0x79; /* y pressed */
	keyboard_mapping[LOWER_CASE][21] = 0x75; /* u pressed */
	keyboard_mapping[LOWER_CASE][22] = 0x69; /* i pressed */
	keyboard_mapping[LOWER_CASE][23] = 0x6F; /* o pressed */
	keyboard_mapping[LOWER_CASE][24] = 0x70; /* p pressed */
	keyboard_mapping[LOWER_CASE][25] = 0x5B; /* [ pressed */
	keyboard_mapping[LOWER_CASE][26] = 0x5D; /* ] pressed */
	//keyboard_mapping[27] =       /* Enter pressed */
	//keyboard_mapping[28] =       /* left control pressed */
	keyboard_mapping[LOWER_CASE][29] = 0x61; /* a pressed */
	keyboard_mapping[LOWER_CASE][30] = 0x73;  /* s pressed */
	keyboard_mapping[LOWER_CASE][31] = 0x64;  /* d pressed */
	keyboard_mapping[LOWER_CASE][32] = 0x66;  /* f pressed */
	keyboard_mapping[LOWER_CASE][33] = 0x67;  /* g pressed */
	keyboard_mapping[LOWER_CASE][34] = 0x68;  /* h pressed */
	keyboard_mapping[LOWER_CASE][35] = 0x6A;  /* j pressed */
	keyboard_mapping[LOWER_CASE][36] = 0x6B;  /* k pressed */
	keyboard_mapping[LOWER_CASE][37] = 0x6C;  /* l pressed */
	keyboard_mapping[LOWER_CASE][38] = 0x3B;  /* ; pressed */
	keyboard_mapping[LOWER_CASE][39] = 0x27;  /* ' pressed */
	keyboard_mapping[LOWER_CASE][40] = 0x60;  /*  pressed */
	//keyboard_mapping[41] =        /* left shift pressed */
	keyboard_mapping[LOWER_CASE][42] = 0x5C;  /* \ pressed */
	keyboard_mapping[LOWER_CASE][43] = 0x7A;  /* z pressed */
	keyboard_mapping[LOWER_CASE][44] = 0x78;  /* x pressed */
	keyboard_mapping[LOWER_CASE][45] = 0x63;  /* c pressed */
	keyboard_mapping[LOWER_CASE][46] = 0x76;  /* v pressed */
	keyboard_mapping[LOWER_CASE][47] = 0x62;  /* b pressed */
	keyboard_mapping[LOWER_CASE][48] = 0x6E;  /* n pressed */
	keyboard_mapping[LOWER_CASE][49] = 0x6D;  /* m pressed */
	keyboard_mapping[LOWER_CASE][50] = 0x2C;  /* , pressed */
	keyboard_mapping[LOWER_CASE][51] = 0x2E;  /* .pressed */
	keyboard_mapping[LOWER_CASE][52] = 0x2F;  /* / pressed */
	//keyboard_mapping[53] =        /* right shift pressed */
	keyboard_mapping[LOWER_CASE][54] = 0x2A;  /* * pressed */
	//keyboard_mapping[55] =        /* left alt pressed */
	keyboard_mapping[LOWER_CASE][56] = 0x20;  /* space alt pressed */
	//keyboard_mapping[57] =        /* Caps Lock pressed */
	//keyboard_mapping[58] =        /* F1 pressed */

	keyboard_mapping[UPPER_CASE][0] = 0x1B; /* Escape pressed */
	keyboard_mapping[UPPER_CASE][1] = 0x21; /* ! pressed */
	keyboard_mapping[UPPER_CASE][2] = 0x40; /* @ pressed */
	keyboard_mapping[UPPER_CASE][3] = 0x23; /* # pressed */
	keyboard_mapping[UPPER_CASE][4] = 0x24; /* $ pressed */
	keyboard_mapping[UPPER_CASE][5] = 0x25; /* % pressed */
	keyboard_mapping[UPPER_CASE][6] = 0x5E; /* ^ pressed */
	keyboard_mapping[UPPER_CASE][7] = 0x26; /* & pressed */
	keyboard_mapping[UPPER_CASE][8] = 0x2A; /* * pressed */
	keyboard_mapping[UPPER_CASE][9] = 0x28;  /* ( pressed */
	keyboard_mapping[UPPER_CASE][10] = 0x29; /* ) pressed */
	keyboard_mapping[UPPER_CASE][11] = 0x5F; /* _ pressed */
	keyboard_mapping[UPPER_CASE][12] = 0x2B; /* + pressed */
	//keyboard_mapping[13] =       /* backspace pressed */
	//keyboard_mapping[14] =       /* tab pressed */
	keyboard_mapping[UPPER_CASE][15] = 0x51; /* Q pressed */
	keyboard_mapping[UPPER_CASE][16] = 0x57; /* W pressed */
	keyboard_mapping[UPPER_CASE][17] = 0x45; /* E pressed */
	keyboard_mapping[UPPER_CASE][18] = 0x52; /* R ressed */
	keyboard_mapping[UPPER_CASE][19] = 0x54; /* T pressed */
	keyboard_mapping[UPPER_CASE][20] = 0x59; /* Y pressed */
	keyboard_mapping[UPPER_CASE][21] = 0x55; /* U pressed */
	keyboard_mapping[UPPER_CASE][22] = 0x49; /* I pressed */
	keyboard_mapping[UPPER_CASE][23] = 0x4F; /* O pressed */
	keyboard_mapping[UPPER_CASE][24] = 0x50; /* P pressed */
	keyboard_mapping[UPPER_CASE][25] = 0x7B; /* { pressed */
	keyboard_mapping[UPPER_CASE][26] = 0x7D; /* } pressed */
	//keyboard_mapping[27] =       /* Enter pressed */
	//keyboard_mapping[28] =       /* left control pressed */
	keyboard_mapping[UPPER_CASE][29] = 0x41; /* A pressed */
	keyboard_mapping[UPPER_CASE][30] = 0x53;  /* S pressed */
	keyboard_mapping[UPPER_CASE][31] = 0x44;  /* D pressed */
	keyboard_mapping[UPPER_CASE][32] = 0x46;  /* F pressed */
	keyboard_mapping[UPPER_CASE][33] = 0x47;  /* G pressed */
	keyboard_mapping[UPPER_CASE][34] = 0x48;  /* H pressed */
	keyboard_mapping[UPPER_CASE][35] = 0x4A;  /* J pressed */
	keyboard_mapping[UPPER_CASE][36] = 0x4B;  /* K pressed */
	keyboard_mapping[UPPER_CASE][37] = 0x4C;  /* L pressed */
	keyboard_mapping[UPPER_CASE][38] = 0x3A;  /* : pressed */
	keyboard_mapping[UPPER_CASE][39] = 0x22;  /* " pressed */
	keyboard_mapping[UPPER_CASE][40] = 0x60;  /*  pressed */
	//keyboard_mapping[41] =        /* left shift pressed */
	keyboard_mapping[UPPER_CASE][42] = 0x7C;  /* | pressed */
	keyboard_mapping[UPPER_CASE][43] = 0x5A;  /* Z pressed */
	keyboard_mapping[UPPER_CASE][44] = 0x58;  /* X pressed */
	keyboard_mapping[UPPER_CASE][45] = 0x43;  /* C pressed */
	keyboard_mapping[UPPER_CASE][46] = 0x56;  /* V pressed */
	keyboard_mapping[UPPER_CASE][47] = 0x42;  /* B pressed */
	keyboard_mapping[UPPER_CASE][48] = 0x4E;  /* N pressed */
	keyboard_mapping[UPPER_CASE][49] = 0x4D;  /* M pressed */
	keyboard_mapping[UPPER_CASE][50] = 0x3C;  /* < pressed */
	keyboard_mapping[UPPER_CASE][51] = 0x3E;  /* >pressed */
	keyboard_mapping[UPPER_CASE][52] = 0x3F;  /* ? pressed */
	//keyboard_mapping[53] =        /* right shift pressed */
	keyboard_mapping[UPPER_CASE][54] = 0x2A;  /* * pressed */
	//keyboard_mapping[55] =        /* left alt pressed */
	keyboard_mapping[UPPER_CASE][56] = 0x20;  /* space alt pressed */
	//keyboard_mapping[57] =        /* Caps Lock pressed */
	//keyboard_mapping[58] =        /* F1 pressed */
//        printf("just before irq enable\n");
        enable_irq(1); // enables keyboard interrupt on IRQ1
	printf("done keyboard initilizing");
        


}

















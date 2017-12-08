#include "terminal_driver.h"
#include "lib.h"
#include "keyboard_handler.h"
#include "scheduler.h"

#define BACKSPACE         0x08
#define EXECUTABLE_ZERO   0x7F
#define EXECUTABLE_ONE    0x45
#define EXECUTABLE_TWO    0x4C
#define EXECUTABLE_THREE  0x46

#define MAX_BUF_LENGTH 128
#define MAX_NUM_COMMANDS 640

#define EXECUTABLE_BYTES 16

//TEMPORARY
#define CURRENT_VISIBLE 0
//TEMPORARY

/*keyboard buffer and length of buffer*/
//char keyboard_buf[MAX_BUF_LENGTH];
//int buf_position=0;
int command_position = 0;

/*command buffer holds command until they're read*/
char command_buf[MAX_NUM_COMMANDS];

/*flag that lets read know a command is ready*/
//int command_ready_flag = -1;

/*terminal_open
 *Description: opens driver returns success
 *Input: NONE
 *Output: NONE
 *Side Effects: NONE
 */
int32_t terminal_open(){
    return 0;
}

/*terminal_close
 *Description: closes driver returns failure because never close
 *Input: None
 *Output: None
 *Side Effects: None
 */
int32_t terminal_close(){
    /*returns -1 because the terminal should never be closed during program execution*/
    return -1;
}

/*terminal_read
 *Description: Gives last input command to caller
 *Input: 
         fd--the file descriptor entry of the file
         buf--pointer you want input coppied to
         nbytes--number of bytes to copy into buf
 *Output: None
 *Side effect: coppies command buffer into input buffer
 */
int32_t terminal_read(int32_t fd, char *buf, int32_t nbytes){
	
     if(nbytes<0) return -1;
     if(buf==NULL) return -1;
     
    /*waits for flag to be set before returning*/
    while(terminal[terminal_num].command_ready_flag==-1){}
    cli();
    int i=0;
    
    /*coppies over bytes until nbytes reached or return carrige*/
    while(i < nbytes){
      char c = command_buf[i];
      if(c == '\n'){
         buf[i] = c;
	 i++;
	 break;
      }
      buf[i] = c;
      i++;
    }
    
	
	int j;
    for(j=0;j<i;j++){
		/* printf("command_position: %d\n",command_position);
		printf("j: %d\n",j);
		printf("j+i: %d\n",j+i); */
		command_buf[j] = command_buf[j+i];
	    command_position--;
		
    }
	terminal[terminal_num].command_ready_flag=-1;
	
	
    
    return i;
}

/*terminal_write
 *Description: Writes input buf to the screen
 *Input:fd--the file descriptor entry of the file
 *      buf--array of chars to be printed to string
 *Output: nbytes--number of bytes to write to the string
 *Side effects: prints input buf to the screen
 */
int32_t terminal_write(int32_t fd, const char* buf, int32_t nbytes){
	cli();
    int i = 0;

    /*error cases*/
    if(nbytes < 0)return -1;
    if(buf==NULL)return -1;

    /*checks if input is an executable file*/
    /* if(nbytes >= EXECUTABLE_BYTES){
       if(buf[0]==EXECUTABLE_ZERO && buf[1]==EXECUTABLE_ONE &&
            buf[2]==EXECUTABLE_TWO && buf[3]==EXECUTABLE_THREE){
            for(i=0;i<EXECUTABLE_BYTES;i++){
                printf("%x",buf[i]);
	    }
            return EXECUTABLE_BYTES;
	  }
      
    } */
    /*prints none executable file*/
    while(i<nbytes){
       printf("%c",buf[i]);
       i++;
    }
    sti();
    return i;
}

/*add_to_buffer
 *Description: called by keyboard interrupt to add ascii chars to buffer
 *Input: c--character to add to the buffer
 *Output: None
 *Side effect: adds characters to buffer and sets flag if return detected
 */
void add_to_buffer(char c){
	//cli();
   /*if return detected flag is set and buffer cleared*/
   if(c=='\n'){
     terminal[current_visible].keyboard_buffer[terminal[current_visible].buf_position] = '\n';
     terminal[current_visible].buf_position++;
	 putc_keyboard(c);
     //print_keyboard_buffer(terminal[current_visible].keyboard_buffer,terminal[current_visible].buf_position);
     copy_command_buffer();
     terminal[current_visible].command_ready_flag=1;
     clear_buffer();
     return;
   }

   else if(c==BACKSPACE){
     if(terminal[current_visible].buf_position!=0){
		 
       terminal[current_visible].buf_position -=1;
       terminal[current_visible].keyboard_buffer[terminal[current_visible].buf_position]=NULL;
	   putc_keyboard(c);
     }
   }
   else if(terminal[current_visible].buf_position==127){
     /*do nothing buffer full*/
   }
   else{
     terminal[current_visible].keyboard_buffer[terminal[current_visible].buf_position] = c;
     terminal[current_visible].buf_position++;
	 putc_keyboard(c);
     
   }
   /* print_keyboard_buffer(terminal[current_visible].keyboard_buffer,terminal[current_visible].buf_position); */
   
   //sti();
}

/*clear_buffer
 *Description: clears the buffer
 *Input: None
 *Output: None
 *Side effects: clears the keyboard buffer
 */
void clear_buffer(){
   int i;
   
   for(i=0;i<terminal[current_visible].buf_position;i++){
      terminal[current_visible].keyboard_buffer[i] = NULL;
   }
   terminal[current_visible].buf_position = 0;
   //print_keyboard_buffer(terminal[terminal_num].keyboard_buffer,terminal[terminal_num].buf_position);

}
/*copy_command_buffer
 *Description: moves data from one buffer to the other
 *Input: none
 *Output: none
 *Side effect: coppies keyboard buffer contents into command
 */
void copy_command_buffer(){
     int i;
     for(i=0;i<terminal[current_visible].buf_position;i++){
         command_buf[command_position] = terminal[current_visible].keyboard_buffer[i];
		 command_position++;
     }
}

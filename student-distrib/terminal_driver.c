#include "terminal_driver.h"
#include "lib.h"
#include "keyboard_handler.h"

#define BACKSPACE         0x08
#define EXECUTABLE_ZERO   0x7F
#define EXECUTABLE_ONE    0x45
#define EXECUTABLE_TWO    0x4C
#define EXECUTABLE_THREE  0x46

#define MAX_BUF_LENGTH 128
#define MAX_NUM_COMMANDS 5

#define EXECUTABLE_BYTES 16

/*keyboard buffer and length of buffer*/
char keyboard_buf[MAX_BUF_LENGTH];
int buf_position=0;

/*command buffer holds command until they're read*/
char command_buf[MAX_NUM_COMMANDS][MAX_BUF_LENGTH];

/*flag that lets read know a command is ready*/
int command_ready_flag = -1;

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
    return -1;
}

/*terminal_read
 *Description: Gives last input command to caller
 *Input: buf--pointer you want input coppied to
         nbytes--number of bytes to copy into buf
 *Output: None
 *Side effect: coppies command buffer into input buffer
 */
int32_t terminal_read(char *buf, int32_t nbytes){
     if(nbytes<0) return -1;
     if(buf==NULL) return -1;
     
    /*waits for flag to be set before returning*/
    while(command_ready_flag==-1){}
    cli();
    int i=0;
    
    /*coppies over bytes until nbytes reached or return carrige*/
    while(i < nbytes){
      char c = command_buf[command_ready_flag][i];
      if(c == '\n'){
         buf[i] = c;
	 i++;
	 break;
      }
      buf[i] = c;
      i++;
    }
    command_ready_flag--;
    sti();
    return i;
}

/*terminal_write
 *Description: Writes input buf to the screen
 *Input: buf--array of chars to be printed to string
 *Output: nbytes--number of bytes to write to the string
 *Side effects: prints input buf to the screen
 */
int32_t terminal_write(const char* buf, int32_t nbytes){
    int i = 0;

    /*error cases*/
    if(nbytes < 0)return -1;
    if(buf==NULL)return -1;

    /*checks if input is an executable file*/
    if(nbytes >= EXECUTABLE_BYTES){
       if(buf[0]==EXECUTABLE_ZERO && buf[1]==EXECUTABLE_ONE &&
            buf[2]==EXECUTABLE_TWO && buf[3]==EXECUTABLE_THREE){
            for(i=0;i<EXECUTABLE_BYTES;i++){
                printf("%x",buf[i]);
	    }
            return EXECUTABLE_BYTES;
	  }
      
    }
    /*prints none executable file*/
    while(i<nbytes){
       printf("%c",buf[i]);
       i++;
    }

    return i;
}

/*add_to_buffer
 *Description: called by keyboard interrupt to add ascii chars to buffer
 *Input: c--character to add to the buffer
 *Output: None
 *Side effect: adds characters to buffer and sets flag if return detected
 */
void add_to_buffer(char c){
   /*if return detected flag is set and buffer cleared*/
   if(c=='\n'){
     keyboard_buf[buf_position] = '\n';
     buf_position++;
     print_keyboard_buffer(keyboard_buf,buf_position);
     copy_command_buffer();
     command_ready_flag++;
     clear_buffer();
     return;
   }

   else if(c==BACKSPACE){
     if(buf_position!=0){
       buf_position -=1;
       keyboard_buf[buf_position]=NULL;
     }
   }
   else if(buf_position==127){
     /*do nothing buffer full*/
   }
   else{
     keyboard_buf[buf_position] = c;
     buf_position++;
     
   }
   print_keyboard_buffer(keyboard_buf,buf_position);
}

/*clear_buffer
 *Description: clears the buffer
 *Input: None
 *Output: None
 *Side effects: clears the keyboard buffer
 */
void clear_buffer(){
   int i;
   for(i=0;i<buf_position;i++){
      keyboard_buf[i] = NULL;
   }
   buf_position = 0;
   print_keyboard_buffer(keyboard_buf,buf_position);

}
/*copy_command_buffer
 *Description: moves data from one buffer to the other
 *Input: none
 *Output: none
 *Side effect: coppies keyboard buffer contents into command
 */
void copy_command_buffer(){
     int i;
     for(i=0;i<buf_position;i++){
         command_buf[command_ready_flag+1][i] = keyboard_buf[i];
     }
}

#include "terminal_driver.h"
#include "lib.h"
#include "keyboard_handler.h"

#define BACKSPACE         0x08
#define EXECUTABLE_ZERO   0x7F
#define EXECUTABLE_ONE    0x45
#define EXECUTABLE_TWO    0x4C
#define EXECUTABLE_THREE  0x46

char keyboard_buf[128];
int buf_position=0;

char command_buf[5][128];

int command_ready_flag = -1;


int32_t terminal_open(){
    return 0;
}

int32_t terminal_close(){
    return -1;
}

int32_t terminal_read(char *buf, int32_t nbytes){
    while(command_ready_flag==-1){}
    cli();
    int i=0;
    
    
    while(i < nbytes){
      char c = command_buf[command_ready_flag][i];
      if(c == '\n'){
         buf[i] = c;
	 i++;
	 break;
      }
      buf[i] = c;//command_buf[command_ready_flag][i];
      i++;
    }
    command_ready_flag--;
    sti();
    return i;
}


int32_t terminal_write(const char* buf, int32_t nbytes){
    int i = 0;
    if(nbytes >= 16){
       if(buf[0]==EXECUTABLE_ZERO && buf[1]==EXECUTABLE_ONE &&
            buf[2]==EXECUTABLE_TWO && buf[3]==EXECUTABLE_THREE){
            for(i=0;i<16;i++){
                printf("%x",buf[i]);
	    }
            return 16;
	  }
      
    }
    while(i<nbytes){
       printf("%c",buf[i]);
       i++;
    }

    return i;
}

void add_to_buffer(char c){
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

void clear_buffer(){
   int i;
   for(i=0;i<buf_position;i++){
      keyboard_buf[i] = NULL;
   }
   buf_position = 0;
   print_keyboard_buffer(keyboard_buf,buf_position);

}
void copy_command_buffer(){
     int i;
     for(i=0;i<buf_position;i++){
         command_buf[command_ready_flag+1][i] = keyboard_buf[i];
     }
}

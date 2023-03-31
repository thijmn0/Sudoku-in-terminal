#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
// https://stackoverflow.com/a/826027



static struct termios orig_termios;

struct editorConfig{
	_Bool raw_mode;
	unsigned int cx;
	unsigned int cy;
};

static struct editorConfig E;


void disable_raw_mode(void){
	if(E.raw_mode){
		tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios);
		E.raw_mode=0;
	}
}

_Bool enable_raw_mode(int fd){
	if(E.raw_mode){
		return 0;
	}
	if(!isatty(STDIN_FILENO)){
		goto fatal;
	}
	atexit(disable_raw_mode);
	if(tcgetattr(fd,&orig_termios)==-1){
		goto fatal;
	}
	struct termios raw;
	raw = orig_termios;  //modify the original mode
    // input modes: no break, no CR to NL, no parity check, no strip char,
    //no start/stop output control. 
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // output modes - disable post processing 
    raw.c_oflag &= ~(OPOST);
    // control modes - set 8 bit chars 
    raw.c_cflag |= (CS8);
    // local modes - choing off, canonical off, no extended functions,
    // * no signal chars (^Z,^C) 
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // control chars - set return condition: min number of bytes and timer. 
    raw.c_cc[VMIN] = 0; // Return each byte, or zero for timeout. 
    raw.c_cc[VTIME] = 1; // 100 ms timeout (unit is tens of second). 
	tcsetattr(STDIN_FILENO, TCSAFLUSH,&raw);
	E.raw_mode=1;

fatal:
	errno=ENOTTY;
	return -1;
}

struct abuf{
	char* b;
	unsigned int len;
};

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b,ab->len+len);

    if (new == NULL) return;
    memcpy(new+ab->len,s,len);
    ab->b = new;
    ab->len += len;
}

void print_large_generated_sudoku(uint8_t* sudoku){
	const char num[]="▗▄  █ ▗█▖▗▄▖▗▄▌▐▄▖▗▄▖▗▄▌▗▄▌▄ ▄▜▃█  █▗▄▖▐▄▖▗▄▌▄▄▖▙▄▖▙▄▌▄▄▖  ▌  ▌▄▄▖▙▄▌▙▄▌▄▄▖▙▄▌▄▄▌";
	// ▗▄  ▗▄▖ ▗▄▖ ▄ ▄ ▗▄▖ ▄▄▖ ▄▄▖ ▄▄▖ ▄▄▖
	//  █  ▗▄▌ ▗▄▌ ▜▃█ ▐▄▖ ▙▄▖   ▌ ▙▄▌ ▙▄▌
	// ▗█▖ ▐▄▖ ▗▄▌   █ ▗▄▌ ▙▄▌   ▌ ▙▄▌ ▄▄▌
	unsigned int cx=0;
	unsigned int cy=0;
	char buffer[319];
	unsigned int lenbuffer;

    //lenbuffer=snprintf(buffer,sizeof(buffer),"┏━━━━━━━┯━━━━━━━┯━━━━━━━┳━━━━━━━┯━━━━━━━┯━━━━━━━┳━━━━━━━┯━━━━━━━┯━━━━━━━┓\r\n┃       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┃");
    
    for(uint8_t i=0;i<243;i++){
    	//TODO 	
    	if(*(sudoku+i)==0){
    		write(STDOUT_FILENO,"       ",7);
    	}
    	else{
    		char c[7]="       ";
    		c[3]=48+*(sudoku+i);
    		write(STDOUT_FILENO,c,7);
    	}
    	if(i%9==8){
    		write(STDOUT_FILENO,"┃\r\n┃",8);

    		if(i!=80){
    			if(i%27==26){
    				write(STDOUT_FILENO,"       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┣━━━━━━━┿━━━━━━━┿━━━━━━━╋━━━━━━━┿━━━━━━━┿━━━━━━━╋━━━━━━━┿━━━━━━━┿━━━━━━━┫\r\n┃       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┃",411);//97*2+3+219 
    			}
    			else{
					write(STDOUT_FILENO,"       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┣╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┠╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┠╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┃\r\n┃       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┃",411);
    			}
    		}
    		else{
    			write(STDOUT_FILENO,"       ╎       ╎       ┃       ╎       ╎       ┃       ╎       ╎       ┃\r\n┗━━━━━━━┷━━━━━━━┷━━━━━━━┻━━━━━━━┷━━━━━━━┷━━━━━━━┻━━━━━━━┷━━━━━━━┷━━━━━━━┛\r\n",313);
    		}
    	}
    	else if(i%3==2){
    		write(STDOUT_FILENO,"┃",3);
    	}
    	else{
    		write(STDOUT_FILENO,"╎",3);
    	}
    }
    //write buffer
}

void print_small_sudoku(uint8_t* sudoku){
    write(STDOUT_FILENO,"╭───────┬───────┬───────╮\r\n│ ",82);
    for(uint8_t i=0;i<81;i++){
    	if(*(sudoku+i)==0){
    		write(STDOUT_FILENO,"·",3);
    	}
    	else{
    		char c[2];
    		c[0]=48+*(sudoku+i);
    		c[1]='\0';
    		write(STDOUT_FILENO,c,2);
    	}
    	if(i%9==8){
    		write(STDOUT_FILENO," │\r\n",6);
    		if(i!=80){
    			if(i%27==26){
    				write(STDOUT_FILENO,"├───────┼───────┼───────┤\r\n│ ",82);
    			}
    			else{
    				write(STDOUT_FILENO,"│ ",4);
    			}
    		}
    		else{
    			write(STDOUT_FILENO,"╰───────┴───────┴───────╯\r\n",78);
    		}
    	}
    	else if(i%3==2){
    		write(STDOUT_FILENO," │ ",5);
    	}else{
    		write(STDOUT_FILENO," ",1);
    	}
    }
}

_Bool process_key(void){
	char c;
	while ((read(STDIN_FILENO, &c, 1)) == 0&&c!='q');
	if(c=='q'){
		return 1;
	}
	return 0;
}

int main(int argc, char **argv){
	//system("clear");
	E.raw_mode=0;
	enable_raw_mode(STDIN_FILENO);
	write(STDOUT_FILENO,"\x1b[0;0H",6);
	uint8_t grid[81]={0,0,0,0,0,0,0,1,2,0,0,0,0,3,5,0,0,0,0,0,0,6,0,0,0,7,0,7,0,0,0,0,0,3,0,0,0,0,0,4,0,0,8,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,8,0,0,0,0,0,4,0,0,5,0,0,0,0,6,0,0};
	print_small_sudoku(grid);
	// write(STDOUT_FILENO,"\033[?1003h",8);
	while(1){
		if(process_key()){
			break;
		}
	}
	printf("\033[?1003l");
	// system("clear");
	return 0;
}

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
// https://stackoverflow.com/a/826027
// Debug with valgrind if ever needed: gcc -g -O1 myEditor.c -o myEditor;	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -s ./myEditor


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
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH,&raw)<0) goto fatal;
	E.raw_mode=1;

fatal:
	errno=ENOTTY;
	return -1;
}

#define ABUF_INIT {NULL,0};

struct abuf{
	char* b;
	unsigned int len;
};

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b,ab->len+len);

    if (new == NULL) return;
    memcpy(new+ab->len,s,len);
    ab->len+=len;
    ab->b=new;
}

void abFree(struct abuf *ab) {
    free(ab->b);
}

void print_large_sudoku(const uint8_t sudoku[81]){
	///im lazy, ill improve it later
	const uint8_t num_len[]={7,5,9,9,9,9,9,9,9,7,9,5,9,9,9,9,9,9,9,5,5,9,9,9,9,9,9};
	const char* num[]={"▗▄ "," █ ","▗█▖","▗▄▖","▗▄▌","▐▄▖","▗▄▖","▗▄▌","▗▄▌","▄ ▄","▜▃█","  █","▗▄▖","▐▄▖","▗▄▌","▄▄▖","▙▄▖","▙▄▌","▄▄▖","  ▌","  ▌","▄▄▖","▙▄▌","▙▄▌","▄▄▖","▙▄▌","▄▄▌"};
	// ▗▄  ▗▄▖ ▗▄▖ ▄ ▄ ▗▄▖ ▄▄▖ ▄▄▖ ▄▄▖ ▄▄▖
	//  █  ▗▄▌ ▗▄▌ ▜▃█ ▐▄▖ ▙▄▖   ▌ ▙▄▌ ▙▄▌
	// ▗█▖ ▐▄▖ ▗▄▌   █ ▗▄▌ ▙▄▌   ▌ ▙▄▌ ▄▄▌

	struct abuf ab=ABUF_INIT;
	abAppend(&ab,"\x1b[?25l",6);
	abAppend(&ab,"\x1b[H",3);
    abAppend(&ab,"┏━━━━━━━┯━━━━━━━┯━━━━━━━┳━━━━━━━┯━━━━━━━┯━━━━━━━┳━━━━━━━┯━━━━━━━┯━━━━━━━┓\r\n┃  ",226);//8*9*3+3+2+10*9+3+2+3
    for(uint8_t i=0,j=0;i<81;i++){
    	if(sudoku[i]==0){
    		abAppend(&ab,"   ",3);
    	}
    	else{
    		uint8_t p=(sudoku[i]-1)*3+j;
    		abAppend(&ab,num[p],num_len[p]);
    	}
    	if(i%9==8){
    		if(j!=2){
    			i-=9;
    			j++;
    			abAppend(&ab,"  ┃\r\n┃  ",12);
    		}else{
    			j=0;
	    		if(i!=80){
	    			if(i%27==26){
	    				abAppend(&ab,"  ┃\r\n┣━━━━━━━┿━━━━━━━┿━━━━━━━╋━━━━━━━┿━━━━━━━┿━━━━━━━╋━━━━━━━┿━━━━━━━┿━━━━━━━┫\r\n┃  ",233);//97*2+3+219 
	    			}
	    			else{
						abAppend(&ab,"  ┃\r\n┣╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┠╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┠╶╶╶╶╶╶╶├╶╶╶╶╶╶╶├╶╶╶╶╶╶╶┃\r\n┃  ",233);
	    			}
	    		}
	    		else{
	    			abAppend(&ab,"  ┃\r\n┗━━━━━━━┷━━━━━━━┷━━━━━━━┻━━━━━━━┷━━━━━━━┷━━━━━━━┻━━━━━━━┷━━━━━━━┷━━━━━━━┛\r\n",228);
	    		}
    		}
    	}
    	else if(i%3==2){
    		abAppend(&ab,"  ┃  ",7);
    	}
    	else{
    		abAppend(&ab,"  ╎  ",7);
    	}
    }
	abAppend(&ab,"\x1b[?25h",6);
    if(write(STDOUT_FILENO,ab.b,ab.len)==-1){
    	//something went wrong
    }
    abFree(&ab);
}

void print_small_sudoku(const uint8_t sudoku[81]){
	//snprintf maybe
	struct abuf ab=ABUF_INIT;
	abAppend(&ab,"\x1b[?25l",6);
	abAppend(&ab,"\x1b[H",3);
    abAppend(&ab,"╭───────┬───────┬───────╮\r\n│ ",82);
    for(uint8_t i=0;i<81;i++){
    	if(sudoku[i]==0){
    		abAppend(&ab,"·",3);
    	}
    	else{
    		char c[1];
    		c[0]=48+sudoku[i];
    		abAppend(&ab,c,1);
    	}
    	if(i%9==8){
    		if(i!=80){
    			if(i%27==26){
    				abAppend(&ab," │\r\n├───────┼───────┼───────┤\r\n│ ",88);
    			}
    			else{
    				abAppend(&ab," │\r\n│ ",10);
    			}
    		}
    		else{
    			abAppend(&ab," │\r\n╰───────┴───────┴───────╯\r\n",84);
    		}
    	}
    	else if(i%3==2){
    		abAppend(&ab," │ ",5);
    	}else{
    		abAppend(&ab," ",1);
    	}
    }
    abAppend(&ab,"\x1b[?25h",6);
    if(write(STDOUT_FILENO,ab.b,ab.len)==-1){
    	//something went wrong
    }
    abFree(&ab);
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
	uint8_t grid[]={0,6,9,0,5,0,3,0,0,0,8,1,0,9,3,0,0,5,0,0,5,4,8,0,0,1,0,9,2,6,0,0,0,7,0,8,0,5,0,0,0,0,0,4,9,0,0,0,0,0,9,6,0,1,0,0,4,0,3,8,0,2,7,0,0,0,0,4,5,0,0,0,5,1,0,2,7,6,8,0,4};
	if(system("clear")){
		return -1;
	}
	if(argc==2){
		FILE* fp=fopen(argv[1],"r");
		if(!fp){
			printf("no file named found\n");
			return 0;
		}
		for(int i=0;i<81;i++){
			grid[i]=(uint8_t)fgetc(fp)-48;
		}
		fclose(fp);
	}
	E.raw_mode=0;
	enable_raw_mode(STDIN_FILENO);
	//const uint8_t grid[]={0,6,9,0,5,0,3,0,0,0,8,1,0,9,3,0,0,5,0,0,5,4,8,0,0,1,0,9,2,6,0,0,0,7,0,8,0,5,0,0,0,0,0,4,9,0,0,0,0,0,9,6,0,1,0,0,4,0,3,8,0,2,7,0,0,0,0,4,5,0,0,0,5,1,0,2,7,6,8,0,4};
	print_large_sudoku(grid);
	// write(STDOUT_FILENO,"\033[?1003h",8);
	while(1){
		if(process_key()){
			break;
		}
	}
	disable_raw_mode();
	printf("\033[?1003l");
	if(system("clear")){
		return -1;
	}
}

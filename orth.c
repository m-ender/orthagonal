/*
MIT License

Copyright (c) 1994, 2016 Jeff Epler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define OP_NOOP 	0
#define OP_PLUS 	1
#define OP_MINUS 	2
#define OP_MULT 	3
#define OP_DIV 		4
#define OP_MOD 		5
#define OP_EXCH 	6
#define OP_NOT 		7
#define OP_AND 		8
#define OP_OR           9
#define OP_XOR          10
#define OP_DUP          11
#define OP_DISC         12
#define OP_PUSH         13
#define OP_SET		14
#define OP_IF           15
#define OP_DX           16
#define OP_DY           17
#define OP_X            18
#define OP_Y            19
#define OP_CHAR         20
#define OP_STRING       21
#define OP_DECIMAL      22
#define OP_CCW          23
#define OP_CW           24
#define OP_REV          25
#define OP_H            26
#define OP_J            27
#define OP_K            28
#define OP_L            29
#define OP_RET          30
#define OP_MAX          31

char *operators[OP_MAX]= {
  "nop","+","-","*","/","%","~","!","&","|","^","@","$","=","#","?","dx","dy",
  "x","y","c","s","d","ccw","cw","rev","h","j","k","l","ret"
};

typedef int s32;

#define GRID_SIZE 256
/* Of course, if you change this you will break any
 * program that relies on wraparound
 */

#define STACK_SIZE 256
/* This ought to be enough for most people */

int type[GRID_SIZE][GRID_SIZE]; 
/* Holds a TRUE value if the associated grid coordinate
 * is an operator
 */

int grid[GRID_SIZE][GRID_SIZE]; 
/* Holds either an integer quantity, or the #defined value
 * of the operator
 */
int stack[STACK_SIZE];
int sp=0;

int parse_element(const char *e)  {
  int i=0;
  
  while(i<OP_MAX)  {
    if (!strcasecmp(e,operators[i])) return i;
    i++;
  }
  
  return OP_MAX;
}

int parse(FILE *f)  {
  char line[80];
  int lnum=0;
  int x,y;
  char element[80];
  
  while(!feof(f))  {
    fgets(line,79,f);
    lnum++;
    if(line[strlen(line)-1]!='\n')  {
      fprintf(stderr,"Warning: Long line %d",lnum);
      while(fgetc(f)!='\n') /* NOTHING */ ;
      line[79]='\n';
    }
    if(line[0]==';') continue;
    sscanf(line,"%d %d %s\n",&x,&y,element);
#ifdef DEBUGGER
    fprintf(stderr,"%d %d %s\n",x,y,element);
#endif

    if((element[0]=='-'&&element[1])||isdigit(element[0]))
      grid[x][y]=atoi(element);
    else if(element[0]=='\'')
      grid[x][y]=element[1];
    else {
      type[x][y]=1;
      if(OP_MAX==(grid[x][y]=parse_element(element)))  {
	fprintf(stderr,"Error: Line %d\n",lnum);
	exit(1);
      }
    }   
  }
  return 0;
}

int push(int quantity)  {
  if(sp==STACK_SIZE)  {
    fprintf(stderr,"Stack overflow\n");
    exit(1);
  }
#ifdef DEBUGGER
  fprintf(stderr,"push: %d\n",quantity);
#endif
  stack[sp++]=quantity;
  return 0;
}

int pop()  {
  if(sp==0)  {
    fprintf(stderr,"Stack underflow\n");
    exit(1);
  }
#ifdef DEBUGGER
  fprintf(stderr,"pop: %d\n",stack[sp-1]);
#endif
  return stack[--sp];
}

int execute()  {
  int x=0,y=0,dx=1,dy=0;
  int temp,temp2;
  
  while(1) {
#ifdef DEBUGGER
    if(type[x][y])
      fprintf(stderr,"(%4d,%4d)\t%s\t%d\n",x,y,operators[grid[x][y]],sp);
    else
      fprintf(stderr,"(%4d,%4d)\t%d\t%d\n",x,y,grid[x][y],sp);
#endif
    if(type[x][y])  {
      switch(grid[x][y])  {
       
       case OP_NOOP:
	break;
       
       case OP_PLUS:
	push(pop()+pop());
	break;
       
       case OP_MINUS:
	temp=pop();
	push(pop()-temp);
	break;
       
       case OP_MULT:
	push(pop()*pop());
	break;
       
       case OP_DIV:
	temp=pop();
	push(pop()/temp);
	break;
  
       case OP_MOD:
	temp=pop();
	push(pop()%temp);
	break;
	
       case OP_EXCH:
	temp=pop();
	temp2=pop();
	push(temp);
	push(temp2);
	break;
	
       case OP_NOT:
	push(!pop());
	break;
	
       case OP_AND:
	push(pop()&pop());
	break;
	
       case  OP_OR:
	push(pop()|pop());
        break;
	
       case OP_XOR:
	push(pop()^pop());
	break;
	
       case OP_DUP:
	temp=pop();
	push(temp);
	push(temp);
	break;
	
       case OP_DISC:
	pop();
	break;
	
       case OP_PUSH:
	temp=pop()%GRID_SIZE;
	temp2=pop()%GRID_SIZE;
	push(grid[temp][temp2]);
	break;
		
       case OP_SET:
	temp=pop()%GRID_SIZE;
	temp2=pop()%GRID_SIZE;
	grid[temp][temp2]=pop();
	type[temp][temp2]=0;
	break;

       case OP_IF:
	if(!pop())  {
	  x+=dx; y+=dy;
	}
	break;
	
       case OP_DX:
	dx=pop();
	break;
	
       case OP_DY:
	dy=pop();
	break;
	
       case OP_X:
	x=pop();
	break;
	
       case OP_Y:
	y=pop();
	break;
	
       case OP_CHAR:
	temp=pop();
	if(temp)
	  putchar(pop());
	else
	  putchar('\n');
	break;
	
       case OP_STRING:
	while((temp=pop())!=0)
	  putchar(temp);
	break;
	
       case OP_DECIMAL:
	printf("%d",pop());
	break;
	
       case OP_CCW:
	temp=dx;
	dx=dy;
	dy=-temp;
	break;
	
       case OP_CW:
	temp=dx;
	dx=-dy;
	dy=temp;
	break;
	
       case OP_REV:
	dx=-dx;
	dy=-dy;
	break;
	
       case OP_H:
	dx=-1;
	dy=0;
	break;
	
       case OP_J:
	dx=0;
	dy=1;
	break;
       
       case OP_K:
	dx=0;
	dy=-1;
	break;
	
       case OP_L:
	dx=1;
	dy=0;
	break;
	
       case OP_RET:
	return(pop());
	break;
	
       default:
	fprintf(stderr,"Unknown opcode %d at (%d,%d)",grid[x][y],x,y);
	exit(1);
      }
    } else
      push(grid[x][y]);
 
    x+=dx;
    while(x<0) x+=GRID_SIZE;
    x%=GRID_SIZE;
    
    y+=dy;
    while(y<0) y+=GRID_SIZE;
    y%=GRID_SIZE;
  }
}

int main(int argc, char **argv)  {
  int i;
  parse(stdin);
  if(argc>1)
    for(i=0;argv[1][i]&&(i<256);i++) {
      grid[i][GRID_SIZE-1]=argv[1][i];
      type[i][GRID_SIZE-1]=0;
    }
  return(execute());
}

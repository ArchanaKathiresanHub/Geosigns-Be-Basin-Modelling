#include <stdio.h>
#include <assert.h>
#include <ctype.h>

int isVerticalSpace(int c)
{
  return c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int isHorizontalSpace(int c)
{
  return c == ' ' || c == '\t'; 
}


int main( int argc, char ** argv)
{
  FILE *input = stdin, *output = stdout;
  int c = 0;
  enum { H_SPACE, V_SPACE, NOT_SPACE } state = NOT_SPACE;

  while( c = getc(input), c != EOF)
  {
    if (isHorizontalSpace(c))
    {
      if (state == H_SPACE)
      {
      }
      else if (state == V_SPACE)
      {
      }
      else if (state == NOT_SPACE)
      {
	state = H_SPACE;
      }
      else
      {
	assert( 0 && "Unknown state");
      }
    }
    else if (isVerticalSpace(c))
    {
      if (state == H_SPACE)
      {
	state = V_SPACE;
      }
      else if (state == V_SPACE)
      {
      }
      else if (state == NOT_SPACE)
      {
	state = V_SPACE;
      }
      else
      {
	assert( 0 && "Unknown state");
      }
    }
    else
    {
      if (state == H_SPACE)
      {
	putc(' ', output);
	putc(c, output);
	state = NOT_SPACE;
      }
      else if (state == V_SPACE)
      {
	putc('\n', output);
	putc(c, output);
	state = NOT_SPACE;
      }
      else if (state == NOT_SPACE)
      {
	putc(c, output);
      }
      else
      {
	assert( 0 && "Unknown state");
      }
    }
  }

  
  

  return 0;
}


#include "fibonacci.h"

// function definition

unsigned long fibonacci(unsigned long number)
{
     if ((number == 0) || (number == 1)) { // base cases
          return number;
     } else {
          return (fibonacci(number-1) + fibonacci(number-2));
     }
}

// end

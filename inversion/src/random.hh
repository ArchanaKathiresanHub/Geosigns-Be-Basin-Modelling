#ifndef RANDOM_HH
#define RANDOM_HH

//hasard == random
int random(int min, int max)
{  
  return (int) (min + ((double) rand() / RAND_MAX * (max - min + 1)));
}

#endif

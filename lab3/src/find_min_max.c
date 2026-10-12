#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  
  int min_t = array[begin];
  int max_t = array[begin];
  for (int i = begin; i <= end; i++)
  {
    if (array[i] < min_t)
    {
      min_t = array[i];
    }
    if (array[i] > max_t)
    {
      max_t = array[i];
    }
  }
  min_max.min = min_t;
  min_max.max = max_t;
  // your code here
  return min_max;
}


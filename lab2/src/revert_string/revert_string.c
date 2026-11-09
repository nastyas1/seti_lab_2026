#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
	int n = strlen(str);
	char copy[n];
	int t = 0;
	for (int i = n - 1; i >= 0; i--)
	{
		copy[t] = str[i];
		t++;
	}
	for (int i = 0; i < n; i++)
	{
		str[i] = copy[0];
	}

}


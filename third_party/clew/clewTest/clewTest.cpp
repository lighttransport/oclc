// clewTest.cpp : Defines the entry point for the console application.
//

#include "clew.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
	int ret = clewInit();
  printf("got it? %d\n", ret);
	return 0;
}


#include "stdafx.h"
#include "ServerPocClass.h"
#include <iostream>


int main()
{
 
	ServerPocClass * spc = new ServerPocClass("PipeExample", 1);
	spc->createMainServerThread();

	delete spc;

    return 0;
}


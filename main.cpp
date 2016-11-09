#include <regex>
#include <iostream>
#include <string>
#include "global.h"
#include "controler.h"
#include <fstream>

using namespace std;


int main () 
{
//	freopen("1.sql","r", stdin);
//	freopen("shouzhuabing.out","w", stdout);
	Controler::instance().run();

	return 0;
}

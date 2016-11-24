#include <Windows.h>

#include "PEAnalyser.h"
#include "PEInject.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	char *from = "D:\\RagnaWinter\\client\\agmon.dll";
	char *to = "D:\\RagnaWinter\\client\\bwinter.exe";
	char *saveto = "D:\\RagnaWinter\\client\\bwinterga.exe";

	PEInject pei;
	pei.InjectFile(to, saveto, from, NULL, 0);

	getchar();

	return 0;
}

#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"

#include <iostream>
#include <fstream>

using namespace transport_guide;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#define _YANDEX_SERVER_TESTING_ 01
#define _SMALL_OUTPUT_EXPECTED_ 01

void JSONinterface(istream& in, ostream& out, Catalogue& catalogue)
{
	PrintReports(ReadInputFile(in).UseInputFile(catalogue), out);
}

int main()
{
#if _YANDEX_SERVER_TESTING_
	auto& fin = cin;
	auto& fout = cout;
#else
	std::ifstream fin("input.txt");
#if _SMALL_OUTPUT_EXPECTED_
	auto& fout = cout;
#else
	std::ofstream fout("output.txt");
#endif
#endif
	Catalogue catalogue;
	JSONinterface(fin, fout, catalogue);
}
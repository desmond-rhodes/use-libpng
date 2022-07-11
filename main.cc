#include <exception>
#include <iostream>
#include <vector>
#include <string>
#include "display.hh"

using namespace std;

int main(int argc, char* argv[]) {
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);
	try {
		vector<string> args(argv, argv+argc);
		return display(args);
	}
	catch (exception const& e) {
		cerr << "An uncaught exception was encountered.\n";
		cerr << e.what() << '\n';
		return -1;
	}
	catch (...) {
		cerr << "An uncaught exception of unknown type was encountered.\n";
		return -1;
	}
}

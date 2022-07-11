#include "display.hh"
#include <iostream>

using namespace std;

int display(vector<string> const& args) {
	for (auto const& i : args)
		cout << i << ' ';
	cout << '\n';

	cout << "Hello, world!\n";
	return 0;
}

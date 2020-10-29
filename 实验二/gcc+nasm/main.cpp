#include <iostream>

using namespace std;

extern "C" int maxofthree(int, int, int);

int main(){
	cout << maxofthree(1, -4, -7) << endl;
	cout << maxofthree(2, -6, 1) << endl;
	return 0;
}
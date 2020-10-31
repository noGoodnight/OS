#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

extern "C" void asm_print(const char *, const int);

/*
 * -----------------------------------------------
 */

void printStr(const char *s);

void split(const string s, vector <string> sv, const char flag = ' ');

/*
 * -----------------------------------------------
 */

int main() {
    string command;
    vector <string> options;

    while (true) {
        printStr("> ");
        getline(cin, command);
        split(command, options, ' ');

        for (auto it = options.begin(); it != options.end();) {
            if (*it == "") {
                it = options.erase(it);
            } else {
                it++;
            }
        }

        if (options[0].compare("exit") == 0) {
            return 0;
        } else {
            printStr("next");
        }
    }
}

void printStr(const char *s) {
//    cout << strlen(s);
    asm_print(s, strlen(s));
}

void split(const string s, vector <string> sv, const char flag) {
    sv.clear();
    istringstream iss(s);
    string temp;
    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return;
}
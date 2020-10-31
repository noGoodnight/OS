#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

// ---------------------------------------------------------------------------------------------------------------------

extern "C" void asm_print(const char *, const int);

// ---------------------------------------------------------------------------------------------------------------------

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned char dword;

const string ParamError = "parameter";
const string CommandError = "command";
const string DirectoryError = "directory";
const string FileError = "file";
const string OpenFileError = "open";

int ByteInSector;
int SectorInCluster;
int SectorInBoot;
int NumOfFAT;
int FileInRoot;
int SectorInFAT;

#pragma pack(1)

struct Boot {
    word byteInSector;
    byte sectorInCluster;
    word sectorInBoot;
    byte numOfFAT;
    word fileInRoot;
    word totalNumOfSector;
    byte media;
    word sectorInFAT;
    word sectorInTrack;
    word numOfHead;
    dword hiddenSector;
    dword sectorInFAT2; // if totalNumOfSector == 0
};

struct Root {
    char rootName[11];
    byte rootAttributes;
    char reserverd[10];
    word modifiedTime;
    word modifiedDate;
    word firstCluster;
    dword fileSize;
};

#pragma pack()

class Node {
public:
    string name;
    vector<Node *> files;
    string path;
    dword fileSize;
    bool isFile = false;
    bool isRoot = true;
    int numOfDirectory = 0;
    int numOfFiles = 0;
    char *content = new char[5000]{NULL};
};

// ---------------------------------------------------------------------------------------------------------------------

void printStr(const char *s);

void split(const string s, vector<string> sv, const char flag = ' ');

void readBoot(FILE *fat12, Boot *boot);

void readFiles(FILE *fat12, Root *root, Node *rootNode);

void getContent(FILE *fat12, int startCluster, Node *son);

// ---------------------------------------------------------------------------------------------------------------------

int main() {
    FILE *fat12;
    Boot *boot;
    Root *root;
    Node *rootNode;

    fat12 = fopen("a.img", "rb");
    boot = new Boot;
    root = new Root;
    rootNode = new Node;
    rootNode->name = "";
    rootNode->path = "/";

    readBoot(fat12, boot);

    ByteInSector = boot->byteInSector;
    SectorInCluster = boot->sectorInCluster;
    SectorInBoot = boot->sectorInBoot;
    NumOfFAT = boot->numOfFAT;
    FileInRoot = boot->fileInRoot;
    SectorInFAT = (boot->sectorInFAT != 0) ? boot->sectorInFAT : boot->sectorInFAT2;

    while (true) {
        string command;
        vector<string> options;

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
    asm_print(s, strlen(s));
}

void split(const string s, vector<string> sv, const char flag) {
    sv.clear();
    istringstream iss(s);
    string temp;
    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return;
}

void readBoot(FILE *fat12, Boot *boot) {
    if (fseek(fat12, 11, SEEK_SET) == -1) {
        printStr("main readBoot fseek\n");
    }
    if (fread(boot, 1, 25, fat12) != 25) {
        printStr("main readBoot fread\n");
    }
}

void readFiles(FILE *fat12, Root *root, Node *father) {
    int offset = (SectorInBoot * 1 + NumOfFAT * SectorInFAT) * ByteInSector;
    char filename[12];

    for (int i = 0; i < FileInRoot; i++) {
        if (fseek(fat12, offset, SEEK_SET) == -1) {
            printStr("main readFiles fseek\n");
        }
        if (fread(root, 1, 32, fat12) != 32) {
            printStr("main readFiles fread\n");
        }
        offset += 32;

        if (root->rootName[0] == '\0') {
            continue;
        }

        if (root->rootAttributes != 0x10) {
            int tempLong = -1;
            for (int k = 0; k < 11; k++) {
                if (root->rootName[k] != ' ') {
                    tempLong++;
                    filename[tempLong] = root->rootName[k];
                } else { // filename: name (space) point extension (space)
                    tempLong++;
                    filename[tempLong] = '.';
                    while (root->rootName[k] == ' ') {
                        k++;
                    }
                    k--;
                }
            }
            tempLong++;
            filename[tempLong] = '\0';

            Node *son = new Node;
            father->files.push_back(son);
            son->name = filename;
            son->fileSize = root->fileSize;
            son->isFile = true;
            son->path = father->path + filename + "/";
            father->numOfFiles++;
            getContent(fat12, root->firstCluster, son);
        } else {
            int tempLong = -1;
            for(int k = 0;k<11;k++){
                
            }
        }
    }
}
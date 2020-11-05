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
typedef unsigned int dword;

const string CommandError = "Error! Invalid Command!\n";
const string OptionError = "Error! Invalid Option\n";
const string PathError = "Error! Invalid Path\n";
const string DirectoryError = "Error! Can Not Open The Directory\n";
const string FileError = "Error! Can Not Open The File\n";

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
    char reserved[10];
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
    bool isValue = true;
    int numOfDirectory = 0;
    int numOfFile = 0;
    char *content = new char[5000]{NULL};
};

// ---------------------------------------------------------------------------------------------------------------------

void printStr(const char *);

vector<string> split(const string, vector<string>, const char flag = ' ');

void readBoot(FILE *, Boot *);

void readRootFiles(FILE *, Root *, Node *);

void getContent(FILE *, int, Node *);

void createDirectoryNode(Node *);

void readDirectoryFiles(FILE *, int, Node *);

int getFATValue(FILE *, int);

Node *findPath(Node *, string);

void printLS(Node *, bool);

void printCAT(Node *);

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

    readRootFiles(fat12, root, rootNode);

    while (true) {
        string command;
        vector<string> options;
        bool nextCommand = false;

        printStr("> ");
        getline(cin, command);
        options = split(command, options, ' ');

        for (vector<string>::iterator it = options.begin(); it != options.end();) {
            if (*it == "") {
                it = options.erase(it);
            } else {
                it++;
            }
        }

        if (options[0].compare("exit") == 0) {
            fclose(fat12);
            return 0;
        } else if (options[0].compare("ls") == 0) {
            string path = "/";
            bool detailed = false;
            bool ackPath = false;

            for (int i = 1; i < options.size(); i++) {
                if (options[i][0] == '-') {
                    detailed = true;
                    if (options[i].length() == 1) {
                        printStr(OptionError.c_str());
                        nextCommand = true;
                        break;
                    }
                    for (int j = 1; j < options[i].length(); j++) {
                        if (options[i][j] != 'l') {
                            printStr(OptionError.c_str());
                            nextCommand = true;
                            break;
                        }
                    }
                } else if (options[i][0] == '/') {
                    if (ackPath) {
                        printStr(PathError.c_str());
                        nextCommand = true;
                        break;
                    } else {
                        path = options[i];
                        ackPath = true;
                    }
                } else {
                    printStr(OptionError.c_str());
                    nextCommand = true;
                    break;
                }
            }
            if (nextCommand) {
                continue;
            }

            if (path[path.length() - 1] != '/') {
                path = path + "/";
            }
            Node *result = findPath(rootNode, path);
            if (result == nullptr || result->isValue == false || result->isFile == true) {
                printStr(DirectoryError.c_str());
                continue;
            } else {
                printLS(result, detailed);
            }
        } else if (options[0].compare("cat") == 0) {
            if (options.size() == 1 || options.size() > 2) {
                printStr(OptionError.c_str());
                continue;
            }

            string filePath = options[1];
            if (filePath[0] != '/') {
                filePath = "/" + filePath;
            }
            Node *result = findPath(rootNode, filePath + "/");
            if (result == nullptr || result->isFile == false) {
                printStr(FileError.c_str());
                continue;
            } else {
                printCAT(result);
            }
        } else {
            printStr(CommandError.c_str());
        }
    }
}

void printStr(const char *s) {
    asm_print(s, strlen(s));
}

vector<string> split(const string s, vector<string> sv, const char flag) {
    sv.clear();
    istringstream iss(s);
    string temp;
    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return sv;
}

void readBoot(FILE *fat12, Boot *boot) {
    if (fseek(fat12, 11, SEEK_SET) == -1) {
        printStr("readBoot fseek\n");
    }
    if (fread(boot, 1, 25, fat12) != 25) {
        printStr("readBoot fread\n");
    }
}

void readRootFiles(FILE *fat12, Root *root, Node *father) {
    int offset = (SectorInBoot + NumOfFAT * SectorInFAT) * ByteInSector;
    char filename[12];

    for (int i = 0; i < FileInRoot; i++) {
        if (fseek(fat12, offset, SEEK_SET) == -1) {
            printStr("readFiles fseek\n");
        }
        if (fread(root, 1, 32, fat12) != 32) {
            printStr("readFiles fread\n");
        }
        offset += 32;

        if (root->rootName[0] == '\0') {
            continue;
        }

        int boolean = 0;
        for (int j = 0; j < 11; j++) {
            if (!(((root->rootName[j] >= 48) && (root->rootName[j] <= 57)) ||
                  ((root->rootName[j] >= 65) && (root->rootName[j] <= 90)) ||
                  ((root->rootName[j] >= 97) && (root->rootName[j] <= 122)) ||
                  (root->rootName[j] == ' '))) {
                boolean = 1;    //非英文及数字、空格
                break;
            }
        }
        if (boolean == 1) {
            continue;
        }

        if ((root->rootAttributes & 0x10) == 0) {
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
            father->numOfFile++;
            getContent(fat12, root->firstCluster, son);
        } else {
            int tempLong = -1;
            for (int k = 0; k < 11; k++) {
                if (root->rootName[k] != ' ') {
                    tempLong++;
                    filename[tempLong] = root->rootName[k];
                } else {
                    tempLong++;
                    filename[tempLong] = '\0';
                    break;
                }
            }
            Node *son = new Node;
            father->files.push_back(son);
            son->name = filename;
            son->path = father->path + filename + "/";
            father->numOfDirectory++;

            createDirectoryNode(son);
            readDirectoryFiles(fat12, root->firstCluster, son);
        }
    }
}

void getContent(FILE *fat12, int startCluster, Node *son) {
    int offset = ByteInSector * (SectorInBoot + SectorInFAT * NumOfFAT +
                                 (FileInRoot * 32 + ByteInSector - 1) / ByteInSector); // some question
    int currentCluster = startCluster;
    int value = 0;
    char *p = son->content;

    if (startCluster == 0) {
        return;
    }
    while (value < 0xFF8) {
        value = getFATValue(fat12, currentCluster);
        if (value == 0xFF7) {
            printStr("getContent value==0xFF7\n");
            break;
        }
        char *str = (char *) malloc(SectorInCluster * ByteInSector);
        int startByte = offset + (currentCluster - 2) * SectorInCluster * ByteInSector;

        if (fseek(fat12, startByte, SEEK_SET) == -1)
            printStr("getContent fseek\n");
        if (fread(str, 1, SectorInCluster * ByteInSector, fat12) != SectorInCluster * ByteInSector)
            printStr("getContent fread\n");
        for (int i = 0; i < SectorInCluster * ByteInSector; i++) {
            *p = str[i];
            p++;
        }
        free(str);
        currentCluster = value;
    }
}

void createDirectoryNode(Node *node) {
    Node *q = new Node;
    q->name = ".";
    q->isValue = false;
    node->files.push_back(q);
    q = new Node;
    q->name = "..";
    q->isValue = false;
    node->files.push_back(q);
}

void readDirectoryFiles(FILE *fat12, int startCluster, Node *father) {
    int offset = ByteInSector *
                 (SectorInBoot * 1 + NumOfFAT * SectorInFAT + (FileInRoot * 32 + ByteInSector - 1) / ByteInSector);
    int currentCluster = startCluster;
    int value = 0;

    while (value < 0xFF8) {
        value = getFATValue(fat12, currentCluster);
        if (value == 0xFF7) {
            printStr("readDirectoryFiles\n");
            break;
        }

        int startByte = offset + (currentCluster - 2) * SectorInCluster * ByteInSector;
        int count = SectorInCluster * ByteInSector;    //每簇的字节数
        int loop = 0;

        while (loop < count) {
            char filename[12];
            Root *sonRoot = new Root;

            if (fseek(fat12, startByte + loop, SEEK_SET) == -1)
                printStr("readDirectoryFiles fseek\n");
            if (fread(sonRoot, 1, 32, fat12) != 32)
                printStr("readDirectoryFiles fread\n");

            loop += 32;
            if (sonRoot->rootName[0] == '\0') {
                continue;
            }

            int boolean = 0;
            for (int j = 0; j < 11; j++) {
                if (!(((sonRoot->rootName[j] >= 48) && (sonRoot->rootName[j] <= 57)) ||
                      ((sonRoot->rootName[j] >= 65) && (sonRoot->rootName[j] <= 90)) ||
                      ((sonRoot->rootName[j] >= 97) && (sonRoot->rootName[j] <= 122)) ||
                      (sonRoot->rootName[j] == ' '))) {
                    boolean = 1;    //非英文及数字、空格
                    break;
                }
            }
            if (boolean == 1) {
                continue;
            }

            if ((sonRoot->rootAttributes & 0x10) == 0) {
                int tempLong = -1;
                for (int k = 0; k < 11; k++) {
                    if (sonRoot->rootName[k] != ' ') {
                        tempLong++;
                        filename[tempLong] = sonRoot->rootName[k];
                    } else { // filename: name (space) point extension (space)
                        tempLong++;
                        filename[tempLong] = '.';
                        while (sonRoot->rootName[k] == ' ') {
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
                son->fileSize = sonRoot->fileSize;
                son->isFile = true;
                son->path = father->path + filename + "/";
                father->numOfFile++;
                getContent(fat12, sonRoot->firstCluster, son);
            } else {
                int tempLong = -1;
                for (int k = 0; k < 11; k++) {
                    if (sonRoot->rootName[k] != ' ') {
                        tempLong++;
                        filename[tempLong] = sonRoot->rootName[k];
                    } else {
                        tempLong++;
                        filename[tempLong] = '\0';
                        break;
                    }
                }
                Node *son = new Node;
                father->files.push_back(son);
                son->name = filename;
                son->path = father->path + filename + "/";
                father->numOfDirectory++;

                createDirectoryNode(son);
                readDirectoryFiles(fat12, sonRoot->firstCluster, son);
            }
        }

        currentCluster = value;
    }
}

int getFATValue(FILE *fat12, int num) {
    int fatBase = SectorInBoot * ByteInSector;
    int fatPosition = fatBase + num * 3 / 2;
    int type = (num % 2 == 0) ? 0 : 1;
    word bytes;
    word *bytes_ptr = &bytes;

    if (fseek(fat12, fatPosition, SEEK_SET) == -1) {
        printStr("getFATValue fseek\n");
    }
    if (fread(bytes_ptr, 1, 2, fat12) != 2) {
        printStr("getFATValue fread\n");
    }

    if (type == 0) {
        bytes = bytes << 4;
        return bytes >> 4;
    } else {
        return bytes >> 4;
    }
}

Node *findPath(Node *root, string path) {
    if (root->path.compare(path) == 0) {
        return root;
    } else {
        for (int i = 0; i < root->files.size(); i++) {
            Node *result = findPath(root->files[i], path);
            if (result != nullptr) {
                return result;
            }

        }
    }
    return nullptr;
}

void printLS(Node *root, bool detailed) {
    printStr(root->path.c_str());
    if (!detailed) {
        printStr(":\n");
        for (int i = 0; i < root->files.size(); i++) {
            if (root->files[i]->isFile == false) {
                printStr(("\033[31m" + root->files[i]->name + "\033[0m").c_str());
            } else {
                printStr(root->files[i]->name.c_str());
            }
            if (i == root->files.size() - 1) {
                printStr("\n");
            } else {
                printStr(" ");
            }
        }
    } else {
        printStr((" " + to_string(root->numOfDirectory) + " " + to_string(root->numOfFile) + ":\n").c_str());
        for (int i = 0; i < root->files.size(); i++) {
            if (root->files[i]->isFile == false) {
                if (root->files[i]->isValue == false) {
                    printStr(("\033[31m" + root->files[i]->name + "\033[0m").c_str());
                } else {
                    printStr(("\033[31m" + root->files[i]->name + "\033[0m" + " " +
                              to_string(root->files[i]->numOfDirectory) + " " +
                              to_string(root->files[i]->numOfFile)).c_str());
                }
            } else {
                printStr((root->files[i]->name + " " + to_string(root->files[i]->fileSize)).c_str());
            }
            printStr("\n");
        }
        printStr("\n");
    }

    for (int i = 0; i < root->files.size(); i++) {
        if (root->files[i]->isValue == true && root->files[i]->isFile == false) {
            printLS(root->files[i], detailed);
        }
    }
}

void printCAT(Node *root) {
    if (root->content[0] != 0) {
        printStr(root->content);
    }
    printStr("\n");
}


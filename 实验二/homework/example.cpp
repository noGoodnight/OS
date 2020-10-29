//
// Created by daisy on 2020/10/29.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<iostream>
#include<string>
#include<sstream>
#include<vector>


using namespace std;

extern "C" {
void asm_print(const char *, const int);
}
typedef unsigned char u8;    //1字节
typedef unsigned short u16;    //2字节
typedef unsigned int u32;    //4字节

const char *ERR_PARAMETER_WRONG = "error: The parameter of ls is wrong!\n";
const char *ERR_COMMAND_WRONG = "error: command not found!\n";
const char *ERR_NO_DIR = "error: cant find the dir!\n";
const char *ERR_NO_FILE = "error: cant find the file!\n";
const char *ERR_CANT_OPEN = "error:file can not open!\n";


int BytsPerSec;    //每扇区字节数
int SecPerClus;    //每簇扇区数
int RsvdSecCnt;    //Boot记录占用的扇区数
int NumFATs;    //FAT表个数
int RootEntCnt;    //根目录最大文件数
int FATSz;    //FAT扇区数

string str_print;

#pragma pack (1) /*指定按1字节对齐*/

//偏移11个字节
struct BPB {
    u16 BPB_BytsPerSec;    //每扇区字节数
    u8 BPB_SecPerClus;    //每簇扇区数
    u16 BPB_RsvdSecCnt;    //Boot记录占用的扇区数
    u8 BPB_NumFATs;    //FAT表个数
    u16 BPB_RootEntCnt;    //根目录最大文件数
    u16 BPB_TotSec16;        //扇区总数
    u8 BPB_Media;        //介质描述符
    u16 BPB_FATSz16;    //FAT扇区数
    u16 BPB_SecPerTrk;  //	每磁道扇区数（Sector/track）
    u16 BPB_NumHeads;    //磁头数（面数）
    u32 BPB_HiddSec;    //隐藏扇区数
    u32 BPB_TotSec32;    //如果BPB_FATSz16为0，该值为FAT扇区数
};
//BPB至此结束，长度25字节

//根目录条目
struct RootEntry {
    char DIR_Name[11];
    u8 DIR_Attr;        //文件属性
    char reserved[10];
    u16 DIR_WrtTime;
    u16 DIR_WrtDate;
    u16 DIR_FstClus;    //开始簇号
    u32 DIR_FileSize;
};
//根目录条目结束，32字节

#pragma pack () /*取消指定对齐，恢复缺省对齐*/


class Node {//链表的node类
public:
    string name;        //名字
    vector<Node *> next;    //下一级目录的Node数组
    string path;            //记录path，便于打印操作
    u32 FileSize;            //文件大小
    bool isfile = false;        //是文件还是目录
    bool isval = true;            //用于标记是否是.和..
    int dir_count = 0;            //记录下一级有多少目录
    int file_count = 0;            //记录下一级有多少文件
    char *content = new char[10000]{NULL};        //存放文件内容
};


void fillBPB(FILE *fat12, struct BPB *bpb_ptr);    //载入BPB
void ReadFiles(FILE *fat12, struct RootEntry *rootEntry_ptr, Node *father);    //读取根目录
void readChildren(FILE *fat12, int startClus, Node *father);    //读取子目录
int getFATValue(FILE *fat12, int num);    //读取FAT表，获取下一个簇号
void printLS(Node *root);      //打印ls命令
void creatNode(Node *p, Node *father);            //创建.和..node，但是并没有实际意义，没有实现cd命令
void printLS_L(Node *root);                    //打印 ls -l命令
void printLSWithPath(Node *root, string path, int &exist, bool hasL);            //打印ls -l 地址  命令
void getContent(FILE *fat12, int startClus, Node *son);                //初始化链表时用来获取文件中的信息
void printCat(Node *root, string path, int &exist);                //打印cat命令
void myPrint(const char *p);

void split(const string &s, vector <string> &sv, const char flag = ' ');

void pathDeal(string &s);

int main() {
    FILE *fat12;
    fat12 = fopen("a.img", "rb");    //打开FAT12的映像文件

    struct BPB bpb;
    struct BPB *bpb_ptr = &bpb;

    //创建根节点
    Node *root = new Node();
    root->name = "";
    root->path = "/";

    //载入BPB
    fillBPB(fat12, bpb_ptr);

    //初始化各个全局变量
    BytsPerSec = bpb_ptr->BPB_BytsPerSec;
    SecPerClus = bpb_ptr->BPB_SecPerClus;
    RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
    NumFATs = bpb_ptr->BPB_NumFATs;
    RootEntCnt = bpb_ptr->BPB_RootEntCnt;
    if (bpb_ptr->BPB_FATSz16 != 0) {
        FATSz = bpb_ptr->BPB_FATSz16;
    } else {
        FATSz = bpb_ptr->BPB_TotSec32;
    }

    struct RootEntry rootEntry;
    struct RootEntry *rootEntry_ptr = &rootEntry;

    ReadFiles(fat12, rootEntry_ptr, root);//*****构建文件链表

    while (true) {   //解析输入的命令
        //cout << "> ";
        myPrint(">");
        string input;
        getline(cin, input);
        vector <string> input_list;
        split(input, input_list, ' ');
        for (auto it = input_list.begin(); it != input_list.end();) {//删除数组中空格产生的空位置
            if (*it == "") {
                it = input_list.erase(it);
            } else {
                it++;
            }
        }
        if (input_list[0].compare("exit") == 0) {
            //exit 退出情况
            myPrint("Bye\n");
            fclose(fat12);
            return 0;
        } else if (input_list[0].compare("ls") == 0) {
            //ls系列命令
            //情况1 ls
            if (input_list.size() == 1) {
                printLS(root);
            } else {
                bool hasL = false;
                bool hasPath = false;
                bool error = false;
                string *path = NULL;
                for (int i = 1; i < input_list.size(); i++) {  //遍历所有参数
                    string s = input_list[i];
                    if (s[0] != '-') {
                        //路径
                        if (hasPath) {
                            myPrint(ERR_PARAMETER_WRONG);
                            error = true;
                            break;
                        } else {
                            hasPath = true;
                            pathDeal(input_list[i]);
                            path = &input_list[i];
                        }
                    } else {
                        //-参数
                        if (s.length() == 1) {
                            //   -
                            myPrint(ERR_PARAMETER_WRONG);
                            error = true;
                            break;
                        }
                        for (int j = 1; j < s.length(); j++) {
                            if (s[j] != 'l') {
                                error = true;
                                myPrint(ERR_PARAMETER_WRONG);
                                break;
                            }
                        }
                        hasL = true;
                    }
                }
                if (error) {
                    continue;
                }
                //情况2  ls -l
                int exist = 0;
                if (hasL && !hasPath) {
                    exist = 1;
                    printLS_L(root);
                } else if (!hasL && hasPath) {
                    //情况3 ls NJU
                    printLSWithPath(root, *path, exist, false);
                } else if (hasL && hasPath) {
                    printLSWithPath(root, *path, exist, true);
                } else {
                    printLS(root);
                    continue;
                }
                if (exist == 0) {
                    myPrint(ERR_NO_DIR);
                    continue;
                } else if (exist == 2) {
                    myPrint(ERR_CANT_OPEN);
                    continue;
                }
            }

        } else if (input_list[0].compare("cat") == 0) {
            //cat系列命令
            if (input_list.size() == 2 && input_list[1][0] != '-') {
                //cat path
                int exist = 0;
                pathDeal(input_list[1]);
                printCat(root, input_list[1], exist);   //执行cat
                if (exist == 0) {
                    myPrint(ERR_NO_FILE);
                    continue;
                } else if (exist == 2) {
                    myPrint(ERR_CANT_OPEN);
                    continue;
                }
            } else {
                //cat后无参数或者参数过多,error
                myPrint(ERR_PARAMETER_WRONG);
                continue;
            }
        } else {
            //错误命令
            myPrint(ERR_COMMAND_WRONG);
            continue;
        }


    }
}


void fillBPB(FILE *fat12, struct BPB *bpb_ptr) {  //读取boot信息
    int check;

    //BPB从偏移11个字节处开始
    check = fseek(fat12, 11, SEEK_SET);
    if (check == -1)
        myPrint("fseek in fillBPB failed!\n");

    //BPB长度为25字节
    check = fread(bpb_ptr, 1, 25, fat12);
    if (check != 25)
        myPrint("fread in fillBPB failed!\n");
}


void ReadFiles(FILE *fat12, struct RootEntry *rootEntry_ptr, Node *father) {
    int base = (RsvdSecCnt + NumFATs * FATSz) * BytsPerSec;    //根目录首字节的偏移数
    int check;
    char realName[12];    //暂存文件名

    //依次处理根目录中的各个条目
    int i;
    for (i = 0; i < RootEntCnt; i++) {

        check = fseek(fat12, base, SEEK_SET);
        if (check == -1)
            myPrint("fseek in printFiles failed!\n");

        check = fread(rootEntry_ptr, 1, 32, fat12);
        if (check != 32)
            myPrint("fread in printFiles failed!\n");

        base += 32;

        if (rootEntry_ptr->DIR_Name[0] == '\0') continue;    //空条目不输出

        //过滤非目标文件
        int j;
        int boolean = 0;
        for (j = 0; j < 11; j++) {
            if (!(((rootEntry_ptr->DIR_Name[j] >= 48) && (rootEntry_ptr->DIR_Name[j] <= 57)) ||
                  ((rootEntry_ptr->DIR_Name[j] >= 65) && (rootEntry_ptr->DIR_Name[j] <= 90)) ||
                  ((rootEntry_ptr->DIR_Name[j] >= 97) && (rootEntry_ptr->DIR_Name[j] <= 122)) ||
                  (rootEntry_ptr->DIR_Name[j] == ' '))) {
                boolean = 1;    //非英文及数字、空格
                break;
            }
        }
        if (boolean == 1) continue;    //非目标文件不输出

        int k;   //名字的处理
        if ((rootEntry_ptr->DIR_Attr & 0x10) == 0) {
            //文件
            int tempLong = -1;
            for (k = 0; k < 11; k++) {
                if (rootEntry_ptr->DIR_Name[k] != ' ') {
                    tempLong++;
                    realName[tempLong] = rootEntry_ptr->DIR_Name[k];
                } else {
                    tempLong++;
                    realName[tempLong] = '.';
                    while (rootEntry_ptr->DIR_Name[k] == ' ') k++;
                    k--;
                }
            }
            tempLong++;
            realName[tempLong] = '\0';    //到此为止，把文件名提取出来放到了realName里
            Node *son = new Node();   //新建该文件的节点
            father->next.push_back(son);  //存到father的next数组中
            son->name = realName;
            son->FileSize = rootEntry_ptr->DIR_FileSize;
            son->isfile = true;
            son->path = father->path + realName + "/";
            father->file_count++;
            getContent(fat12, rootEntry_ptr->DIR_FstClus, son);//读取文件的内容
        } else {
            //目录
            int tempLong = -1;
            for (k = 0; k < 11; k++) {
                if (rootEntry_ptr->DIR_Name[k] != ' ') {
                    tempLong++;
                    realName[tempLong] = rootEntry_ptr->DIR_Name[k];
                } else {
                    tempLong++;
                    realName[tempLong] = '\0';
                    break;
                }
            }    //到此为止，把目录名提取出来放到了realName
            Node *son = new Node();
            father->next.push_back(son);
            son->name = realName;
            son->path = father->path + realName + "/";
            father->dir_count++;
            creatNode(son, father);
            //输出目录及子文件
            readChildren(fat12, rootEntry_ptr->DIR_FstClus, son);  //读取目录的内容
        }
    }
}


void readChildren(FILE *fat12, int startClus, Node *father) {
    //数据区的第一个簇（即2号簇）的偏移字节
    int dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);

    int currentClus = startClus;
    int value = 0;//value用来查看是否存在多个簇（查FAT表）
    while (value < 0xFF8) {
        value = getFATValue(fat12, currentClus);//查FAT表获取下一个簇号
        if (value == 0xFF7) {
            myPrint("坏簇，读取失败!\n");
            break;
        }


        int startByte = dataBase + (currentClus - 2) * SecPerClus * BytsPerSec;
        int check;

        int count = SecPerClus * BytsPerSec;    //每簇的字节数
        int loop = 0;
        while (loop < count) {
            int i;

            RootEntry sonEntry;//读取目录项
            RootEntry *sonEntryP = &sonEntry;
            check = fseek(fat12, startByte + loop, SEEK_SET);
            if (check == -1)
                myPrint("fseek in printFiles failed!\n");

            check = fread(sonEntryP, 1, 32, fat12);
            if (check != 32)
                myPrint("fread in printFiles failed!\n");//读取完毕
            loop += 32;
            if (sonEntryP->DIR_Name[0] == '\0') {
                continue;
            }    //空条目不输出
            //过滤非目标文件
            int j;
            int boolean = 0;
            for (j = 0; j < 11; j++) {
                if (!(((sonEntryP->DIR_Name[j] >= 48) && (sonEntryP->DIR_Name[j] <= 57)) ||
                      ((sonEntryP->DIR_Name[j] >= 65) && (sonEntryP->DIR_Name[j] <= 90)) ||
                      ((sonEntryP->DIR_Name[j] >= 97) && (sonEntryP->DIR_Name[j] <= 122)) ||
                      (sonEntryP->DIR_Name[j] == ' '))) {
                    boolean = 1;    //非英文及数字、空格
                    break;
                }
            }
            if (boolean == 1) {
                continue;
            }


            if ((sonEntryP->DIR_Attr & 0x10) == 0) {
                //文件处理
                char tempName[12];    //暂存替换空格为点后的文件名
                int k;
                int tempLong = -1;
                for (k = 0; k < 11; k++) {
                    if (sonEntryP->DIR_Name[k] != ' ') {
                        tempLong++;
                        tempName[tempLong] = sonEntryP->DIR_Name[k];
                    } else {
                        tempLong++;
                        tempName[tempLong] = '.';
                        while (sonEntryP->DIR_Name[k] == ' ') k++;
                        k--;
                    }
                }
                tempLong++;
                tempName[tempLong] = '\0';    //到此为止，把文件名提取出来放到tempName里
                Node *son = new Node();
                father->next.push_back(son);
                son->name = tempName;
                son->FileSize = sonEntryP->DIR_FileSize;
                son->isfile = true;
                son->path = father->path + tempName + "/";
                father->file_count++;
                getContent(fat12, sonEntryP->DIR_FstClus, son);

            } else {
                char tempName[12];
                int count = -1;
                for (int k = 0; k < 11; k++) {
                    if (sonEntryP->DIR_Name[k] != ' ') {
                        count++;
                        tempName[count] = sonEntryP->DIR_Name[k];
                    } else {
                        count++;
                        tempName[count] = '\0';
                    }
                }

                Node *son = new Node();
                father->next.push_back(son);
                son->name = tempName;
                son->path = father->path + tempName + "/";
                father->dir_count++;
                creatNode(son, father);
                readChildren(fat12, sonEntryP->DIR_FstClus, son);
            }


        }


        currentClus = value;//下一个簇
    };
}


int getFATValue(FILE *fat12, int num) {
    //FAT1的偏移字节
    int fatBase = RsvdSecCnt * BytsPerSec;
    //FAT项的偏移字节
    int fatPos = fatBase + num * 3 / 2;
    //奇偶FAT项处理方式不同，分类进行处理，从0号FAT项开始
    int type = 0;
    if (num % 2 == 0) {
        type = 0;
    } else {
        type = 1;
    }

    //先读出FAT项所在的两个字节
    u16 bytes;
    u16 *bytes_ptr = &bytes;
    int check;
    check = fseek(fat12, fatPos, SEEK_SET);
    if (check == -1)
        myPrint("fseek in getFATValue failed!");

    check = fread(bytes_ptr, 1, 2, fat12);
    if (check != 2)
        myPrint("fread in getFATValue failed!");

    //u16为short，结合存储的小尾顺序和FAT项结构可以得到
    //type为0的话，取byte2的低4位和byte1构成的值，type为1的话，取byte2和byte1的高4位构成的值
    if (type == 0) {
        bytes = bytes << 4;   //这里原文错误，原理建议看网上关于FAT表的文章
        return bytes >> 4;
    } else {
        return bytes >> 4;
    }
}

void printLS(Node *r) {
    Node *p = r;//通过root节点，遍历所有的节点
    if (p->isfile == true) {
        return;
    } else {
        //cout << p->path << ":" << endl;
        str_print = p->path + ":\n";
        myPrint(str_print.c_str());
        str_print.clear();
        //打印每个next
        Node *q;
        int leng = p->next.size();
        for (int i = 0; i < leng; i++) {
            q = p->next[i];
            if (q->isfile == false) {
                //文件夹
                //			cout << "\033[31m"<<q->name << "\033[0m"<<"  ";
                //cout << q->name << "  ";
                str_print = "\033[31m" + q->name + "\033[0m" + "  ";
                myPrint(str_print.c_str());
                str_print.clear();
            } else {
                //文件
                //cout << q->name << "  ";
                str_print = q->name + "  ";
                myPrint(str_print.c_str());
                str_print.clear();
            }
        }
        //cout << endl;
        str_print = "\n";
        myPrint(str_print.c_str());
        str_print.clear();
        //进行递归
        for (int i = 0; i < leng; i++) {
            if (p->next[i]->isval == true) printLS(p->next[i]);
        }


    }


}

void creatNode(Node *p, Node *father) {
    Node *q = new Node();
    q->name = ".";
    q->isval = false;
    p->next.push_back(q);
    q = new Node();
    q->name = "..";
    q->isval = false;
    p->next.push_back(q);
}

void printLS_L(Node *root) {
    Node *p = root;
    if (p->isfile) {
        //如果该Node是文件，不处理
        return;
    } else {
        //cout << p->path <<" "<<p->dir_count<<" "<<p->file_count<< ":" << endl;
        str_print = p->path + " " + to_string(p->dir_count) + " " + to_string(p->file_count) + ":\n";
        myPrint(str_print.c_str());
        str_print.clear();
        //打印每个next
        Node *q;
        int leng = p->next.size();
        for (int i = 0; i < leng; i++) {
            q = p->next[i];
            if (q->isfile == false) {
                //文件夹
                //			cout << "\033[31m"<<q->name << "\033[0m"<<"  ";
                if (q->isval) {
                    //cout << q->name << "  " << q->dir_count << " " << q->file_count << endl;
                    str_print = "\033[31m" + q->name + "\033[0m" + "  " + to_string(q->dir_count) + " " +
                                to_string(q->file_count) + "\n";
                    myPrint(str_print.c_str());
                    str_print.clear();
                } else {
                    //处理. ..
                    //cout << q->name << "  "<<endl;
                    str_print = "\033[31m" + q->name + "\033[0m" + "  \n";
                    myPrint(str_print.c_str());
                    str_print.clear();
                }
            } else {
                //文件
                //cout << q->name << "  " << q->FileSize << endl;
                str_print = q->name + "  " + to_string(q->FileSize) + "\n";
                myPrint(str_print.c_str());
                str_print.clear();
            }
        }
        //cout << endl;
        myPrint("\n");
        //进行递归
        for (int i = 0; i < leng; i++) {
            if (p->next[i]->isval == true) printLS_L(p->next[i]);
        }


    }
}

void printLSWithPath(Node *root, string p, int &exist, bool hasL) {
    if (p.compare(root->path) == 0) {  //路径完全相同，说明就是该节点
        //查找到
        if (root->isfile) {  //如果是文件，无法打开
            exist = 2;
            return;
        } else {
            exist = 1;
            if (hasL) {
                printLS_L(root);
            } else {
                printLS(root);
            }
        }
        return;
    }
    if (p.length() <= root->path.length()) {  //出现该情况直接出去
        return;
    }
    string temp = p.substr(0, root->path.length());   //截取输入的path的部分字符串，和当前节点的path比较，如果相同，说明目标在该节点下级
    if (temp.compare(root->path) == 0) {
        //路径部分匹配
        for (Node *q : root->next) {
            printLSWithPath(q, p, exist, hasL);
        }
    }
}

void getContent(FILE *fat12, int startClus, Node *son) {//获取文件内容
    int dataBase = BytsPerSec * (RsvdSecCnt + FATSz * NumFATs + (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec);
    int currentClus = startClus;
    int value = 0;        //这里用value来进行不同簇的读取（超过512字节）
    char *p = son->content;
    if (startClus == 0) {
        return;
    }
    while (value < 0xFF8) {
        value = getFATValue(fat12, currentClus);//获取下一个簇
        if (value == 0xFF7
                ) {
            myPrint("坏簇，读取失败!\n");
            break;
        }
        char *str = (char *) malloc(SecPerClus * BytsPerSec);    //暂存从簇中读出的数据
        char *content = str;
        int startByte = dataBase + (currentClus - 2) * SecPerClus * BytsPerSec;
        int check;
        check = fseek(fat12, startByte, SEEK_SET);
        if (check == -1)
            myPrint("fseek in printChildren failed!");

        check = fread(content, 1, SecPerClus * BytsPerSec, fat12);//提取数据
        if (check != SecPerClus * BytsPerSec)
            myPrint("fread in printChildren failed!");

        int count = SecPerClus * BytsPerSec;
        int loop = 0;
        for (int i = 0; i < count; i++) {//读取赋值
            *p = content[i];
            p++;
        }
        free(str);
        currentClus = value;
    }
}

void printCat(Node *root, string p, int &exist) {
    if (p.compare(root->path) == 0) {
        //查找到
        if (root->isfile) {
            exist = 1;
            if (root->content[0] != 0) {
                //cout << root->content << endl;
                myPrint(root->content);
                myPrint("\n");
            }
            return;
        } else {
            exist = 2;
            return;
        }
    }
    if (p.length() <= root->path.length()) {
        return;
    }
    string temp = p.substr(0, root->path.length());
    if (temp.compare(root->path) == 0) {
        //路径部分匹配
        for (Node *q : root->next) {
            printCat(q, p, exist);
        }
    }
}

void myPrint(const char *p) {
    asm_print(p, strlen(p));
}


void split(const string &s, vector <string> &sv, const char flag) {
    sv.clear();
    istringstream iss(s);
    string temp;
    while (getline(iss, temp, flag)) {
        sv.push_back(temp);
    }
    return;
}

void pathDeal(string &s) {
    if (s[0] != '/') {
        s = "/" + s;
    }
    if (s[s.length() - 1] != '/') {
        s += '/';
    }
}

//server
#include <windows.h>
#include <iostream>
#include <bitset>
#include <stdio.h>
#include <string>
#include <fstream>
#include <ctime>

using namespace std;

#define BUF_SIZE 1024
char buf_msg[BUF_SIZE];
bitset<48> subKey[16];
bitset<64> Key(0x0);
bitset<64> IV(0x0);

int IP[] = { 58, 50, 42, 34, 26, 18, 10, 2,
             60, 52, 44, 36, 28, 20, 12, 4,
             62, 54, 46, 38, 30, 22, 14, 6,
             64, 56, 48, 40, 32, 24, 16, 8,
             57, 49, 41, 33, 25, 17, 9,  1,
             59, 51, 43, 35, 27, 19, 11, 3,
             61, 53, 45, 37, 29, 21, 13, 5,
             63, 55, 47, 39, 31, 23, 15, 7 };

// 将64位密钥变换成56位
int PC_1[] = { 57, 49, 41, 33, 25, 17,9,
               1, 58, 50, 42, 34, 26, 18,
               10, 2, 59, 51, 43, 35, 27,
               19, 11, 3, 60, 52, 44, 36,
               63, 55, 47, 39, 31, 23, 15,
               7, 62, 54, 46, 38, 30, 22,
               14, 6, 61, 53, 45, 37, 29,
               21, 13, 5, 28, 20, 12, 4 };

// 将56位密钥压缩成48位子密钥
int PC_2[] = { 14, 17, 11, 24, 1, 5,
               3, 28, 15, 6, 21, 10,
               23, 19, 12, 4, 26, 8,
               16, 7, 27, 20, 13, 2,
               41, 52, 31, 37, 47, 55,
               30, 40, 51, 45, 33, 48,
               44, 49, 39, 56, 34, 53,
               46, 42, 50, 36, 29, 32 };

// 每轮左移的位数
int ShiftStep[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

//函数F的扩展置换表
int E[] = { 32, 1, 2, 3, 4, 5,
            4, 5, 6, 7, 8, 9,
            8, 9, 10, 11, 12, 13,
            12, 13, 14, 15, 16, 17,
            16, 17, 18, 19, 20, 21,
            20, 21, 22, 23, 24, 25,
            24, 25, 26, 27, 28, 29,
            28, 29, 30, 31, 32, 1 };

// 8个S盒，每个S盒将6位压缩为4位
int S_BOX[8][4][16] = {
        {
                { 14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7 },
                { 0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8 },
                { 4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0 },
                { 15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13 }
        },
        {
                { 15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10 },
                { 3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5 },
                { 0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15 },
                { 13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9 }
        },
        {
                { 10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8 },
                { 13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1 },
                { 13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7 },
                { 1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 }
        },
        {
                { 7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15 },
                { 13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9 },
                { 10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4 },
                { 3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14 }
        },
        {
                { 2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9 },
                { 14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6 },
                { 4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14 },
                { 11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3 }
        },
        {
                { 12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11 },
                { 10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8 },
                { 9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6 },
                { 4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 }
        },
        {
                { 4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
                { 13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
                { 1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
                { 6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
        },
        {
                { 13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7 },
                { 1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2 },
                { 7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8 },
                { 2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11 }
        }
};

// P置换
int P[] = { 16, 7, 20, 21,
            29, 12, 28, 17,
            1, 15, 23, 26,
            5, 18, 31, 10,
            2, 8, 24, 14,
            32, 27, 3, 9,
            19, 13, 30, 6,
            22, 11, 4, 25 };

// 尾置换 IP-1
int IP_1[] = { 40, 8, 48, 16, 56, 24, 64, 32,
               39, 7, 47, 15, 55, 23, 63, 31,
               38, 6, 46, 14, 54, 22, 62, 30,
               37, 5, 45, 13, 53, 21, 61, 29,
               36, 4, 44, 12, 52, 20, 60, 28,
               35, 3, 43, 11, 51, 19, 59, 27,
               34, 2, 42, 10, 50, 18, 58, 26,
               33, 1, 41, 9, 49, 17, 57, 25 };

bitset<32> F ( bitset<32> R, bitset<48> K ) {
    bitset<48> Rplus;
    bitset<32> Fout;
    for(int i=0; i<48; i++)
    {
        Rplus[i] = R[32-E[47-i]];
    }
    Rplus = Rplus ^ K;
    for(int i=0, x=0; i<48; i+=6, x+=4)
    {
        int RowNum = Rplus[47-i]*2 + Rplus[47-i-5];
        int ColNum = Rplus[47-i-1]*8 + Rplus[47-i-2]*4 + Rplus[47-i-3]*2 + Rplus[47-i-4];
        int Num = S_BOX[i/6][RowNum][ColNum];
        bitset<4> Sout(Num);
        for(int j=0; j<4; j++)
        {
            Fout[31-x-j] = Sout[4-j];
        }
    }
    bitset<32> tmp = Fout;
    for(int i=0; i<32; i++)
    {
        Fout[i] = tmp[32-P[31-i]];
    }
    return Fout;
}

bitset<28> LeftShift (bitset<28> K, int step) {
    for(int i=27; i>1; i--) {
        if(i-step >= 0)
        {
            K[i] = K[i-step];
        }
        else
        {
            K[i] = K[i-step+28];
        }
    }
    return K;
}

void GenerateSubKeys () {
    bitset<56> ActualKey;
    bitset<28> High, Low;
    bitset<56> RoundKey;
    bitset<48> sKey;
    for(int i=0; i<56; i++)
    {
        ActualKey[i] = Key[64-PC_1[55-i]];
    }
    for(int r=0; r<16; r++)
    {
        for(int p=0; p<28; p++)
        {
            High[p] = ActualKey[p+28];
            Low[p] = ActualKey[p];
        }
        High = LeftShift(High, ShiftStep[r]);
        Low = LeftShift(Low, ShiftStep[r]);
        for(int p=0; p<28; p++)
        {
            RoundKey[p] = Low[p];
            RoundKey[p+28] = High[p];
        }
        for(int i=0; i<48; i++)
        {
            sKey[i] = RoundKey[56-PC_2[47-i]];
        }
        subKey[r] = sKey;
    }
}

bitset<64> char2bit (const char a[8]) {
    bitset<64> text;
    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            text[8*i+j] = ( a[i] >> j ) & 1;
        }
    }
    return text;
}

string bit2string (const bitset<64> Text) {
    string s = "12345678";
    for(int i=0; i<64; i+=8)
    {
        s[i/8] = Text[i] + Text[i+1]*2 + Text[i+2]* 4 + Text[i+3]*8 +
                   Text[i+4]*16 + Text[i+5]*32 + Text[i+6]*64 + Text[i+7]*128;
    }
    return s;
}

bitset<64> DESencrypt (bitset<64>& PlainText) {
    bitset<64> CurrentBit, CipherText;
    bitset<32> High, Low, tmp;
    for(int i=0; i<64; i++)
    {
        CurrentBit[i] = PlainText[64-IP[63-i]];
    }
    for(int p=0; p<32; p++)
    {
        High[p] = CurrentBit[p+32];
        Low[p] = CurrentBit[p];
    }

    for(int i=0; i<7; i++)
    {
        tmp = Low;
        Low = F(Low, subKey[i]) ^ High;
        High = tmp;
    }
    High = F(Low, subKey[7]) ^ High;

    for(int p=0; p<32; p++)
    {
        CipherText[p] = Low[p];
        CipherText[p+32] = High[p];
    }
    CurrentBit = CipherText;
    for(int i=0; i<64; i++)
    {
        CipherText[i] = CurrentBit[64-IP_1[63-i]];
    }
    return CipherText;
}

bitset<64> DESdecrypt (bitset<64>& CipherText) {
    bitset<64> CurrentBit, PlainText;
    bitset<32> High, Low, tmp;
    for(int i=0; i<64; i++)
    {
        CurrentBit[i] = CipherText[64-IP[63-i]];
    }
    for(int p=0; p<32; p++)
    {
        High[p] = CurrentBit[p+32];
        Low[p] = CurrentBit[p];
    }

    for(int i=0; i<7; i++)
    {
        tmp = Low;
        Low = F(Low, subKey[7-i]) ^ High;
        High = tmp;
    }
    High = F(Low, subKey[0]) ^ High;

    for(int p=0; p<32; p++)
    {
        PlainText[p] = Low[p];
        PlainText[p+32] = High[p];
    }
    CurrentBit = PlainText;
    for(int i=0; i<64; i++)
    {
        PlainText[i] = CurrentBit[64-IP_1[63-i]];
    }
    return PlainText;
}

void CBCencrypt (string& PlainText) {
    bitset<64> PreviousSeg;
    bitset<64> CurrentSeg;
    int sz = PlainText.size();
    int num = sz % 8;
    if(num)
    {
        for(int i=0; i<8-num; i++)
        {
            PlainText += " ";
        }
    }
    for(int i=0; i<PlainText.size(); i+=8)
    {
        CurrentSeg = char2bit(PlainText.substr(i, 8).c_str());
        if(i==0)
        {
            CurrentSeg = CurrentSeg ^ IV;
        }
        else
        {
            CurrentSeg = CurrentSeg ^ PreviousSeg;
        }
        PreviousSeg = DESencrypt(CurrentSeg);
        cout << PreviousSeg;
    }
}

void CBCdecrypt () {
    bitset<64> CurrentSeg, PreviousSeg, tmp;
    string OutPut;
    PreviousSeg = IV;
    while(cin >> CurrentSeg)
    {
        tmp = CurrentSeg;
        CurrentSeg = DESdecrypt(CurrentSeg);
        CurrentSeg = CurrentSeg ^ PreviousSeg;
        PreviousSeg = tmp;
        OutPut = bit2string(CurrentSeg);
        cout << OutPut;
    }
}

int Fetch(string type, DWORD * num_rcv)
{
    if(type != "Key" && type != "IV")
    {
        cerr << "Invalid type required!\n";
        return 1;
    }
    HANDLE h_pipe;
    h_pipe = ::CreateNamedPipe("\\\\.\\pipe\\Innerpipe", PIPE_ACCESS_INBOUND, PIPE_READMODE_BYTE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUF_SIZE, BUF_SIZE, 0, nullptr);
    if (h_pipe == INVALID_HANDLE_VALUE)
    {
        if(type == "Key")
            cerr << "Failed to create named pipe for Key translation! Error code: " << ::GetLastError() << "\n";
        else
            cerr << "Failed to create named pipe for IV translation! Error code: " << ::GetLastError() << "\n";
        return 1;
    }
    else
    {
        if(type == "Key")
            cout << "Innerpipe for Key translation created successfully\n";
        else
            cout << "Innerpipe for IV translation created successfully\n";
    }
    //等待命名管道客户端连接
    if (::ConnectNamedPipe(h_pipe, nullptr))
    {
        cout << "UserB connected\n";
        memset(buf_msg, 0, BUF_SIZE);
        //读取数据
        if (!(::ReadFile(h_pipe, buf_msg, BUF_SIZE, num_rcv, nullptr)))
        {
            if(type == "Key")
                cerr << "Failed to receive Key! Error code: " << ::GetLastError() << "\n";
            else
                cerr << "Failed to receive IV! Error code: " << ::GetLastError() << "\n";
            ::CloseHandle(h_pipe);
            return 1;
        }
        if(type == "Key")
            cout << "Recieved Key from UserB\n";
        else
            cout << "Recieved IV from UserB\n";
    }
    ::CloseHandle(h_pipe);
    return 0;
}

int GetDESMessage()
{
    DWORD Knum_rcv = 0, IVnum_rcv = 0;
    int TranslationK = Fetch("Key", &Knum_rcv);
    if(TranslationK)
        return 1;
    cout << "DES key is " << buf_msg << endl;
    for(int pos = Knum_rcv-1, idx_bitset = 0; pos > 1; pos --, idx_bitset += 4)
    {
        int num = (buf_msg[pos]-48 < 10) ?  buf_msg[pos] - 48 : buf_msg[pos] - 97 + 10;
        Key[idx_bitset] = num & 0x1;
        Key[idx_bitset + 1] = (num >> 1) & 0x1;
        Key[idx_bitset + 2] = (num >> 2) & 0x1;
        Key[idx_bitset + 3] = (num >> 3) & 0x1;
    }

    
    int TranslationIV = Fetch("IV", &IVnum_rcv);
    if(TranslationIV)
        return 1;
    cout << "DES IV is " << buf_msg << endl;
    for(int pos = Knum_rcv-1, idx_bitset = 0; pos > 1; pos --, idx_bitset += 4)
    {
        int num = (buf_msg[pos]-48 < 10) ?  buf_msg[pos] - 48 : buf_msg[pos] - 97 + 10;
        IV[idx_bitset] = num & 0x1;
        IV[idx_bitset + 1] = (num >> 1) & 0x1;
        IV[idx_bitset + 2] = (num >> 2) & 0x1;
        IV[idx_bitset + 3] = (num >> 3) & 0x1;
    }
    return 0;
}

void DoDES()
{
    GenerateSubKeys();
    cout << "\n\n---This is a DES_RSA demo by 18307130071\n";
    cout << "Press 1 to encrypt.\n";
    cout << "Press 2 to decrypt.\n";
    int x;
    cin >> x;
    if(x==1)
    {
        cout << "Choose texts you wan to encrypt from the following files..\n\n"
             << "Press 1 for Text1.txt: Donald J. Trump is the 45th President...\n"
             << "Press 2 for Text2.txt: Just in case you were about to feel an...\n"
             << "Press 3 for Text3.txt: Play FIFA 21 by the end of October 9...\n";
        string y;
        cin >> y;
        if(y!="1"&&y!="2"&&y!="3")
        {
            cerr << "Invalid input!\n";
            return;
        }
        string PlainText;
        ifstream fin("Text" + y + ".txt");
        ofstream fout("Cipher.txt");
        streambuf *cinbackup;
        streambuf *coutbackup;
        cinbackup = cin.rdbuf(fin.rdbuf());
        coutbackup = cout.rdbuf(fout.rdbuf());
        getline(cin, PlainText);
        CBCencrypt(PlainText);
        cin.rdbuf(cinbackup);
        cout.rdbuf(coutbackup);
        fin.close();
        fout.close();
        cout << "\n\n...\nCongratulations, encryption succeeded!\n"
                "Look into file Cipher.txt for your cipher text.\n";
    }
    else
    {
        cout << "This will start to decrypt from file Cipher.txt.\n";
        ifstream fin("Cipher.txt");
        ofstream fout("Plain.txt");
        streambuf *cinbackup;
        streambuf *coutbackup;
        cinbackup = cin.rdbuf(fin.rdbuf());
        coutbackup = cout.rdbuf(fout.rdbuf());
        CBCdecrypt();
        cin.rdbuf(cinbackup);
        cout.rdbuf(coutbackup);
        fin.close();
        fout.close();

        cout << "\n\n...\nCongratulations, decryption succeeded!\n"
                "Look into file Plain.txt for your plain text.\n";
    }
}

int main()
{
    int msg = GetDESMessage();
    if(msg == 1)
        return 1;
    DoDES();
    return 0;
}
/**
 * use 'g++ -fexec-charset=GBK -std=c++0x -O3 -g -o test test.cpp' to compile the source
 */

#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>
#include "board.h"
#include "bitboard.h"
#include <cstdio>
#include <cstring>
#include<time.h>

using namespace std;
/**
 * 0x4312752186532731ull
 * +------------------------+
 * |     2     8   128     4|
 * |     8    32    64   256|
 * |     2     4    32   128|
 * |     4     2     8    16|
 * +------------------------+
 *
 */
void print(int arr[4][4]){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            cout.width(6);
            cout << arr[i][j];
        }
        cout << endl;
    }
}

void print_row(int arr[4]){
    for(int j = 0; j < 4; j++){
        cout.width(6);
        cout << arr[j];
    }
    cout << endl;
}

uint32_t cal_fetch(int arr[4]){
    uint32_t result = 0;
    for(int i = 3; i >= 0; i--){
        result *= 32;
        result += arr[i];
    }
    return result;
}

int main(int argc, const char* argv[]) {
    bitboard b;
    bitboard temp;
    // cout << "测试空的盘面" << endl;
    // b = bitboard(0, 0);
    // cout << b;
    // cin.ignore();

    // cout << "31 21 的盘面" << endl;
    // b = bitboard(0, 0b1010111111);
    // // cout << b._left << endl;
    // // cout << b._right << endl;
    // // cout << b.at(0) << endl;
    // // cout << b.at(1) << endl;
    // // cout << b.at(14) << endl;
    // // cout << b.at(15) << endl;
    // cout << b;
    // cin.ignore();

    // uint16_t a = 10;
    // for(int i = 0; i < 64; i++){
    //     cout << i << "\t" << (a >> i) << endl;
    // }

    int arr[4][4] = {
        {31, 21, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 31, 31, 31},
    };
    print(arr);
    b = bitboard(0xfffe, 0b1010111111);
    cout << b;
    cin.ignore();

    // cout << "测试右移操作" << endl;
    // for(int i = 0; i < 16; i++){
    //     cout << (b >> (i*5));
    //     cin.ignore();
    // }
    
    // cout << "测试 >>= 操作" << endl;
    // temp = b;
    // for(int i = 0; i < 16; i++){
    //     cout << temp;
    //     temp >>= 5;
    //     cin.ignore();
    // }

    // cout << "测试左移操作" << endl;
    // for(int i = 0; i < 16; i++){
    //     cout << (b << (i*5));
    //     cin.ignore();
    // }

    // cout << "测试 <<= 操作" << endl;
    // temp = b;
    // for(int i = 0; i < 16; i++){
    //     cout << temp;
    //     temp <<= 5;
    //     cin.ignore();
    // }

    // arr[0][3] = 4;
    // arr[1][2] = 1;
    // arr[1][0] = 7;
    // arr[2][1] = 21;
    // arr[3][2] = 10;
    // b.set(3, 4);
    // b.set(6, 1);
    // b.set(4, 7);
    // b.set(9, 21);
    // b.set(14, 10);
    // cout << "测试b.set" << endl;
    // print(arr);
    // cout << b;
    // cin.ignore();

    
    // cout << "测试fetch" << endl;
    // for(int i = 0; i < 4; i++){
    //     // print_row(arr[i]);
    //     cout << b.fetch(i) << "\t" << cal_fetch(arr[i]) << endl;
    //     cout << endl;
    // }
    // cin.ignore();

    cout << "测试place" << endl;
    int arr1[4][4] = {
        {1, 3, 7, 2},
        {3, 5, 6, 8},
        {1, 2, 5, 7},
        {2, 1, 3, 4}
    };
    for(int i = 0; i < 4; i++){
        b.place(i, cal_fetch(arr1[i]));
    }
    print(arr1);
    cout << b;
    cin.ignore();

    cout << "测试transpose" << endl;
    print(arr1);
    temp = b;
    temp.transpose();
    cout << temp;
    cin.ignore();

    cout << "测试水平翻转 reflect_horizontal" << endl;
    print(arr1);
    temp = b;
    temp.reflect_horizontal();
    cout << temp;
    cin.ignore();

    cout << "测试竖直翻转 reflect_vertical" << endl;
    print(arr1);
    temp = b;
    temp.reflect_vertical();
    cout << temp;
    cin.ignore();
}

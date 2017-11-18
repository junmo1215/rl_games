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

int main(int argc, const char* argv[]) {
    bitboard b;
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

    int arr[4][4] = {
        {31, 21, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 31, 31, 31},
    };
    // print(arr);
    b = bitboard(0xfffe, 0);
    for(int i = 0; i < 16; i++)
        cout << i << "\t" << b.at(i) << endl;
    cout << b.at(6) << endl;
    // cout << b._left << endl;
    // cout << b._right << endl;
    // cout << b.at(0) << endl;
    // cout << b.at(1) << endl;
    // cout << b.at(14) << endl;
    // cout << b.at(15) << endl;
    // cout << b;
}

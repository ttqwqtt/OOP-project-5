#include "Count.h"

// 宣告全域的靜態變數，確保不論在哪個地方呼叫 Count，分數都會累加在一起
int Count::chooseA = 0;
int Count::chooseB = 0;
int Count::chooseC = 0;
int Count::chooseD = 0;
int Count::chooseE = 0;
int Count::i = 0; // 記錄目前總共進行了幾次選擇

Count::Count() {
    choose = 0;
}

// 取得各個選項的分數
int Count::getA() { return chooseA; }
int Count::getB() { return chooseB; }
int Count::getC() { return chooseC; }
int Count::getD() { return chooseD; }
int Count::getE() { return chooseE; }
int Count::getI() { return i; } // 取得回合數

// 記錄玩家選擇：依據傳入的選項 x (1~5)，為對應的計數器 + 1
void Count::cho(int x) {
    if (x == 1) { i++; chooseA++; }
    else if (x == 2) { i++; chooseB++; }
    else if (x == 3) { i++; chooseC++; }
    else if (x == 4) { i++; chooseD++; }
    else if (x == 5) { i++; chooseE++; }
}

// 終極結算裁判：根據玩家各個選項累積的次數，判定該去哪個結局
int Count::getTheEnd() {
    // 【極端結局】如果玩家對某個選項情有獨鍾，連選 5 次
    if (chooseA == 5) return 0;
    else if (chooseB == 5) return 1;
    else if (chooseC == 5) return 2;
    else if (chooseD == 5) return 3;
    else if (chooseE == 5) return 4;

    // 【普通結局】如果某個選項剛好選了過半數的 3 次
    else if (chooseA >= 3 && chooseA<5) return 5;
    else if (chooseB >= 3 && chooseB < 5) return 6;
    else if (chooseC >= 3 && chooseC < 5) return 7;
    else if (chooseD >= 3 && chooseD < 5) return 8;
    else if (chooseE >= 3 && chooseE < 5) return 9;

    // 【混合結局】如果玩家東點西點，都沒有過半的明顯偏好
    else return 10;
}

// 重置計分板：當遊戲通關回到主選單，一切從零開始
void Count::reset() {
    chooseA = 0;
    chooseB = 0;
    chooseC = 0;
    chooseD = 0;
    chooseE = 0;
    i = 0;
}

// 讀檔專用：直接強制覆蓋計分板上的所有數字 (讓讀檔時能還原計分)
void Count::setAll(int a, int b, int c, int d, int e, int round) {
    chooseA = a;
    chooseB = b;
    chooseC = c;
    chooseD = d;
    chooseE = e;
    i = round;
}
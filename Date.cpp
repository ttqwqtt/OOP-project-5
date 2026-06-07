#define _CRT_SECURE_NO_WARNINGS
#include "Date.h"
#include <fstream>
#include <ctime>

using namespace std;

Date::Date() {}

// 存檔動作：將目前遊戲所有狀態寫入 txt 檔案
bool Date::saveProgress(int slotID, int state, int lineIndex, Count& count, string& timeStr) {
    // 根據格子編號，決定檔名 (例如 saves/savegame_1.txt)
    string filename = "saves/savegame_" + to_string(slotID) + ".txt";
    ofstream out(filename); // 開啟檔案準備寫入
    if (!out.is_open()) return false;

    // --- 取得電腦當前時間 ---
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    char timeBuf[100];
    strftime(timeBuf, sizeof(timeBuf), "%Y/%m/%d %H:%M:%S", &ltm); // 格式化為 2026/06/06 12:00:00
    timeStr = timeBuf; // 讓主程式知道存檔時間

    // --- 把所有資料依序寫入文字檔 ---
    out << timeBuf << "\n";      // 第 1 行：存檔時間
    out << state << "\n";        // 第 2 行：當前場景狀態 (STORY, ENDING...)
    out << lineIndex << "\n";    // 第 3 行：讀到第幾句話
    // 第 4 行：把玩家所有選項的計數器 (A,B,C,D,E) 和回合數 (i) 全部記下來
    out << count.getA() << " " << count.getB() << " "
        << count.getC() << " " << count.getD() << " "
        << count.getE() << " " << count.getI() << "\n";

    out.close();
    return true; // 存檔成功
}

// 讀檔動作：從 txt 檔案倒出所有遊戲狀態，還原遊戲進度
bool Date::loadProgress(int slotID, int& state, int& lineIndex, Count& count, string& timeStr) {
    string filename = "saves/savegame_" + to_string(slotID) + ".txt";
    ifstream in(filename); // 開啟檔案準備讀取
    if (!in.is_open()) return false;

    getline(in, timeStr); // 第 1 行：讀取時間

    int a, b, c, d, e, round;
    // 一次讀取後面的所有變數：狀態、對話進度、選項分數
    if (in >> state >> lineIndex >> a >> b >> c >> d >> e >> round) {
        count.setAll(a, b, c, d, e, round); // 強制把分數寫回計分系統
        in.close();
        return true; // 讀檔成功
    }

    in.close();
    return false;
}

// 只讀取該格子的「存檔時間」，用來顯示在存讀檔選單畫面上
bool Date::getSlotInfo(int slotID, string& timeStr) {
    string filename = "saves/savegame_" + to_string(slotID) + ".txt";
    ifstream in(filename);
    if (!in.is_open()) return false;

    getline(in, timeStr); // 只讀取第一行的時間
    in.close();
    return true;
}
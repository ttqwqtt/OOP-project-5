#ifndef DATE_H
#define DATE_H
#include <string>
#include "Count.h"

// ==========================================
// 存檔與時間管理員類別
// ==========================================
class Date {
public:
    Date();

    // 儲存遊戲進度 (新增 slotID 參數)
    bool saveProgress(int slotID, int state, int lineIndex, Count& count, std::string& timeStr);

    // 讀取遊戲進度 (新增 slotID 參數)
    bool loadProgress(int slotID, int& state, int& lineIndex, Count& count, std::string& timeStr);

    // 取得該欄位是否已有存檔，並回傳時間 (供選單顯示使用)
    bool getSlotInfo(int slotID, std::string& timeStr);
};
#endif
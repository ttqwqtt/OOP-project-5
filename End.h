#ifndef END_H
#define END_H
#include <iostream>
#include <string>
#include <vector>
#include "Count.h"

using namespace std;

// ==========================================
// 結局劇本讀取管理員
// ==========================================
class End {
private:
    Count choose;
public:
    End();
    void setchoose(Count choose);
    Count getChoose() const;

    // 給他一個結局 ID，他會幫你讀取對應的 txt 檔，並把每一行文字打包成 vector 回傳
    vector<string> getEndingLines(int endingID);
};
#endif
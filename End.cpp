#include "End.h"
#include <fstream> 

End::End() {}

void End::setchoose(Count choose) {
    this->choose = choose;
}

Count End::getChoose() const {
    return choose;
}

// 根據結算代碼，尋找並讀取對應的結局文字檔
vector<string> End::getEndingLines(int endingID) {
    vector<string> lines;
    string filename;

    // 判斷 1：如果觸發的是真結局 (代號 11)
    if (endingID == 11) {
        filename = "結局一覽/tureEnd.txt"; // 指定讀取真結局劇本
    }
    // 判斷 2：如果是一般結局 (代號 0~10)
    else {
        int s = endingID + 1; // 為了對齊檔名，把 0~10 轉換成 1~11
        filename = "結局一覽/";

        // 補零機制：如果是 1~9，前面補個 '0'，變成 "01", "02"... 等等
        if (s < 10) {
            filename += "0";
        }
        filename += to_string(s) + ".txt"; // 組合完整檔名，例如 "結局一覽/05.txt"
    }

    // 打開剛剛組合好名字的文字檔
    ifstream inputFile(filename);
    if (inputFile.is_open()) {
        string line;
        // 一行一行讀取文字，直到檔案結束
        while (getline(inputFile, line)) {
            if (!line.empty()) { // 略過空白行
                lines.push_back(line); // 把這句話塞進陣列裡
            }
        }
        inputFile.close();
    }
    else {
        // 如果找不到檔案的防呆機制，把錯誤訊息印在遊戲畫面上
        lines.push_back("讀取結局檔案失敗！找不到檔案：");
        lines.push_back(filename);
        lines.push_back("請確認 [結局一覽] 資料夾與你的程式在同一層目錄。");
    }

    // 把讀好的整包劇本文字交給主程式
    return lines;
}
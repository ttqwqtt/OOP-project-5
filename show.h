#ifndef SHOW_H
#define SHOW_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// ==========================================
// 結局一覽 (相簿) 管理員
// ==========================================
class Show {
private:
    // --- 圖片資源 ---
    cv::Mat galleryBg;   // 相簿背景圖 (showend.png)
    cv::Mat lockedImg;   // 未解鎖時顯示的圖片 (snowTV.jpg)
    cv::Mat unlockedImg; // 解鎖後顯示的縮圖 (1000005712.png/jpg)
    int width, height;   // 視窗寬高

    // --- 解鎖與分頁狀態 ---
    bool unlocked[12];   // 記錄 12 個結局的解鎖狀態 (true=解鎖, false=未解鎖)
    int currentPage;     // 記錄現在是第幾頁 (0 = 第1頁，顯示結局 1~6；1 = 第2頁，顯示結局 7~12)

    // --- 大圖預覽功能 ---
    bool isZoomed;       // 記錄目前是否正在「看大圖」模式
    int zoomedEndingID;  // 記錄目前正在放大看哪一個結局

    // --- 點擊感應區 (Rect) ---
    std::vector<cv::Rect> slots; // 畫面上 6 個格子的點擊感應區
    cv::Rect leftArrow;          // 左翻頁按鈕感應區
    cv::Rect rightArrow;         // 右翻頁按鈕感應區

public:
    Show();
    void init(int w, int h);             // 初始化相簿資源與格子座標
    void loadUnlockStatus();             // 從文字檔讀取解鎖紀錄
    void saveUnlockStatus();             // 把解鎖紀錄存入文字檔
    void unlockEnding(int endingID);     // 觸發某個結局並將它解鎖

    // 用來檢查前 11 個一般結局 (代號 0~10) 是否已經全部解鎖 (觸發真結局的條件)
    bool checkTrueEndCondition();

    void render(cv::Mat& frame);         // 把相簿畫面畫出來
    void handleMouseClick(int x, int y, int& gameState, int previousState); // 處理相簿裡的滑鼠點擊

    bool getIsZoomed() const { return isZoomed; } // 取得目前是否在看大圖
    void closeZoom() { isZoomed = false; }        // 關閉大圖模式
};

#endif
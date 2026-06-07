#pragma execution_character_set("utf-8") 

#include "show.h"
#include "TextRenderer.h"
#include <fstream>
#include <iostream>

using namespace cv;
using namespace std;

// 建構子：遊戲一開始，預設把 12 個結局都設為「未解鎖 (false)」
Show::Show() : width(0), height(0), currentPage(0), isZoomed(false), zoomedEndingID(0) {
    for (int i = 0; i < 12; ++i) {
        unlocked[i] = false;
    }
}

// 初始化函式：載入圖片，並設定格子跟翻頁按鈕的位置
void Show::init(int w, int h) {
    width = w;
    height = h;

    galleryBg = imread("picture/showend.png");
    lockedImg = imread("picture/snowTV.jpg");
    unlockedImg = imread("picture/1000005712.png");

    if (!galleryBg.empty()) resize(galleryBg, galleryBg, Size(width, height));

    // 設定畫面上 6 個九宮格的位置
    float boxW = width * 0.27;
    float boxH = height * 0.34;
    slots = {
        Rect(width * 0.085, height * 0.23, boxW, boxH),
        Rect(width * 0.365, height * 0.23, boxW, boxH),
        Rect(width * 0.662, height * 0.23, boxW, boxH),
        Rect(width * 0.085, height * 0.550, boxW, boxH),
        Rect(width * 0.375, height * 0.550, boxW, boxH),
        Rect(width * 0.662, height * 0.550, boxW, boxH)
    };

    // 設定左右翻頁按鈕的位置
    leftArrow = Rect(width * 0.04, height * 0.82, width * 0.10, height * 0.13);
    rightArrow = Rect(width * 0.86, height * 0.82, width * 0.10, height * 0.13);

    loadUnlockStatus(); // 遊戲啟動時，去讀取之前的解鎖紀錄
}

// 讀取解鎖進度：從 saves/achievements.txt 讀取 12 個 0 或 1 的數字
void Show::loadUnlockStatus() {
    ifstream in("saves/achievements.txt");
    if (in.is_open()) {
        for (int i = 0; i < 12; ++i) {
            int val = 0;
            if (in >> val) {
                unlocked[i] = (val == 1); // 如果讀到 1 就設為 true(解鎖)，0 為 false
            }
        }
        in.close();
    }
}

// 儲存解鎖進度：把 12 個 true/false 轉成 1/0 存入 txt 檔
void Show::saveUnlockStatus() {
    ofstream out("saves/achievements.txt");
    if (out.is_open()) {
        for (int i = 0; i < 12; ++i) {
            out << (unlocked[i] ? 1 : 0) << " ";
        }
        out.close();
    }
}

// 當玩家打出某個結局時呼叫此函式，解鎖該結局並存檔
void Show::unlockEnding(int endingID) {
    if (endingID >= 0 && endingID < 12) {
        unlocked[endingID] = true;
        saveUnlockStatus();
    }
}

// 檢查是否達成真結局條件：掃描前 11 個結局，如果全都 true，就回傳 true
bool Show::checkTrueEndCondition() {
    for (int i = 0; i < 11; ++i) {
        if (!unlocked[i]) return false; // 只要找到一個沒解鎖的，挑戰就失敗
    }
    return true; // 恭喜，全部解鎖！
}

// 繪製相簿畫面
void Show::render(Mat& frame) {
    // 狀況 A：如果玩家點擊了某張已解鎖的圖 (進入大圖模式)
    if (isZoomed && !unlockedImg.empty()) {
        resize(unlockedImg, frame, Size(width, height)); // 把圖放大到全螢幕
        TextRenderer::drawText(frame, "【 結局大圖查看 - 點選任意畫面返回 】", Point(width * 0.3, height * 0.95), Scalar(255, 255, 255), 24);
        return; // 畫完大圖就提早結束，不畫底下的九宮格了
    }

    // 狀況 B：一般相簿模式，先畫上相簿背景
    if (!galleryBg.empty()) {
        frame = galleryBg.clone();
    }
    else {
        frame = Mat::zeros(Size(width, height), CV_8UC3); // 沒背景就畫黑底
    }

    // 依序畫上 6 個格子
    for (int i = 0; i < 6; ++i) {
        int endingID = currentPage * 6 + i; // 根據目前頁數計算這是第幾號結局
        if (endingID >= 12) break; // 如果超過 12 個，就不畫了

        Rect rect = slots[i];
        // 稍微往內縮一點點，留下方空間寫字
        Rect thumbRect(rect.x + 12, rect.y + 12, rect.width - 24, rect.height - 55);

        // 如果這個結局已經解鎖
        if (unlocked[endingID]) {
            if (!unlockedImg.empty()) {
                Mat resized;
                resize(unlockedImg, resized, thumbRect.size());
                resized.copyTo(frame(thumbRect)); // 畫上解鎖的 CG 圖
            }

            // 如果這是第 12 個結局 (代號 11)，給它尊爵不凡的「真結局」三個字
            if (endingID == 11) {
                TextRenderer::drawText(frame, "真結局", Point(rect.x + rect.width * 0.38, rect.y + rect.height - 15), Scalar(0, 0, 0), 22);
            }
        }
        // 如果這個結局還沒解鎖
        else {
            if (!lockedImg.empty()) {
                Mat resized;
                resize(lockedImg, resized, thumbRect.size());
                resized.copyTo(frame(thumbRect)); // 畫上電視雪花圖
            }
        }
    }
}

// 處理相簿畫面內的滑鼠點擊
void Show::handleMouseClick(int x, int y, int& gameState, int previousState) {
    // 如果正在看大圖，點擊畫面任何地方都會關閉大圖
    if (isZoomed) {
        isZoomed = false;
        return;
    }

    // 檢查玩家有沒有點擊那 6 個格子
    for (size_t i = 0; i < slots.size(); ++i) {
        Rect rect = slots[i];
        Rect thumbRect(rect.x + 12, rect.y + 12, rect.width - 24, rect.height - 55);

        if (thumbRect.contains(Point(x, y))) {
            int endingID = currentPage * 6 + i;

            // 只有點到「已經解鎖」的格子，才會打開大圖
            if (endingID < 12 && unlocked[endingID]) {
                isZoomed = true;
                zoomedEndingID = endingID;
            }
            return; // 處理完格子點擊就結束
        }
    }

    // 檢查翻頁按鈕
    if (leftArrow.contains(Point(x, y))) {
        currentPage = 0; // 回第一頁
    }
    else if (rightArrow.contains(Point(x, y))) {
        currentPage = 1; // 去第二頁
    }
    // 點擊其他空白處，退回原本的畫面
    else {
        gameState = previousState;
    }
}
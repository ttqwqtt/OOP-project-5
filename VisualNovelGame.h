#ifndef VisualNovelGame_H
#define VisualNovelGame_H
#include <opencv2/opencv.hpp>
#include <vector>
#include "Count.h"
#include "End.h"
#include "Date.h"
#include "show.h" 

// 定義遊戲的所有狀態（場景），用來控制現在該顯示什麼畫面
enum GameState { MENU, STORY, SELECTION, FEEDBACK, ENDING, SAVELOAD, GALLERY };

class VisualNovelGame {
private:
    GameState state;            // 記錄目前的遊戲狀態
    bool isRunning;             // 遊戲是否正在執行（如果設為 false 遊戲就會關閉）
    int targetDelay;            // 控制畫面更新的延遲時間（用來控制幀率 FPS）
    std::string windowName;     // 視窗的標題名稱

    // 引入其他自訂的系統管理員
    Count gameCount;            // 負責計算選項分數
    End gameEnd;                // 負責管理腳本文字與結局讀取
    Date gameDate;              // 負責存讀檔的進度與時間管理
    Show gameShow;              // 負責結局相簿（Gallery）的管理

    std::string sysMessage;     // 用來顯示系統提示文字（例如："成功存入檔案"）
    int sysMessageTimer;        // 系統提示文字顯示在畫面上的倒數計時器
    std::vector<std::string> endingLines; // 暫存目前正在播放的結局文字陣列

    // --- 資源與佈景 (OpenCV 影像矩陣) ---
    cv::Mat menuImg, storyImg, selectionImg, saveMenuImg; // 各大場景的背景圖片
    cv::Mat trueEndSelectionImg; // 真結局專屬的選項背景圖 (TureEnd.jpg)
    cv::VideoCapture characterVideo; // 用來播放人魚背景的影片檔
    int width, height;           // 記錄遊戲視窗的寬度與高度

    // --- 互動區域 (UI) 按鈕變數 ---
    // 這些 Rect 就像是畫面上的「透明感應框」，負責捕捉滑鼠點擊
    cv::Rect startButton;          // 主選單的「開始遊戲」按鈕
    cv::Rect menuSaveLoadButton;   // 主選單的「存讀檔」按鈕
    cv::Rect menuGalleryButton;    // 主選單的「結局一覽」按鈕
    cv::Rect storySaveLoadButton;  // 故事進行中的「存讀檔」按鈕
    cv::Rect storyGalleryButton;   // 故事進行中的「結局相簿」按鈕
    cv::Rect storyReturnMenuButton;// 故事進行中的「回主選單」按鈕
    std::vector<cv::Rect> choiceButtons; // 存放 5 個一般選項的感應區
    cv::Rect trueEndButton;        // 真結局專屬的「放他走」隱藏按鈕感應區
    cv::Mat currentFrame;          // 暫存目前畫面的影像（用來當作存檔的縮圖）

    // --- 腳本與打字機效果變數 ---
    std::vector<std::string> storyLines; // 存放一般故事的所有台詞
    int currentLineIndex;                // 記錄目前故事唸到第幾行
    std::string fullText;                // 目前這句話的「完整文字」
    std::string currentText;             // 目前「已經打出來」的文字（打字機效果用）
    size_t textIndex;                    // 打字機效果的文字索引進度

    // --- 事件回饋專屬變數 ---
    cv::Mat eventImg; // 玩家按下選項後，顯示在畫面的對應事件插圖
    std::vector<std::vector<std::string>> feedbackTexts; // 存放 5 個選項對應的後續回饋對話

    // --- 存讀檔專屬 UI 變數 ---
    bool isSavingMode;                   // 記錄現在是「存檔模式」還是「讀檔模式」
    GameState previousState;             // 記錄進入存讀檔/相簿前，玩家原本在哪個畫面（方便按 ESC 退回去）
    cv::Mat currentScreenshot;           // 存檔時擷取的畫面縮圖
    std::vector<cv::Rect> saveSlots;     // 6 個存讀檔格子的點擊感應區
    std::vector<std::string> slotTimes;  // 6 個存讀檔格子的存檔時間字串
    std::vector<cv::Mat> slotThumbs;     // 6 個存讀檔格子的縮圖影像

public:
    VisualNovelGame(); // 建構子：初始化變數
    bool init();       // 初始化函式：載入所有資源
    void run();        // 遊戲主迴圈：讓遊戲持續運作

private:
    // 渲染函式：負責把對應狀態的圖片和UI畫到畫面上
    void renderMenu(cv::Mat& frame);
    void renderStory(cv::Mat& frame);
    void renderSelection(cv::Mat& frame);
    void renderFeedback(cv::Mat& frame);
    void renderEnding(cv::Mat& frame);
    void renderSaveLoad(cv::Mat& frame);

    // 工具函式
    void updateTypewriter(); // 處理文字一個字一個字浮現的效果
    void refreshSaveSlots(); // 刷新存讀檔畫面的縮圖和時間

    // 滑鼠事件控制
    static void onMouseWrapper(int event, int x, int y, int flags, void* userdata); // 讓 OpenCV 能呼叫的靜態中繼站
    void handleMouseClick(int event, int x, int y); // 實際處理滑鼠點擊邏輯的函式
};
#endif
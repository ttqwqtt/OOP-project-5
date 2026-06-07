// 引入我們自己寫的遊戲主標頭檔，裡面有 VisualNovelGame 類別的宣告
#include "VisualNovelGame.h"
// 引入負責把文字畫到 OpenCV 圖片上的文字渲染工具
#include "TextRenderer.h"
// 引入檔案串流庫，讓我們可以讀取 txt 檔或寫入存檔
#include <fstream>
// 引入標準輸入輸出庫，主要用於終端機的 cout 測試印出 (雖然這裡比較少用)
#include <iostream>
// 引入目錄控制庫，讓我們可以呼叫 _mkdir 來建立資料夾
#include <direct.h> 

// 這是一段編譯器指令 (Pragma)。告訴編譯器：「請幫我連結 Windows 的 winmm.lib 函式庫」。
// 因為我們要用 Windows 內建的功能來播音樂，這樣才不會跟 OpenCV 的功能打架。
#pragma comment(lib, "winmm.lib") 
// 宣告一個來自 C 語言底層的外部函式 mciSendStringA，它是 Windows 用來發送多媒體指令 (如播放 mp3) 的標準函式。
extern "C" __declspec(dllimport) unsigned long __stdcall mciSendStringA(const char* lpstrCommand, char* lpstrReturnString, unsigned int uReturnLength, void* hwndCallback);

// 告訴終端機 (Console) 在執行時使用 UTF-8 編碼，這樣如果有中文字才不會印出亂碼
#pragma execution_character_set("utf-8") 

// 省去每次都要打 cv:: 的麻煩，直接使用 OpenCV 命名空間
using namespace cv;
// 省去每次都要打 std:: 的麻煩，直接使用 C++ 標準函式庫命名空間
using namespace std;

// ==========================================
// 建構子 (Constructor)
// ==========================================
// 當我們在主程式 new 一個 VisualNovelGame 的時候，這段會第一個執行。
// 冒號 (:) 後面叫做「初始化列表」，比在括號裡面用等號賦值還要有效率。
VisualNovelGame::VisualNovelGame()
    : state(MENU),               // 遊戲一開機，預設狀態設定為 MENU (主選單)
    isRunning(true),             // 設定遊戲正在運行中，讓主迴圈可以啟動
    targetDelay(1000 / 30),      // 計算 30 FPS 的延遲時間 (大約每 33 毫秒更新一次畫面)
    windowName("My Visual Novel"), // 設定 OpenCV 視窗左上角的標題文字
    currentLineIndex(0),         // 對話進度歸零 (從第 0 句話開始唸)
    textIndex(0),                // 打字機特效的字元索引歸零
    sysMessageTimer(0),          // 系統提示文字的停留時間歸零
    isSavingMode(true) {         // 預設進入存讀檔畫面時是「存檔模式」
}

// ==========================================
// 初始化函式 (init)
// ==========================================
bool VisualNovelGame::init() {
    // 建立 saves (存檔) 與 music (音樂) 資料夾。如果資料夾已經存在，這行不會有副作用；
    // 如果不存在就會幫你建好，避免等等讀寫檔案時發生崩潰 (Crash)。_mkdir 是英文 "make directory"（建立資料夾） 的縮寫。
    _mkdir("saves");
    _mkdir("music");

    // --- 讀取主線劇本 ---
    ifstream inputFile("frist.txt"); // 打開名為 frist.txt 的檔案準備讀取
    if (inputFile.is_open()) {       // 檢查檔案有沒有成功打開 (防呆機制)
        string line;                 // 準備一個空字串，用來接每一行的文字
        while (getline(inputFile, line)) { // getline 會一行一行讀取，直到檔案結束
            if (!line.empty()) storyLines.push_back(line); // 如果這行不是空的，就把這句話塞進 storyLines 陣列的尾巴
        }
        inputFile.close();           // 讀完記得關閉檔案，釋放資源
    }

    // --- 讀取選項回饋劇本 ---
    feedbackTexts.resize(5);         // 我們有 5 個選項，所以把外部大陣列的大小調整為 5
    for (int i = 0; i < 5; ++i) {    // 跑一個 0 到 4 的迴圈
        // 利用 to_string 把數字轉成字串，組合成檔名 (例如 "options/051.txt")
        string filename = "options/05" + to_string(i + 1) + ".txt";
        ifstream fbFile(filename);   // 打開剛剛組合好名字的檔案
        if (fbFile.is_open()) {
            string line;
            while (getline(fbFile, line)) { // 一樣一行一行讀取
                if (!line.empty()) feedbackTexts[i].push_back(line); // 塞進對應選項的小陣列裡
            }
            fbFile.close();
        }
    }

    // --- 讀取圖片素材 ---
    // imread 是 OpenCV 讀取圖片的指令，會把圖片資料轉成 Mat 矩陣格式
    menuImg = imread("picture/cg.png");             // 讀取主選單背景
    storyImg = imread("picture/bg.jpg");            // 讀取故事背景
    selectionImg = imread("picture/2222.png");      // 讀取選項背景
    saveMenuImg = imread("picture/save.png");       // 讀取存檔介面背景
    trueEndSelectionImg = imread("picture/TureEnd.png"); // 讀取真結局專屬選項背景

    // 讀取動態影片 (VideoCapture 是 OpenCV 處理影片的類別)
    characterVideo.open("picture/漂漂人魚.mp4");

    // 以主選單圖片的寬高作為整個遊戲視窗的標準尺寸
    width = menuImg.cols;  // cols (行數) 代表影像的寬度 X
    height = menuImg.rows; // rows (列數) 代表影像的高度 Y

    // 把其他圖片都強制縮放 (resize) 成跟主選單一樣大，確保切換畫面時不會忽大忽小，resize(原本的圖片, 縮放後的圖片要存去哪, 指定的尺寸);
    resize(storyImg, storyImg, Size(width, height));
    resize(selectionImg, selectionImg, Size(width, height));
    resize(saveMenuImg, saveMenuImg, Size(width, height));
    resize(trueEndSelectionImg, trueEndSelectionImg, Size(width, height));

    // --- 設定 UI 感應區 (Rect = 矩形) ---
    // 利用寬高乘上比例 (例如 width * 0.13)，讓按鈕位置能適應不同解析度
    // 參數順序為：Rect(左上角 X, 左上角 Y, 矩形的寬, 矩形的高)
    startButton = Rect(width * 0.13, height * 0.515, width * 0.285, height * 0.098);
    menuSaveLoadButton = Rect(width * 0.13, height * 0.645, width * 0.285, height * 0.10);
    menuGalleryButton = Rect(width * 0.13, height * 0.78, width * 0.29, height * 0.10);

    // 故事畫面右下角的幾個小按鈕感應區
    storySaveLoadButton = Rect(width * 0.457, height * 0.911, width * 0.07, height * 0.05);
    storyGalleryButton = Rect(width * 0.535, height * 0.911, width * 0.07, height * 0.05);
    storyReturnMenuButton = Rect(width * 0.779, height * 0.907, width * 0.07, height * 0.05);

    // 一次把 5 個選項按鈕的範圍塞進陣列裡，方便以後用迴圈處理
    choiceButtons = {
        Rect(width * 0.515, height * 0.15, width * 0.44, height * 0.11),
        Rect(width * 0.515, height * 0.29, width * 0.44, height * 0.11),
        Rect(width * 0.505, height * 0.44, width * 0.45, height * 0.11),
        Rect(width * 0.515, height * 0.58, width * 0.44, height * 0.11),
        Rect(width * 0.515, height * 0.72, width * 0.44, height * 0.11)
    };

    // 真結局專屬隱藏按鈕
    trueEndButton = Rect(width * 0.52, height * 0.89, width * 0.52, height * 0.10);

    // 設定存讀檔 6 個格子的範圍
    float boxW = width * 0.27; // 算出格子的統一寬度
    float boxH = height * 0.32; // 算出格子的統一高度
    saveSlots = {
        Rect(width * 0.047, height * 0.24, boxW, boxH), // 第一排左
        Rect(width * 0.365, height * 0.24, boxW, boxH), // 第一排中
        Rect(width * 0.685, height * 0.24, boxW, boxH), // 第一排右
        Rect(width * 0.047, height * 0.59, boxW, boxH), // 第二排左
        Rect(width * 0.365, height * 0.59, boxW, boxH), // 第二排中
        Rect(width * 0.685, height * 0.59, boxW, boxH)  // 第二排右
    };

    // 把算好的視窗寬高交給相簿子系統，讓它也能去設定自己畫面的格子位置
    gameShow.init(width, height);

    // --- 建立視窗與綁定滑鼠 ---
    // 呼叫 OpenCV 的 namedWindow 建立出遊戲視窗，WINDOW_AUTOSIZE 代表視窗大小會跟隨圖片自動調整
    namedWindow(windowName, WINDOW_AUTOSIZE);
    // 把滑鼠事件綁定到這個視窗上。當滑鼠有動作時，會呼叫 onMouseWrapper 這個靜態函式，
    // 並把 this (目前這個遊戲物件的指標) 當作 userdata 傳進去，這樣靜態函式才知道是哪個遊戲被點了。
    setMouseCallback(windowName, onMouseWrapper, this);

    // --- 播放音樂 ---
    // 傳送指令給 Windows 底層：打開音樂檔並給它一個綽號叫 bgm
    mciSendStringA("open \"music/bgmusic.mp3\" alias bgm", 0, 0, 0);
    // 設定 bgm 的音量為 500 (範圍通常是 0~1000)
    mciSendStringA("setaudio bgm volume to 500", 0, 0, 0);
    // 讓 bgm 開始播放，並且加上 repeat 讓它無限循環
    mciSendStringA("play bgm repeat", 0, 0, 0);

    return true; // 初始化完成，回傳 true 告訴主程式可以開始了
}

// ==========================================
// 更新存讀檔畫面內容
// ==========================================
void VisualNovelGame::refreshSaveSlots() {
    slotTimes.assign(6, "");  // 先把時間陣列清空，塞入 6 個空字串
    slotThumbs.assign(6, Mat()); // 先把縮圖陣列清空，塞入 6 張空的圖片矩陣

    for (int i = 0; i < 6; ++i) { // 掃描 6 個存檔格
        // 去問 Date 系統：「第 i+1 個格子有沒有存檔？」，如果有，會把時間寫進 slotTimes[i] 裡
        if (gameDate.getSlotInfo(i + 1, slotTimes[i])) {
            // 試著讀取對應的縮圖檔 (例如 "saves/thumb_1.jpg")
            Mat img = imread("saves/thumb_" + to_string(i + 1) + ".jpg");
            if (!img.empty()) { // 如果圖片存在而且沒損壞
                // 根據我們剛剛定義好的格子大小 (saveSlots)，稍微縮小一點點 (減掉邊界留白)，把圖片塞進去
                resize(img, slotThumbs[i], Size(saveSlots[i].width - 20, saveSlots[i].height - 50));
            }
        }
    }
}

// ==========================================
// 遊戲主迴圈 (The Heartbeat)
// ==========================================
void VisualNovelGame::run() {
    while (isRunning) { // 只要 isRunning 還是 true，這個迴圈就會一直跑
        int64 startTick = getTickCount(); // 用 OpenCV 內建函式，記下這一幀開始運算的「時鐘滴答數」
        Mat displayImg; // 準備一張空白的畫布 (Mat)，這回合所有的東西都會畫在這上面

        // 根據目前的 state (遊戲狀態)，決定把哪種畫面畫到 displayImg 畫布上
        switch (state) {
        case MENU:      renderMenu(displayImg);      break;
        case STORY:     renderStory(displayImg);     break;
        case SELECTION: renderSelection(displayImg); break;
        case FEEDBACK:  renderFeedback(displayImg);  break;
        case ENDING:    renderEnding(displayImg);    break;
        case SAVELOAD:  renderSaveLoad(displayImg);  break;
        case GALLERY:   gameShow.render(displayImg); break;
        }

        // 把剛剛畫好那一瞬間的畫面，備份到 currentFrame 裡。
        // 用 clone() 是為了真正拷貝一份新的記憶體，而不是只傳參考。存檔時就會拿這張圖當縮圖。
        currentFrame = displayImg.clone();

        // 如果系統提示訊息的倒數計時器大於 0 (例如剛存完檔)
        if (sysMessageTimer > 0 && !displayImg.empty()) {
            // 在畫面的左上角 (15, 35) 畫出綠色的提示文字
            TextRenderer::drawText(displayImg, sysMessage, Point(15, 35), Scalar(150, 255, 150), 24);
            sysMessageTimer--; // 倒數計時器減 1
        }

        // 畫布都畫完了，如果有內容，就更新到視窗上讓玩家看到
        if (!displayImg.empty()) {
            imshow(windowName, displayImg);
        }

        // --- 控制畫面更新率 (FPS) ---
        int64 endTick = getTickCount(); // 記下這回合畫完的時鐘滴答數
        // 計算剛剛從 startTick 到 endTick 總共花了幾毫秒 (ms)
        double executeTime = (endTick - startTick) * 1000.0 / getTickFrequency();
        // 拿我們預期的延遲時間 (targetDelay，大約 33 毫秒) 減掉實際執行的時間
        int waitTime = targetDelay - (int)executeTime;
        // 如果執行得太慢，waitTime 會變成負的，這時強制休息 1 毫秒避免卡死
        if (waitTime < 1) waitTime = 1;

        // --- 鍵盤監聽 ---
        // waitKey 是 OpenCV 的函式，它會暫停程式執行 waitTime 毫秒，並回傳這段期間玩家按了哪個鍵
        int key = waitKey(waitTime);
        if (key == 27) { // 27 在 ASCII 碼裡面代表 ESC 鍵
            if (state == SAVELOAD) {
                state = previousState; // 如果在存檔畫面，按 ESC 就退回上一頁
            }
            else if (state == GALLERY) {
                if (gameShow.getIsZoomed()) gameShow.closeZoom(); // 如果在看大圖，就關閉大圖
                else state = previousState; // 如果在相簿主頁，就退回上一頁
            }
            else {
                isRunning = false; // 如果在其他地方按 ESC，就把迴圈開關關掉，準備退出遊戲
            }
        }
        else if (key == 's' || key == 'S') { // 如果按下鍵盤的 S 鍵
            // 只要不是在某些特殊畫面，就快速切換到「存檔模式」
            if (state != SAVELOAD && state != MENU && state != GALLERY) {
                previousState = state; // 記住現在在哪裡，以後才能回來
                isSavingMode = true;   // 設定為存檔模式
                state = SAVELOAD;      // 切換狀態
                currentScreenshot = displayImg.clone(); // 擷取當前畫面準備做成縮圖
                refreshSaveSlots();    // 刷新格子資訊
            }
        }
        else if (key == 'l' || key == 'L') { // 如果按下鍵盤的 L 鍵
            // 只要不是在特殊畫面，就快速切換到「讀檔模式」
            if (state != SAVELOAD && state != GALLERY) {
                previousState = state;
                isSavingMode = false;  // 設定為讀檔模式
                state = SAVELOAD;
                refreshSaveSlots();
            }
        }
    }

    // 當 isRunning 變成 false (玩家按了 ESC 退出)，迴圈結束，來到這裡
    mciSendStringA("close bgm", 0, 0, 0); // 傳送指令關閉音樂
    destroyAllWindows(); // 銷毀 OpenCV 的所有視窗，把記憶體還給作業系統
}

// ==========================================
// 以下是負責把對應狀態的圖片與文字畫上畫布的函式
// ==========================================

void VisualNovelGame::renderMenu(Mat& frame) {
    frame = menuImg.clone(); // 主選單最單純，直接把背景圖拷貝給畫布
}

void VisualNovelGame::renderStory(Mat& frame) {
    int targetLineIndex = 11; // 設定條件：對話進行到第 11 句 (索引值) 時切換影片背景
    // 如果達標，且人魚影片有成功開啟
    if (currentLineIndex >= targetLineIndex && characterVideo.isOpened()) {
        Mat videoFrame; // 準備一張放影片截圖的矩陣
        characterVideo >> videoFrame; // 從影片中抓取「下一幀」出來，塞給 videoFrame
        if (videoFrame.empty()) { // 如果抓不到 (代表影片播到最後一秒結束了)
            characterVideo.set(CAP_PROP_POS_FRAMES, 0); // 用 set 把影片進度條拉回第 0 幀 (從頭播放)
            characterVideo >> videoFrame; // 重新抓第一幀
        }
        // 如果影片解析度跟遊戲視窗不一樣大，就強制縮放
        if (videoFrame.cols != width || videoFrame.rows != height) {
            resize(videoFrame, videoFrame, Size(width, height));
        }
        frame = videoFrame.clone(); // 把這幀影片當作背景圖

        // --- 繪製半透明的對話框黑底 ---
        Rect textBox(width * 0.05, height * 0.65, width * 0.9, height * 0.25); // 設定框框位置
        Mat roi = frame(textBox); // 從畫布上「框出」這個區域 (ROI: Region of Interest)
        Mat whiteBlock(roi.size(), CV_8UC3, Scalar(255, 255, 255)); // 建立一張跟 ROI 一樣大的純白圖片
        // addWeighted 用來混合兩張圖片。這裡把白圖 (30%透明度) 和原本的背景 (70%透明度) 疊加在一起
        addWeighted(whiteBlock, 0.3, roi, 0.7, 0, roi);
    }
    else {
        // 條件沒達成，就畫普通的靜態背景圖
        frame = storyImg.clone();
    }

    updateTypewriter(); // 呼叫打字機邏輯，算出現在該顯示多少字了
    // 呼叫文字渲染器，把算好的 currentText 畫在指定座標上，黑色，字體大小 32
    TextRenderer::drawText(frame, currentText, Point(width * 0.1, height * 0.75), Scalar(0, 0, 0), 32);
}

void VisualNovelGame::renderSelection(Mat& frame) {
    // 去問相簿系統：真結局的條件都達成了嗎？如果達成了且專屬圖存在...
    if (gameShow.checkTrueEndCondition() && !trueEndSelectionImg.empty()) {
        frame = trueEndSelectionImg.clone(); // 用真結局的背景圖
    }
    else {
        frame = selectionImg.clone(); // 否則用一般選項的背景圖
    }
}

void VisualNovelGame::renderFeedback(Mat& frame) {
    // 背景選擇邏輯跟上面 Selection 一樣
    if (gameShow.checkTrueEndCondition() && !trueEndSelectionImg.empty()) {
        frame = trueEndSelectionImg.clone();
    }
    else {
        frame = selectionImg.clone();
    }

    // --- 讓背景整體變暗 (凸顯事件圖) ---
    Mat blackOverlay(frame.size(), frame.type(), Scalar(0, 0, 0)); // 建一張跟畫面一樣大的全黑圖片
    addWeighted(frame, 0.4, blackOverlay, 0.6, 0, frame); // 原背景保留 40%，黑色蓋上 60%

    // --- 畫上事件 CG 圖 ---
    if (!eventImg.empty()) { // 確保圖有正確讀進來
        int evW = width * 0.65; // 設定事件圖寬度
        int evH = height * 0.55; // 設定事件圖高度
        Mat resizedEvent;
        resize(eventImg, resizedEvent, Size(evW, evH)); // 縮放到指定大小
        Rect roiRect(width * 0.175, height * 0.05, evW, evH); // 算出它要在畫面的哪裡
        resizedEvent.copyTo(frame(roiRect)); // 把事件圖複製貼上到畫布的對應位置
    }

    // --- 畫上半透明對話框 ---
    Rect textBox(width * 0.15, height * 0.65, width * 0.7, height * 0.25);
    Mat roi = frame(textBox);
    // 這裡用 Scalar(210, 210, 210) 建立偏灰白色的底，並設定 80% 不透明度
    Mat whiteBlock(roi.size(), CV_8UC3, Scalar(210, 210, 210));
    addWeighted(whiteBlock, 0.8, roi, 0.2, 0, roi);

    updateTypewriter(); // 更新文字進度
    TextRenderer::drawText(frame, currentText, Point(width * 0.18, height * 0.72), Scalar(0, 0, 0), 28);
}

void VisualNovelGame::renderEnding(Mat& frame) {
    frame = storyImg.clone(); // 結局直接套用故事場景的背景圖
    updateTypewriter();
    TextRenderer::drawText(frame, currentText, Point(width * 0.1, height * 0.75), Scalar(0, 0, 0), 32);
}

void VisualNovelGame::renderSaveLoad(Mat& frame) {
    frame = saveMenuImg.clone(); // 畫上存讀檔背景
    // 用三元運算子 ( ? : ) 判斷，如果是存檔模式就顯示前者字串，反之顯示後者
    string modeTitle = isSavingMode ? "【 請選擇存檔位置 】" : "【 請選擇讀取檔案 】";
    // 畫出大大的標題跟左上角的提示
    TextRenderer::drawText(frame, modeTitle, Point(width * 0.36, height * 0.04), Scalar(50, 50, 50), 36);
    TextRenderer::drawText(frame, "(按 ESC 返回遊戲)", Point(20, 20), Scalar(50, 50, 50), 22);

    // 把 6 個格子的時間文字和縮圖貼上去
    for (size_t i = 0; i < 6; ++i) {
        if (!slotThumbs[i].empty()) { // 確定這個格子有縮圖才畫
            // 設定縮圖要貼的位置 (比格子的 x,y 再往內推 10 個像素，留邊框)
            Rect thumbRect(saveSlots[i].x + 10, saveSlots[i].y + 10, slotThumbs[i].cols, slotThumbs[i].rows);
            slotThumbs[i].copyTo(frame(thumbRect)); // 貼上縮圖
            // 在縮圖的下方畫出存檔的時間字串
            TextRenderer::drawText(frame, slotTimes[i], Point(saveSlots[i].x + 15, saveSlots[i].y + slotThumbs[i].rows + 20), Scalar(0, 0, 0), 20);
        }
    }
}

// ==========================================
// 打字機特效邏輯 (精準切割 UTF-8 字元)
// ==========================================
void VisualNovelGame::updateTypewriter() {
    // 如果目前打出來的字 (textIndex) 還沒到達整句話 (fullText) 的長度
    if (textIndex < fullText.length()) {
        // 抓出目前位置的第一個 Byte (位元組)
        unsigned char c = fullText[textIndex];
        int charBytes = 1; // 預設這個字元只佔 1 個 Byte (英文或符號)

        // 這是處理 UTF-8 編碼的核心邏輯，用位元遮罩 (&) 判斷它到底佔幾個 Byte
        if ((c & 0xE0) == 0xC0) charBytes = 2;      // 如果前綴是 110，代表佔 2 Bytes
        else if ((c & 0xF0) == 0xE0) charBytes = 3; // 如果前綴是 1110，代表佔 3 Bytes (大部分的中文字落在這裡)
        else if ((c & 0xF8) == 0xF0) charBytes = 4; // 如果前綴是 11110，代表佔 4 Bytes (罕見字或 Emoji)

        // 檢查如果把這些 Bytes 加上去，會不會超過字串總長度
        if (textIndex + charBytes <= fullText.length()) {
            // 用 append 函式，從 fullText 的 textIndex 位置開始，精準抓取 charBytes 個數量的資料，塞到 currentText 後面
            currentText.append(fullText, textIndex, charBytes);
            textIndex += charBytes; // 更新進度指標
        }
        else {
            // 防呆機制：如果算錯了超出長度，就把剩下的所有字串全部塞進去，並把指標設定到最後
            currentText += fullText.substr(textIndex);
            textIndex = fullText.length();
        }
    }
}//透過檢查 UTF-8 編碼，精確計算中文字的 Byte 長度，確保打字機一個字一個字跳出來時不會崩潰。

// ==========================================
// 滑鼠事件的橋樑 (中繼站)
// ==========================================
// 為什麼需要這個？因為 OpenCV 的 setMouseCallback 規定只能吃「靜態 (static)」函式或全域函式。
// 靜態函式是不屬於任何單一物件的，所以它不知道 `this` 是誰。
void VisualNovelGame::onMouseWrapper(int event, int x, int y, int flags, void* userdata) {
    // 我們在 init() 綁定時，把 this 當成 userdata 傳了進來。
    // 現在用 static_cast 把它從無型別指標 (void*) 硬轉回我們遊戲類別的指標 (VisualNovelGame*)
    VisualNovelGame* game = static_cast<VisualNovelGame*>(userdata);
    // 轉型成功後，就可以去呼叫類別裡面的那個非靜態、真正的處理函式了
    if (game) game->handleMouseClick(event, x, y);//handleMouseClick(滑鼠的按法, 橫坐標, 縱坐標);
}

// ==========================================
// 核心：處理所有畫面的滑鼠點擊 (上帝函式)
// ==========================================
void VisualNovelGame::handleMouseClick(int event, int x, int y) {
    if (event != EVENT_LBUTTONDOWN) return; // 如果不是「按下鼠標左鍵」的事件，就直接結束，什麼都不做

    // --- 狀態 A：如果現在是在主選單 ---
    if (state == MENU) {
        // contains 是 Rect (矩形) 內建的功能，可以判斷滑鼠座標 (Point) 有沒有落在這個框框裡
        if (startButton.contains(Point(x, y))) {
            state = STORY; // 切換到故事狀態
            currentLineIndex = 0; // 進度歸零
            if (!storyLines.empty()) fullText = storyLines[currentLineIndex]; // 載入第一句話
            currentText = ""; // 畫面字體清空
            textIndex = 0;    // 打字機歸零
        }
        else if (menuSaveLoadButton.contains(Point(x, y))) {
            previousState = state; // 記住現在是 MENU，讓按 ESC 時可以退回來
            isSavingMode = false;  // 左下角按鈕是讀檔，所以設為 false
            state = SAVELOAD;      // 切換狀態
            refreshSaveSlots();    // 抓取讀檔格子的資料
        }
        else if (menuGalleryButton.contains(Point(x, y))) {
            previousState = state;
            state = GALLERY;       // 切換到相簿
        }
    }
    // --- 狀態 B：如果現在是在講故事 ---
    else if (state == STORY) {
        // 先檢查是不是點到了右下角的 UI 按鈕
        if (storySaveLoadButton.contains(Point(x, y))) {
            previousState = state;
            isSavingMode = true; // 這裡是存檔模式
            state = SAVELOAD;
            currentScreenshot = currentFrame.clone(); // 偷拍一張照當作存檔縮圖
            refreshSaveSlots();
            return; // 提早結束，避免觸發下面的對話邏輯
        }
        else if (storyGalleryButton.contains(Point(x, y))) {
            previousState = state;
            state = GALLERY;
            return;
        }
        else if (storyReturnMenuButton.contains(Point(x, y))) {
            state = MENU; // 退回選單
            currentLineIndex = 0; textIndex = 0; currentText = ""; fullText = ""; // 清除文字殘留
            gameCount.reset(); // 回主選單要重置計分板，因為這等於放棄遊戲了
            return;
        }

        // 如果不是點按鈕，而是點了畫面的任何一處 (推進對話)
        if (textIndex < fullText.length()) {
            // 狀況 1：字還沒打完。玩家沒耐心了，直接把這句話的「完整內容」塞給「現在的文字」瞬間顯示
            currentText = fullText;
            textIndex = fullText.length();
        }
        else {
            // 狀況 2：字已經打完了。玩家點擊是要看下一句。
            currentLineIndex++; // 推進到下一句的索引值
            if (currentLineIndex < (int)storyLines.size()) { // 檢查劇本念完了沒
                fullText = storyLines[currentLineIndex]; // 載入下一句話
                currentText = ""; // 清空打字機
                textIndex = 0;
            }
            else {
                state = SELECTION; // 劇本念完了 (超出陣列長度)，自動進入選擇環節
            }
        }
    }
    // --- 狀態 C：如果現在是在選擇選項 ---
    else if (state == SELECTION) {
        // 特例檢查：真結局條件有沒有達成，而且玩家有點到那個隱形的隱藏按鈕
        if (gameShow.checkTrueEndCondition() && trueEndButton.contains(Point(x, y))) {
            int endID = 11; // 11 是真結局的代號
            gameShow.unlockEnding(endID); // 把真結局的相簿圖解鎖

            state = ENDING; // 遊戲狀態切換成 ENDING
            endingLines = gameEnd.getEndingLines(endID); // 去 End 系統撈出真結局的劇本陣列
            currentLineIndex = 0; // 重置文字狀態
            if (!endingLines.empty()) fullText = endingLines[0];
            else fullText = "找不到真結局文字檔！"; // 檔案遺失防呆
            currentText = "";
            textIndex = 0;
            return; // 提早結束，不再判斷下面的普通選項
        }

        // 一般選擇邏輯：用迴圈掃描 5 個按鈕
        for (size_t i = 0; i < choiceButtons.size(); ++i) {
            if (choiceButtons[i].contains(Point(x, y))) { // 只要找到滑鼠踩中的那一個
                gameCount.cho(i + 1); // 呼叫計分系統，把分數登記上去 (i 是 0~4，選項是 1~5)

                // 去計分板查查看，剛剛點的那個選項 (A~E)，現在總共被點了幾次？這決定了事件升級到哪個層級
                int level = 0;
                if (i == 0) level = gameCount.getA();
                else if (i == 1) level = gameCount.getB();
                else if (i == 2) level = gameCount.getC();
                else if (i == 3) level = gameCount.getD();
                else if (i == 4) level = gameCount.getE();

                state = FEEDBACK; // 分數加完了，進入事件回饋模式

                // 去讀取對應的事件回饋文字
                if (level - 1 < (int)feedbackTexts[i].size()) {
                    fullText = feedbackTexts[i][level - 1]; // 撈出對應等級的那句話
                }
                currentText = "";
                textIndex = 0;

                // 組裝檔名去讀取這個事件該顯示的插圖 (例如 "picture/event_1_2.jpg")
                string imgName = "picture/" + to_string(i + 1) + ".png";
                eventImg = imread(imgName);
                if (eventImg.empty()) { // 找不到圖的話防呆
                    eventImg = selectionImg.clone();
                }
                break; // 已經點到按鈕了，後面的按鈕不用檢查了，跳出迴圈
            }
        }
    }
    // --- 狀態 D：如果現在正在看選擇後的回饋文字 ---
    else if (state == FEEDBACK) {
        // 對話推進邏輯與 STORY 相同
        if (textIndex < fullText.length()) {
            currentText = fullText;
            textIndex = fullText.length();
        }
        else {
            // 去問計分系統，現在總共進行了幾次選擇 (getI)
            if (gameCount.getI() >= 5) {
                // 如果已經選了 5 次，遊戲結束，開始結算
                int endID = gameCount.getTheEnd(); // 算出對應的一般結局代號 (0~10)
                gameShow.unlockEnding(endID); // 解鎖該相簿

                state = ENDING; // 進入結局播放模式
                endingLines = gameEnd.getEndingLines(endID); // 讀結局文本
                currentLineIndex = 0;
                if (!endingLines.empty()) fullText = endingLines[0];
                currentText = "";
                textIndex = 0;
            }
            else {
                state = SELECTION; // 選擇還沒到 5 次，退回選項畫面繼續選
            }
        }
    }
    // --- 狀態 E：如果現在正在播結局的故事 ---
    else if (state == ENDING) {
        // 右下角的 UI 按鈕判定，與 STORY 狀態相同
        if (storySaveLoadButton.contains(Point(x, y))) {
            previousState = state;
            isSavingMode = true;
            state = SAVELOAD;
            currentScreenshot = currentFrame.clone();
            refreshSaveSlots();
            return;
        }
        else if (storyGalleryButton.contains(Point(x, y))) {
            previousState = state;
            state = GALLERY;
            return;
        }
        else if (storyReturnMenuButton.contains(Point(x, y))) {
            state = MENU;
            currentLineIndex = 0; textIndex = 0; currentText = ""; fullText = "";
            gameCount.reset();
            return;
        }

        // 對話推進邏輯
        if (textIndex < fullText.length()) {
            currentText = fullText;
            textIndex = fullText.length();
        }
        else {
            currentLineIndex++;
            if (currentLineIndex < (int)endingLines.size()) {
                fullText = endingLines[currentLineIndex];
                currentText = "";
                textIndex = 0;
            }
            else {
                // 結局的最後一句話念完了，代表整個遊戲通關！
                state = MENU; // 退回主選單
                currentLineIndex = 0; textIndex = 0; currentText = ""; fullText = "";
                gameCount.reset(); // 重置計分系統，為下一次遊玩準備
            }
        }
    }
    // --- 狀態 F：如果現在在存讀檔畫面 ---
    else if (state == SAVELOAD) {
        // 用迴圈檢查玩家有沒有點到 6 個存檔格子的任何一個
        for (size_t i = 0; i < saveSlots.size(); ++i) {
            if (saveSlots[i].contains(Point(x, y))) {
                int slotID = i + 1; // 算出格子的真實編號 (1~6)

                if (isSavingMode) { // 如果是存檔模式
                    string timeStr; // 用來接存檔時間
                    // 呼叫 Date 系統把現在的 state, lineIndex 和計分 count 全寫進 txt 裡
                    if (gameDate.saveProgress(slotID, (int)previousState, currentLineIndex, gameCount, timeStr)) {
                        // 存文字檔成功後，順便把剛剛備份的縮圖 currentScreenshot 也存成實體的 jpg 檔
                        imwrite("saves/thumb_" + to_string(slotID) + ".jpg", currentScreenshot);
                        sysMessage = "成功存入檔案 " + to_string(slotID) + "！"; // 設定提示訊息
                        sysMessageTimer = 90; // 設定倒數計時大約 3 秒
                    }
                    state = previousState; // 存檔完畢，自動退回原本遊玩的畫面
                }
                else { // 如果是讀檔模式
                    int savedState, savedLine;
                    string timeStr;
                    // 呼叫 Date 系統去 txt 把以前的進度全部讀出來
                    if (gameDate.loadProgress(slotID, savedState, savedLine, gameCount, timeStr)) {
                        state = (GameState)savedState; // 覆蓋目前的遊戲狀態
                        currentLineIndex = savedLine;  // 覆蓋對話進度

                        // 讀檔回來後，必須重新把之前讀到一半的那句話 (fullText) 載入記憶體
                        if (state == STORY && currentLineIndex < (int)storyLines.size()) {
                            fullText = storyLines[currentLineIndex];
                        }
                        else if (state == ENDING) {
                            int endID = gameCount.getTheEnd(); // 用剛剛讀取的分數，重新推算這是哪個結局

                            // 特別注意：如果這是一個達成真結局的存檔，推算出來的一般結局可能不準確
                            if (gameShow.checkTrueEndCondition()) {
                                endID = 11; // 強制設定為真結局的代號，不然會讀錯劇本
                            }

                            endingLines = gameEnd.getEndingLines(endID); // 重新載入結局劇本
                            if (currentLineIndex < (int)endingLines.size()) {
                                fullText = endingLines[currentLineIndex];
                            }
                        }
                        // 因為是讀檔，所以我們讓這句話直接全部顯示 (跳過打字機)
                        currentText = fullText;
                        textIndex = fullText.length();
                        sysMessage = "成功讀取檔案 " + to_string(slotID) + "！";
                        sysMessageTimer = 90;
                    }
                }
                break; // 點完格子就跳出迴圈
            }
        }
    }
    // --- 狀態 G：如果現在在相簿畫面 ---
    else if (state == GALLERY) {
        // 主程式不管相簿裡的按鈕怎麼點，直接把滑鼠座標 (x, y) 丟給 gameShow 子系統去判斷
        int tempState = (int)state; // 建立暫存變數
        gameShow.handleMouseClick(x, y, tempState, (int)previousState);
        state = (GameState)tempState; // 判斷完再把狀態拿回來 (如果相簿裡玩家點了退回，這行就會把 state 改回 previousState)
    }
}
#include "TextRenderer.h"
#include <windows.h>
#include <unordered_map>

using namespace cv;
using namespace std;

void TextRenderer::drawText(Mat& dst, const string& str, Point org, Scalar color, int fontSize) {
    // 如果字串是空的，就什麼都不做直接結束
    if (str.empty()) return;

    // ==========================================
    // 1. 快取 (Cache) 系統機制
    // ==========================================
    // 建立一個靜態字典，用來「記憶」已經畫過的字體影像，避免每幀重複運算導致遊戲卡頓
    static unordered_map<string, Mat> cache;
    // 將「字串內容」與「字體大小」結合成一個獨一無二的鑰匙 (例如: "你好_32")
    string key = str + "_" + to_string(fontSize);

    // 如果在快取字典裡「找不到」這把鑰匙，代表這個字是第一次出現，我們必須請 Windows 把它畫出來
    if (cache.find(key) == cache.end()) {

        // 簡單的記憶體管理：如果快取記了超過 100 句話，就把大腦清空，避免吃光電腦記憶體
        if (cache.size() > 100) cache.clear();

        // --- 字串格式轉換 ---
        // OpenCV 預設使用 UTF-8 編碼，但 Windows API 只看得懂寬字元 (UTF-16)
        // 下面這兩行是計算需要多大的空間，並將 UTF-8 轉成寬字元 wstring
        int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        wstring wstr(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);

        // ==========================================
        // 2. 呼叫 Windows 系統繪製文字 (GDI)
        // ==========================================
        // 建立一個存在於記憶體中的「隱形繪圖板」 (Device Context)
        HDC hdc = CreateCompatibleDC(NULL);

        // 設定字體：大小為 fontSize，字型為 "標楷體"，並開啟平滑邊緣 (ANTIALIASED_QUALITY)
        HFONT hFont = CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"標楷體");
        HGDIOBJ oldF = SelectObject(hdc, hFont); // 把字體裝進繪圖板裡

        // 計算這段文字畫出來會佔用多寬、多高的像素 (存入 sz 變數)
        SIZE sz;
        GetTextExtentPoint32W(hdc, wstr.c_str(), len - 1, &sz);

        // 設定一張位圖 (Bitmap) 的規格：寬度 sx.cx，高度 -sz.cy (負號代表影像由上往下繪製)，24位元全彩
        BITMAPINFO info = { {sizeof(BITMAPINFOHEADER), sz.cx, -sz.cy, 1, 24, BI_RGB} };

        // 根據規格，創造出一張真正的「隱形畫布」，並取得它的像素記憶體位置 (pix)
        void* pix;
        HBITMAP hBmp = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, &pix, NULL, 0);
        HGDIOBJ oldB = SelectObject(hdc, hBmp); // 把畫布裝進繪圖板裡

        // 設定畫筆顏色：背景塗成純黑色 (RGB 0,0,0)，字體塗成純白色 (RGB 255,255,255)
        // 這樣白色的部分等一下就可以當作「遮罩 (Mask)」使用
        SetBkColor(hdc, RGB(0, 0, 0));
        SetTextColor(hdc, RGB(255, 255, 255));

        // 正式把文字印到隱形畫布上！
        TextOutW(hdc, 0, 0, wstr.c_str(), len - 1);

        // --- 將 Windows 畫布轉交給 OpenCV ---
        // 把畫好的像素資料 (pix) 轉成 OpenCV 的 Mat 格式，並存入快取大腦裡
        // ((sz.cx * 3 + 3) / 4) * 4 是為了解決 Windows 影像寬度必須是 4 的倍數 (Padding) 的對齊問題
        cache[key] = Mat(sz.cy, sz.cx, CV_8UC3, pix, ((sz.cx * 3 + 3) / 4) * 4).clone();

        // 用完 Windows 的畫家工具後，必須歸還並銷毀，否則會造成記憶體嚴重外洩 (Memory Leak)
        SelectObject(hdc, oldB);
        SelectObject(hdc, oldF);
        DeleteObject(hBmp);
        DeleteObject(hFont);
        DeleteDC(hdc);
    }

    // ==========================================
    // 3. 將文字貼到遊戲畫面，並進行平滑去背融合
    // ==========================================
    // 從快取大腦裡拿出那張黑底白字的文字圖 (稱作遮罩 mask)
    Mat mask = cache[key];

    // 計算文字要貼在遊戲畫面的哪裡。用 & Rect(...) 取交集，是為了防止文字不小心超出視窗邊界導致程式崩潰
    Rect roi = Rect(org.x, org.y, mask.cols, mask.rows) & Rect(0, 0, dst.cols, dst.rows);
    if (roi.empty()) return; // 如果文字完全在視窗外面，就不畫了

    // 擷取遊戲畫面中，準備要貼上文字的那一塊小區域 (target)
    Mat target = dst(roi);
    // 擷取文字圖 (因為可能被邊界裁切，所以要對應計算正確大小)
    Mat mROI = mask(Rect(roi.x - org.x, roi.y - org.y, roi.width, roi.height));

    // --- 去背與透明度 (Alpha) 運算 ---
    Mat alpha;
    cvtColor(mROI, alpha, COLOR_BGR2GRAY); // 將黑底白字的文字圖轉成單一通道的灰階圖
    alpha.convertTo(alpha, CV_32FC1, 1.0 / 255.0); // 將像素值從 0~255 轉換成 0.0 ~ 1.0 的小數 (1.0 代表不透明字體，0.0 代表完全透明背景)

    // 把遊戲畫面的目標區域拆成 B(藍), G(綠), R(紅) 三個色彩通道
    vector<Mat> channels(3);
    split(target, channels);

    // 針對每一個色彩通道，進行去背融合運算 (Alpha Blending)
    for (int i = 0; i < 3; ++i) {
        Mat chFloat;
        channels[i].convertTo(chFloat, CV_32FC1); // 先轉成浮點數方便計算

        // 核心公式： 原本背景的顏色 * (1.0 - 透明度) + 你指定的文字顏色 * 透明度
        // 這會讓字體的邊緣呈現完美的平滑過渡，不會有醜醜的鋸齒
        chFloat = chFloat.mul(1.0f - alpha) + alpha * (float)color[i];

        // 算完後轉回正常的 8 位元影像格式
        chFloat.convertTo(channels[i], CV_8UC1);
    }

    // 把算好的 B, G, R 三個通道合併回去，完美地印在遊戲畫面上！
    merge(channels, target);
}//利用 Windows 畫出黑底白字的遮罩，再用數學公式把字體顏色與遊戲背景進行半透明融合，達成完美去背。
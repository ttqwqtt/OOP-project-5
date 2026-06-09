## 組別：第5組
## 系級班級：資訊工程1A
## 成員資訊：凌佩佩
## 小專題題目：模擬養成器(視覺小說)
## 程式介紹(含UML圖)：

<img width="299" height="469" alt="螢幕擷取畫面 2026-06-07 225618" src="https://github.com/user-attachments/assets/84d3463b-4501-4fd0-a6a7-fd7a2da6c83b" />

#### 在圖中，VisualNovelGame 連接著四個主要模組，這張圖有三個主要模組
Date (存讀檔管理員)：
負責處理遊戲進度的存取。
虛線箭頭指向 Count： Date 在執行 saveProgress或 loadProgress 時，必須索取 Count 裡面的計分資料才能完成存檔。

Count (計分與結局裁判)：
裡面的變數（如 chooseA）前面有加上 static 底線，代表這些分數是全域共用的。它負責記錄玩家按了哪 些 選 項 ， 並 結 算 出 最 終 的 結 局 代 號。
（getTheEnd()）。

End (劇本搬運工)：
負責讀取 .txt 文字檔，將故事台詞和結局對話載入記憶體。
它也有一條線連向 Count，因為它需要知道現在的計分狀態，才能決定要倒出哪一個結局的劇本給主程式。

<img width="223" height="455" alt="螢幕擷取畫面 2026-06-07 225625" src="https://github.com/user-attachments/assets/90aa20b1-5cc7-415c-96cd-49f66263fa1f" />

MAIN：
程式的進入點：負責把VISUALNOVELGAME 建立出來，並呼叫它的 INIT() 和 RUN() 讓遊戲動起來。

VISUALNOVELGAME：
遊戲的主引擎，裡面塞滿了各種變數（STATE, MENUIMG,STORYRETURNMENUBUTTON 等）與函式（RENDERSTORY,HANDLEMOUSECLICK 等）。
它負責監聽玩家的滑鼠點擊、更新畫面，並在不同的場景（主選單、故事、選項等）之間切換。

<img width="278" height="436" alt="螢幕擷取畫面 2026-06-07 225651" src="https://github.com/user-attachments/assets/2c5e6a4a-0366-483d-9129-929019e4296d" />

Show (相簿與成就系統)：
負責管理「結局一覽」的九宮格畫面。它會讀寫 achievements.txt，判斷哪些CG 圖該顯示，並掌管，真結局解鎖條件。

TextRenderer (文字渲染引擎)：
這是一個獨立的工具箱。 在需要畫中文的時候用它的 drawText 功能。

GameState (遊戲狀態列舉)：
圖中最右下角的模組，定義了 MENU,STORY, SELECTION, FEEDBACK,ENDING, SAVELOAD, GALLERY 這 7種狀態。它是讓主引擎知道「現在該畫什麼畫面」的核心指標。


## 程式如何安裝執行：
### 準備工作：
#### 下載 OpenCV： 前往 OpenCV 官網下符和你電腦的安裝包（自解壓檔）。
#### 解壓縮： 執行下載的 .exe 檔，將其解壓縮到一個路徑簡單的目錄（例如 C:\opencv）。後續說明將以 C:\opencv 作為範例路徑，請根據你的實際路徑做調整。

#### 步驟 1：設定系統環境變數（讓系統找到 DLL）為了讓編譯好的程式在執行時能找到 OpenCV 的動態連結庫（.dll），我們需要設定環境變數：
#### 1.1在 Windows 搜尋列輸入「編輯系統環境變數」並打開。
#### 1.2點擊「環境變數」按鈕。
#### 1.3在「系統變數」中找到 Path，選取後點擊「編輯」。
#### 1.4點擊「新增」，填入你的 OpenCV 執行檔路徑：
    C:\opencv\build\x64\vc16\bin 
#### （註：vc16 適用於 VS 2019 與 VS 2022）一路點擊「確定」儲存。（設定完後，請重啟 Visual Studio 讓環境變數生效）

#### 步驟 2：配置 Visual Studio 專案屬性
#### 2.1打開 Visual Studio，建立一個全新的 C++ 空白專案（Empty Project），並確保上方工具列的「方案平台」設定為 x64（OpenCV 官方預編譯版目前主要支援 64 位元）。
#### 2.2接下來，請在專案名稱上點擊右鍵，選擇「屬性」（Properties），進行以下三項核心設定：
#### a. 配置「包含目錄」（Include Directories）
#### 路徑：組態屬性 ➔ VC++ 目錄 ➔ 包含目錄
#### 操作：點擊右側的下拉箭頭 ➔ 編輯 ➔ 新增以下路徑：
    C:\opencv\build\include
#### b. 配置「程式庫目錄」（Library Directories）
#### 路徑：組態屬性 ➔ VC++ 目錄 ➔ 程式庫目錄
#### 操作：點擊右側的下拉箭頭 ➔ 編輯 ➔ 新增以下路徑：
    C:\opencv\build\x64\vc16\lib
#### c. 配置「其他相依性」（Additional Dependencies）
#### 路徑：組態屬性 ➔ 連結器 ➔ 輸入 ➔ 其他相依性
#### 操作：這裡需要根據你的組態（Debug 或 Release）來填入對應的 .lib 檔名。請先確認你目前要設定哪一個（建議兩個分開設定）：
#### 填入帶有 d 的檔案，例如 opencv_world4100d.lib(註：4100 代表 OpenCV 4.10.0 版，請去C:\opencv\build\x64\vc16\lib 資料夾下確認你下載的版本數字並修改。)
#### 像這樣：
    opencv_world4100d.lib
    
### 從 GitHub 下載下來的檔案，只有 .cpp、.h 檔案喔！！！（單純的原始碼）
#### 1.打開 Visual Studio，依據前文教學建立一個全新的 C++ 空白專案。
#### 2.在右側的「方案總管」中，右鍵點擊「來源檔案」資料夾 ➔ 加入 ➔ 現有項目。
#### 3.選取你從 GitHub 下載下來的那些 .cpp 檔案。
#### 4.如果有 .h 檔案，則在「標頭檔」資料夾上點右鍵 ➔ 加入 ➔ 現有項目 把它們放進去。
#### ps:frist.txt是文檔，記得手動放入資料夾!!!

## 程式畫面截圖：
<img width="413" height="374" alt="螢幕擷取畫面 2026-06-07 223334" src="https://github.com/user-attachments/assets/f8380289-5a05-452f-bf05-bb13e5683515" />

#### 使用鍵盤跳轉畫面，S為跳轉存檔畫面，L為跳轉讀檔畫面，ESC可以返回上一頁面。


<img width="470" height="368" alt="螢幕擷取畫面 2026-06-07 223419" src="https://github.com/user-attachments/assets/fb87189b-851e-42c2-b40d-b234510e7792" />

#### 使用滑鼠在進行點選項時，可以按到那個選項，除了累計之外，還會帶有回饋事件。



## 分工資訊：全都是一人做。

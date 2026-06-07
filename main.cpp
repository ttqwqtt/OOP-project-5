#include "VisualNovelGame.h"

int main() {
    VisualNovelGame game;

    // 初始化遊戲，如果失敗（例如找不到圖片）就關閉程式
    if (!game.init()) {
        return -1;
    }

    // 啟動遊戲主迴圈
    game.run();

    return 0;
}
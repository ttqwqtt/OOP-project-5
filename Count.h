#ifndef COUNT_H
#define COUNT_H

class Count {
private:
    // 記錄各選項被選擇的次數 (靜態變數供全局累加)
    static int chooseA;
    static int chooseB;
    static int chooseC;
    static int chooseD;
    static int chooseE;
    static int i; // 記錄目前是第幾局選擇

    int choose;   // 玩家單次的選擇

public:
    Count();
    int getA();
    int getB();
    int getC();
    int getD();
    int getE();
    int getI();

    // 處理玩家選擇並累加對應的計數器
    void cho(int choose);

    // 根據選擇次數結算，回傳對應的結局代號 (0~10)
    int getTheEnd();
    // 【新增】重置所有計數器，為了多周目遊玩準備
    void reset();
    // 【新增】用來在讀檔時，強制寫入所有屬性數值
    void setAll(int a, int b, int c, int d, int e, int round);
};
#endif

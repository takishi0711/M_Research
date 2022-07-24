#include <string>

#include "accm.hpp"



int main(int argc, char *argv[]) {
    // コマンドライン引数にグラフデータがあるディレクトリを入れる
    // "./dataset/test/abilene06.txt"だったら「./a.out ./dataset/test/」

    // char* から string に変換
    std::string dir_path = argv[1]; 

    // 自律的輻輳制御機構 (Autonomous_Congestion_Control_Mechanism) を起動
    ACCM accm(dir_path); 
    
    return 0;
}
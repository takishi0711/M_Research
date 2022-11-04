#include <string>

#include "src2/arws.hpp"



int main(int argc, char *argv[]) {
    // コマンドライン引数にグラフデータがあるディレクトリを入れる
    // "./dataset/test/abilene06.txt"だったら「./a.out ./dataset/test/」

    // char* から string に変換
    std::string dir_path = argv[1]; 

    // Async Random Walk System を起動 
    ARWS arws(dir_path); 
    
    return 0;
}
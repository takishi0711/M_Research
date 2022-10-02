

#include "random_walker.hpp"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace Message 
{   
    // 生存している RWer 送信用のメッセージ作成
    void createSendRWerMessage(char* message, RandomWalker RWer);

    // 終了した RWer 送信用のメッセージ作成
    void createSendFinRWerMessage(char* message, RandomWalker RWer);

    // メッセージから RWer の復元
    RandomWalker readMessage(char* message);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline void Message::createSendRWerMessage(char* message, RandomWalker RWer) {
    // メッセージ ID は 2
    message[0] = '2';

    // メッセージに RWer 情報を格納
    int message_index = 1;
    RWer.serialize(message, message_index);
}

inline void Message::createSendFinRWerMessage(char* message, RandomWalker RWer) {
    // メッセージ ID は 3
    message[0] = '3';

    // メッセージに RWer 情報を格納
    int message_index = 1;
    RWer.serialize(message, message_index);
}

inline RandomWalker Message::readMessage(char* message) {
    int message_index = 1;
    
    return RandomWalker(message, message_index);
}
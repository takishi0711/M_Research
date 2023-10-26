#include <iostream>

using namespace std;

#include "../cache_helper.hpp"

int main() {
    LRU cache;

    cache.setCapacity(3);

    cache.putIndex(1, 11, 111);
    cache.putIndex(2, 22, 222);

    cache.printList();
    cout << endl;

    cache.putIndex(3, 33, 333);

    cache.printList();
    cout << endl;

    cache.putIndex(4, 44, 444);

    cache.printList();
    cout << endl;

    cache.putIndex(2, 22, 222);

    cache.printList();
    cout << endl;

    cache.putIndex(4, 44, 444);

    cache.printList();
    cout << endl;

    cache.putIndex(1, 11, 111);

    cache.printList();
    cout << endl;

    return 0; 
}
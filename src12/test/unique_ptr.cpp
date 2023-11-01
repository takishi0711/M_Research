#include <iostream>
#include <memory>
#include <vector>
#include <utility>

using namespace std;

int main() {
    vector<unique_ptr<int>> vec;
    vec.push_back(make_unique<int>(5));
    unique_ptr<int> ptr = move(vec[0]);
    cout << *ptr << endl;
    return 0;
}
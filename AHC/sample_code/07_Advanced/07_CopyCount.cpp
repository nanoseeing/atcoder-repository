// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <iostream>
#include <random>
#include <queue>

int operator_count = 0;

class State
{

public:
    int value_;

    State(const int value = 0) : value_(value) {}
    void operator=(const State &state)
    {
        this->value_ = state.value_;
        ++operator_count;
    }
};
bool operator<(const State &state1, const State &state2)
{
    return state1.value_ < state2.value_;
}
int main()
{
    using std::cout;
    using std ::endl;
    std::mt19937 mt(0);
    std::priority_queue<State> queue;
    for (int i = 0; i < 100; i++)
    {
        queue.push(State(mt() % 100));
    }
    cout << "operator is called " << operator_count << " times" << endl;
    return 0;
}
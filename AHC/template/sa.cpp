#include <bits/stdc++.h>
using namespace std;

// =================================
// Simulated Annealing
// =================================

double exponential_schedule(double init, double obj, double elapsed_time, double max_time) {
    double lambda_param = log(obj / init) / max_time;
    return init * exp(lambda_param * elapsed_time);
}

double linear_schedule(double init, double obj, double elapsed_time, double max_time) {
    return init + (obj - init) * (elapsed_time / max_time);
}

template <typename T>
tuple<T, double> SimulatedAnnealing(
    const T &x0,
    double t0,
    double t1,
    double max_time,
    int display_interval = 10000) {

    T x = x0;
    T best_x = x;

    double current_cost = 0; // TODO
    double best_cost = current_cost;

    TimeKeeper time_keeper(max_time);

    int iteration = 0;
    for(iteration = 0;; iteration++) {
        int elapsed = time_keeper.getElapsedTime();
        if(elapsed >= max_time)
            break;

        double temp = linear_schedule(t0, t1, elapsed, max_time);

        T new_x = x;         // TODO
        double new_cost = 0; // TODO

        double delta_cost = new_cost - current_cost;
        if(delta_cost < 0 || x32rng.rand() < exp(-delta_cost / temp)) { // TODO Tempをexp内に含めると勾配が急になる（≒受理確率が下がる）
            current_cost = new_cost;
            x = new_x;
        }

        if(current_cost < best_cost) {
            best_x = x;
            best_cost = current_cost;
        }

        iteration++;
        if(iteration % display_interval == 0) {
            cerr << "Iteration: " + to_string(iteration) +
                        ", Current cost: " + to_string(current_cost) +
                        ", Best cost: " + to_string(best_cost) +
                        ", Temp: " + to_string(temp)
                 << endl;
        }
    }

    cerr << "Iteration: " + to_string(iteration) +
                ", Best cost: " + to_string(best_cost)
         << endl;

    return {best_x, best_cost};
}
#ifndef CONGESTION_CONTROL_H_INCLUDED
#define CONGESTION_CONTROL_H_INCLUDED
#include <bits/stdc++.h>

class MyCongestion
{
public:
    MyCongestion(float initial_window, int initial_threshold);
    void set_windowSize(float window);
    void set_threshold(int threshold);
    float get_windowSize();
    int get_threshold();
    void set_state(std::string new_state);
    int get_numOfDupACKs();
    void update(std::string event);
    std::string get_currentState();
    void update_slowStart(std::string event);
    void update_fastRecovery(std::string event);
    void update_congestionAvoidance(std::string event);
};

#endif // CONGESTION_CONTROL_H_INCLUDED

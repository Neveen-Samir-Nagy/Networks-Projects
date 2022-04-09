#include "Congestion_Control.h"
/**-----------------------------------------------------------------------------------------
 * Usage   : handle congestion control
 *-----------------------------------------------------------------------------------------*/

 using namespace std;
 std::string state = "SLOW_START";
 int current_threshold;
 float current_windowSize;
 int numOfACKs = 0;

 MyCongestion::MyCongestion(float initial_window, int initial_threshold)
 {
     numOfACKs = 0;
     current_threshold = initial_threshold;
     current_windowSize = initial_window;
 }

 void MyCongestion::set_threshold(int threshold)
 {
     current_threshold = threshold;
 }

 void MyCongestion::set_windowSize(float window)
 {
     current_windowSize = window;
 }

 void MyCongestion::set_state(std::string new_state)
 {
     state = new_state;
 }

 std::string MyCongestion::get_currentState()
 {
     return state;
 }

 int MyCongestion::get_threshold()
 {
     return current_threshold;
 }

 int MyCongestion::get_numOfDupACKs()
 {
     return numOfACKs;
 }

 float MyCongestion::get_windowSize()
 {
     return current_windowSize;
 }

 void MyCongestion::update(std::string event)
 {
     if(state == "SLOW_START")
     {
         update_slowStart(event);
     }
     else if(state == "FAST_RECOVERY")
     {
         update_fastRecovery(event);
     }
     else if(state == "CON_AVOID")
     {
         update_congestionAvoidance(event);
     }
 }

 void MyCongestion::update_fastRecovery(std::string event)
 {
     if(event == "DUP_ACK")
     {
         current_windowSize = current_windowSize + 1;
     }
     else if(event == "TIME_OUT")
     {
         state = "SLOW_START";
         current_threshold = current_windowSize/2;
         current_windowSize  = 1;
         numOfACKs = 0;
     }
     else if(event == "NEW_ACK")
     {
         state = "CON_AVOID";
         current_windowSize = current_threshold;
         numOfACKs = 0;
     }
 }

 void MyCongestion::update_congestionAvoidance(std::string event)
 {
     if(event == "DUP_ACK")
     {
         numOfACKs++;
     }
     else if(event == "NEW_ACK")
     {
         numOfACKs = 0;
         current_windowSize = current_windowSize + (1 / floor(current_windowSize));
     }
     else if(event == "TIME_OUT")
     {
         state = "SLOW_START";
         current_threshold = current_windowSize/2;
         current_windowSize = 1;
         numOfACKs = 0;
     }
     if(numOfACKs == 3)
     {
         state = "FAST_RECOVERY";
         current_threshold = current_windowSize/2;
         current_windowSize = current_threshold + 3;
     }
 }

 void MyCongestion::update_slowStart(std::string event)
 {
     if(event == "TIME_OUT")
     {
         current_threshold = current_windowSize/2;
         current_windowSize = 1;
         numOfACKs = 0;
     }
     else if(event == "DUP_ACK")
     {
         numOfACKs++;
     }
     else if(event == "NEW_ACK")
     {
         current_windowSize = current_windowSize + 1;
         numOfACKs = 0;
     }
     if(current_windowSize >= current_threshold)
     {
         state = "CON_AVOID";
     }
     if(numOfACKs == 3)
     {
         state = "FAST_RECOVERY";
         current_threshold = current_windowSize/2;
         current_windowSize = current_threshold + 3;
     }
 }

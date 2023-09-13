#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include <array>
#include <iterator>
#include <set>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <pthread.h>
#include <semaphore.h>
#include "progtest_solver.h"
#include "sample_tester.h"
using namespace std;
#endif /* __PROGTEST__ */

class CCargoPlanner
{
    private:
        vector<thread> salesThreads;
        vector<thread> workersThreads;
        deque<AShip> newShips;
        deque<pair<AShip, vector<CCargo>>> toLoadShips;
        vector<ACustomer> customers;
        condition_variable cv_newShip;
        condition_variable cv_toLoadShip;
        mutex mtx_newShip;
        mutex mtx_loadShip;

        void salerMan();
        void worker();

    public:
        static int               SeqSolver                     ( const vector<CCargo> & cargo,
                                                             int               maxWeight,
                                                             int               maxVolume,
                                                             vector<CCargo>  & load );
        void                     Start                         ( int               sales,
                                                             int               workers );
        void                     Stop                          ( );

        void                     Customer                      ( ACustomer         customer );
        void                     Ship                          ( AShip             ship );
};


/* --------------- CCargoPlanner ------------------------ */

int CCargoPlanner::SeqSolver(const vector<CCargo> &cargo, int maxWeight, int maxVolume, vector<CCargo> &load)
{
    return ProgtestSolver(cargo, maxWeight, maxVolume, load);
}

void CCargoPlanner::Start(int sales, int workers)
{
    for (int i = 0; i < sales; i++)
        salesThreads.emplace_back( &CCargoPlanner::salerMan, this );

    for (int i = 0; i < workers; i++)
        workersThreads.emplace_back( &CCargoPlanner::worker, this );
}

void CCargoPlanner::Stop()
{
    {
        lock_guard<mutex> lck(mtx_newShip);
        newShips.emplace_back(nullptr);
    }

    for (auto & it: salesThreads)
        it.join();

    {
        lock_guard<mutex> lck(mtx_loadShip);
        toLoadShips.emplace_back(nullptr, vector<CCargo>{});
    }

    cv_toLoadShip.notify_all();

    for (auto & it: workersThreads)
        it.join();
}

void CCargoPlanner::Customer(ACustomer customer)
{
    customers.emplace_back(customer);
}

void CCargoPlanner::Ship(AShip ship)
{
    {
        lock_guard<mutex> lck(mtx_newShip);
        newShips.emplace_back(ship);
    }
    cv_newShip.notify_all();
}

void CCargoPlanner::salerMan()
{
    while (true)
    {
        unique_lock<mutex> lck(mtx_newShip);
        while (newShips.empty()) cv_newShip.wait(lck);
        auto actualShip = newShips.front();
        if (actualShip == nullptr)
            break;
        newShips.pop_front();
        lck.unlock();

        vector <CCargo> possibleLoad;
        for (auto & customer : customers)
        {
            vector <CCargo> actCargo;
            customer->Quote(actualShip->Destination(), actCargo);
            possibleLoad.insert(possibleLoad.end(), make_move_iterator(actCargo.begin()), make_move_iterator(actCargo.end()));
        }

        {
            lock_guard<mutex> lck2(mtx_loadShip);
            toLoadShips.emplace_back(actualShip, possibleLoad);
        }
        cv_toLoadShip.notify_all();
    }

}

void CCargoPlanner::worker()
{
    while (true)
    {
        unique_lock<mutex> lck(mtx_loadShip);
        while (toLoadShips.empty()) cv_toLoadShip.wait(lck);
        auto actualShip = toLoadShips.front();
        if (actualShip.first == nullptr)
            break;
        toLoadShips.pop_front();
        lck.unlock();

        vector<CCargo> load;
        SeqSolver(actualShip.second, actualShip.first->MaxWeight(), actualShip.first->MaxVolume(), load);
        actualShip.first->Load(load);
    }

}

//-------------------------------------------------------------------------------------------------
#ifndef __PROGTEST__
int                main                                    ( void )
{
  CCargoPlanner  test;
  vector<AShipTest> ships;
  vector<ACustomerTest> customers { make_shared<CCustomerTest> (), make_shared<CCustomerTest> () };
  
  ships . push_back ( g_TestExtra[0] . PrepareTest ( "New York", customers ) );
  ships . push_back ( g_TestExtra[1] . PrepareTest ( "Barcelona", customers ) );
  ships . push_back ( g_TestExtra[2] . PrepareTest ( "Kobe", customers ) );
  ships . push_back ( g_TestExtra[8] . PrepareTest ( "Perth", customers ) );
  
  
  for ( auto x : customers )
    test . Customer ( x );
  
  test . Start ( 3, 2 );
  
  for ( auto x : ships )
    test . Ship ( x );

  test . Stop  ();

  for ( auto x : ships )
    cout << x -> Destination () << ": " << ( x -> Validate () ? "ok" : "fail" ) << endl;
  return 0;  
}
#endif /* __PROGTEST__ */ 

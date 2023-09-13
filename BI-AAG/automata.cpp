#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <vector>
using namespace std;


using State = unsigned int;
using Symbol = char;

#include "sample.h"

#endif

#define NOT_EXIST 2222775555


void printMap( std::vector < std::vector < State > > & bigMap )
{
  for (auto & it: bigMap)
  {
    for (auto & itit: it)
      cout << itit << "|";
    cout << std::endl;
  }
}

DFA determinize ( const MISNFA & nfa )
{
  std::set < std::set < State > > dkaStates;
  std::queue < std::set < State > > dkaNextStates;
  std::map < std::pair < std::set < State >, Symbol >, std::set < State > > dkaTransitions;

  dkaStates.insert(nfa.m_InitialStates);
  dkaNextStates.push(nfa.m_InitialStates);

  while (!dkaNextStates.empty())
  {
    auto setState = dkaNextStates.front();
    dkaNextStates.pop();
    for (const auto& letter: nfa.m_Alphabet)
    {
      std::set < State > newState;
      for (const auto& oneState: setState)
      {
        std::set<State> tempState;
        try
        {
          tempState = nfa.m_Transitions.at(std::make_pair(oneState, letter));
        }
        catch (std::out_of_range &)
        {

        }
        for (const auto& it: tempState)
        {
          newState.insert(it);
        }
      }
      if (!newState.empty())
        if (dkaStates.insert(newState).second)
          dkaNextStates.push(newState);
      dkaTransitions[std::make_pair(setState, letter)] = newState;
    }
  }

  std::map < std::set < State >,  State > statesName;
  DFA resFA;
  resFA.m_Alphabet = nfa.m_Alphabet;
  State stateCounter = 0;
  for (const auto& newState: dkaStates)
  {
    statesName[newState] = stateCounter;
    resFA.m_States.insert(stateCounter);
    if (newState == nfa.m_InitialStates)
      resFA.m_InitialState = stateCounter;

    for (const auto& it: newState)
      if (nfa.m_FinalStates.find(it) != nfa.m_FinalStates.end())
      {
        resFA.m_FinalStates.insert(stateCounter);
        break;
      }

    stateCounter++;
  }

  for (const auto& it: dkaTransitions)
  {
    if (!it.second.empty())
      resFA.m_Transitions[std::make_pair(statesName[it.first.first], it.first.second)] = statesName[it.second];
  }

  return resFA;
}

DFA trim ( const DFA & dfa )
{
  std::set < State > usefulStates(dfa.m_FinalStates.begin(), dfa.m_FinalStates.end());
  std::queue < State > nextStates;
  for (auto& it: usefulStates)
    nextStates.push(it);
  DFA resFA;

  while (!nextStates.empty())
  {
    auto actState = nextStates.front();
    nextStates.pop();
    for (auto& it: dfa.m_Transitions)
    {
      if (it.second == actState)
      {
        if (usefulStates.insert(it.first.first).second)
          nextStates.push(it.first.first);
      }
    }
  }
  std::vector < State > useless;

  std::set_difference(dfa.m_States.begin(), dfa.m_States.end(), usefulStates.begin(), usefulStates.end(), std::inserter(useless, useless.begin()));

  std::set < State > uselessStates(useless.begin(), useless.end());

  resFA.m_FinalStates = dfa.m_FinalStates;
  resFA.m_InitialState = dfa.m_InitialState;
  resFA.m_Alphabet = dfa.m_Alphabet;

  if (usefulStates.empty())
  {
    resFA.m_States.insert(dfa.m_InitialState);
    return resFA;
  }

  resFA.m_States = usefulStates;
  for (auto & it: dfa.m_Transitions)
  {
    if (uselessStates.find(it.first.first) == uselessStates.end() && uselessStates.find(it.second) == uselessStates.end())
      resFA.m_Transitions[it.first] = it.second;
  }

  return resFA;
}

DFA minimize ( const DFA & dfa )
{
  std::unordered_map < State, State > actNames;
  std::map < std::vector < State >, State > seqMap;

  std::vector < std::vector < State > > bigMap;
  std::map < Symbol, unsigned int > symbolNames;

  unsigned int symbolCounter = 0;
  for (auto & it: dfa.m_Alphabet)
    symbolNames[it] = symbolCounter++;

  int stateCounter = 0;
  for (auto & actState: dfa.m_States)
  {
    bigMap.emplace_back();
    bigMap[stateCounter].push_back(actState);
    for (auto & actLetter: dfa.m_Alphabet)
    {
      auto res = dfa.m_Transitions.find(std::make_pair(actState, actLetter));
      if (res == dfa.m_Transitions.end())
        bigMap[stateCounter].push_back(NOT_EXIST);
      else
        bigMap[stateCounter].push_back(res->second);
    }
    stateCounter++;
  }

  std::map < bool, State > finalNonFinal;
  if (dfa.m_FinalStates.find(bigMap[0][0]) != dfa.m_FinalStates.end())
  {
    finalNonFinal[true] = bigMap[0][0];
  }


  for (auto & actState: bigMap)
  {
    if (dfa.m_FinalStates.find(actState[0]) != dfa.m_FinalStates.end())
    {
      if (finalNonFinal.find(true) == finalNonFinal.end())
      {
        actState.push_back(actState[0]);
        finalNonFinal[true] = actState[0];
      }
      else
        actState.push_back(finalNonFinal.find(true)->second);
    }
    else
    {
      if (finalNonFinal.find(false) == finalNonFinal.end())
      {
        actState.push_back(actState[0]);
        finalNonFinal[false] = actState[0];
      }
      else
        actState.push_back(finalNonFinal.find(false)->second);
    }
    actNames[actState[0]] = actState.back();
  }

  bool same = true;
  for (auto & it: bigMap)
  {
    if (it[0] != it.back())
    {
      same = false;
      break;
    }
  }

  int columnCounter = (signed)dfa.m_Alphabet.size()+1;
  std::set < std::vector < State > > statesSeq;
  while (!same)
  {
    for (auto & actState: bigMap)
    {
      for (int actLetter = 1; actLetter <= (signed) dfa.m_Alphabet.size(); actLetter++)
      {
        if (actState[actLetter] == NOT_EXIST)
          actState.push_back(NOT_EXIST);
        else
          actState.push_back(actNames.at(actState[actLetter]));
      }
    }

    seqMap.clear();
    for (auto & actState: bigMap)
    {
      std::vector < State > actSeq(actState.begin() + columnCounter, actState.end());
      if (seqMap.find(actSeq) != seqMap.end())
      {
        actState.push_back(seqMap[actSeq]);
      }
      else
      {
        seqMap[actSeq] = actState[0];
        actState.push_back(actState[0]);
      }
      actNames[actState[0]] = actState.back();
    }

    same = true;
    for (auto & it: bigMap)
    {
      if (it[columnCounter] != it.back())
      {
        same = false;
        break;
      }
    }
    if (same) break;
    columnCounter = (signed)bigMap[0].size()-1;
  }

  DFA resFA;
  resFA.m_InitialState = dfa.m_InitialState;

  std::set < State > withoutDuplStatesTmp;
  std::set < State > withoutDuplStates;
  std::map <State, State> mapDupl;
  for (auto & it: bigMap)
  {
    {
      if (withoutDuplStatesTmp.insert(it.back()).second)
        withoutDuplStates.insert(it[0]);
      else
        mapDupl[it[0]] = it.back();
      if (dfa.m_InitialState == it[0])
        resFA.m_InitialState = it.back();
    }
  }

  resFA.m_States = withoutDuplStates;
  resFA.m_Alphabet = dfa.m_Alphabet;


  for (auto & it: dfa.m_FinalStates)
  {
    if (withoutDuplStates.find(it) != withoutDuplStates.end())
      resFA.m_FinalStates.insert(it);
  }

  for (auto & it: dfa.m_Transitions)
  {
    if (withoutDuplStates.find(it.first.first) == withoutDuplStates.end())
      continue;

    if (mapDupl.find(it.second) != mapDupl.end())
      resFA.m_Transitions[it.first] = mapDupl[it.second];
    else
      resFA.m_Transitions[it.first] = it.second;
  }

  return resFA;
}


#ifndef __PROGTEST__

int main ( ) {

  // determinize
  assert ( determinize ( in0 ) == outD0 );
  assert ( determinize ( in1 ) == outD1 );
  assert ( determinize ( in2 ) == outD2 );
  assert ( determinize ( in3 ) == outD3 );
  assert ( determinize ( in4 ) == outD4 );
  assert ( determinize ( in5 ) == outD5 );
  assert ( determinize ( in6 ) == outD6 );
  assert ( determinize ( in7 ) == outD7 );
  assert ( determinize ( in8 ) == outD8 );
  assert ( determinize ( in9 ) == outD9 );
  assert ( determinize ( in10 ) == outD10 );
  assert ( determinize ( in11 ) == outD11 );
  assert ( determinize ( in12 ) == outD12 );
  assert ( determinize ( in13 ) == outD13 );
  determinize ( in0 ) ;
  determinize ( in1 ) ;
  determinize ( in2 ) ;
  determinize ( in3 ) ;
  determinize ( in4 ) ;
  determinize ( in5 ) ;
  determinize ( in6 ) ;
  determinize ( in7 ) ;
  determinize ( in8 ) ;
  determinize ( in9 ) ;
  determinize ( in10 );
  determinize ( in11 );
  determinize ( in12 );
  determinize ( in13 );


  // trim
  assert ( trim ( determinize ( in0 ) ) == outT0 );
  assert ( trim ( determinize ( in1 ) ) == outT1 );
  assert ( trim ( determinize ( in2 ) ) == outT2 );
  assert ( trim ( determinize ( in3 ) ) == outT3 );
  assert ( trim ( determinize ( in4 ) ) == outT4 );
  assert ( trim ( determinize ( in5 ) ) == outT5 );
  assert ( trim ( determinize ( in6 ) ) == outT6 );
  assert ( trim ( determinize ( in7 ) ) == outT7 );
  assert ( trim ( determinize ( in8 ) ) == outT8 );
  assert ( trim ( determinize ( in9 ) ) == outT9 );
  assert ( trim ( determinize ( in10 ) ) == outT10 );
  assert ( trim ( determinize ( in11 ) ) == outT11 );
  assert ( trim ( determinize ( in12 ) ) == outT12 );
  assert ( trim ( determinize ( in13 ) ) == outT13 );
  assert ( trim(limitVal) == limitVal );

  // minimize
  assert ( minimize ( trim ( determinize ( in0 ) ) ) == outM0 );
  assert ( minimize ( trim ( determinize ( in1 ) ) ) == outM1 );
  assert ( minimize ( trim ( determinize ( in2 ) ) ) == outM2 );
  assert ( minimize ( trim ( determinize ( in3 ) ) ) == outM3 );
  assert ( minimize ( trim ( determinize ( in4 ) ) ) == outM4 );
  assert ( minimize ( trim ( determinize ( in5 ) ) ) == outM5 );
  assert ( minimize ( trim ( determinize ( in6 ) ) ) == outM6 );
  assert ( minimize ( trim ( determinize ( in7 ) ) ) == outM7 );
  assert ( minimize ( trim ( determinize ( in8 ) ) ) == outM8 );
  assert ( minimize ( trim ( determinize ( in9 ) ) ) == outM9 );
  assert ( minimize ( trim ( determinize ( in10 ) ) ) == outM10 );
  assert ( minimize ( trim ( determinize ( in11 ) ) ) == outM11 );
  assert ( minimize ( trim ( determinize ( in12 ) ) ) == outM12 );
  assert ( minimize ( trim ( determinize ( in13 ) ) ) == outM13 );
  minimize ( trim ( determinize ( in14 ) ) );


  return 0;
}
#endif

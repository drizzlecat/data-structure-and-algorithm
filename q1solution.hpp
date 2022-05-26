#ifndef SOLUTION_HPP_
#define SOLUTION_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include "ics46goody.hpp"
#include "ics_exceptions.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_map.hpp"


//Helper Functions (you decide what is useful)
//Hint: I used helpers for only sort_descendants and sort_generations
int desc_sum(const ics::ArrayQueue<int>& d){
  int sum = 0;
  for (auto ad : d)
    sum += ad;
  return sum;
}

bool sd_gt_descendants(const ics::pair<std::string,ics::ArrayQueue<int>>& a, const ics::pair<std::string,ics::ArrayQueue<int>>& b) {
  int desc_a = desc_sum(a.second), desc_b = desc_sum(b.second);
  return desc_a == desc_b ? a.first < b.first :
                          desc_a < desc_b;
}

bool sd_gt_generations(const ics::pair<std::string,ics::ArrayQueue<int>>& a, const ics::pair<std::string,ics::ArrayQueue<int>>& b) {
  int desc_a = desc_sum(a.second), desc_b = desc_sum(b.second);
  int gen_a = a.second.size(), gen_b = b.second.size();
  return gen_a == gen_b ?
           (desc_a == desc_b ?  a.first < b.first : desc_a > desc_b) :
           gen_a > gen_b;
}



//Problem #1a and #1b
template<class KEY,class T>
void swap (ics::ArrayMap<KEY,T>& m, KEY key1, KEY key2) {
  m[key2] = m.put(key1,m[key2]);
  // m.put(key2,  m.put(key1, m[key2])); //Alternative (all put) solution
}


template<class KEY,class T>
void values_set_to_queue (const ics::ArrayMap<KEY,ics::ArraySet<T>>& m1,
                          ics::ArrayMap<KEY,ics::ArrayQueue<T>>&     m2) {
  for (auto e : m1)
    m2[e.first] = ics::ArrayQueue<T>(e.second);
    //or m2[e.first].enqueue_all(e.second);
}


//Problem #2a and #2b
ics::ArrayQueue<std::string> sort_descendants (const ics::ArrayMap<std::string,ics::ArrayQueue<int>>& m) {
 ics::ArrayPriorityQueue<ics::pair<std::string,ics::ArrayQueue<int>>,sd_gt_descendants> sorted(m);
 ics::ArrayQueue<std::string> answer;
 for (auto nd : sorted)
   answer.enqueue(nd.first);

 return answer;
}


ics::ArrayQueue<ics::pair<std::string,ics::ArrayQueue<int>>> sort_generations (const ics::ArrayMap<std::string,ics::ArrayQueue<int>>& m) {
 ics::ArrayPriorityQueue<ics::pair<std::string,ics::ArrayQueue<int>>,sd_gt_generations> sorted(m);
 ics::ArrayQueue<ics::pair<std::string,ics::ArrayQueue<int>>> answer(sorted);

 return answer;
}


//Problem #3a and #3b
ics::ArraySet<std::string> big_family(const ics::ArrayMap<std::string,ics::ArrayQueue<int>>& m) {
  ics::ArraySet<std::string> answer;
  for (auto nd : m)
    if (desc_sum(nd.second) > 10)
      answer.insert(nd.first);

  return answer;
}


ics::ArrayMap<std::string,ics::ArraySet<char>> big_word_map(const std::string sentence) {
  ics::ArrayMap<std::string,ics::ArraySet<char>> answer;
  auto words = ics::split(sentence," ");

  for (auto word : words)
    if (word.size() > 3)
       answer[word] = ics::ArraySet<char>(word);

  return answer;
}


//Problem #4
ics::ArrayMap<char,ics::ArraySet<char>> near(const std::string word, int dist) {
  ics::ArrayMap<char,ics::ArraySet<char>> answer;
  for (int i = 0; i < int(word.size()); ++i)
    for (int j = std::max<int>(0,i-dist); j <= std::min<int>(word.size()-1,i+dist); ++j)
      answer[word[i]].insert(word[j]);
  return answer;
}


//Problem #5
ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>> alt_map(const  ics::ArrayMap<ics::pair<std::string,std::string>,int>calls) {
  ics::ArrayMap<std::string,ics::ArrayMap<std::string,int>> answer;
  for (auto cci : calls) {
    if (!answer.has_key(cci.first.first))
      answer[cci.first.first] = ics::ArrayMap<std::string,int>();
    answer[cci.first.first][cci.first.second] = cci.second;
  }

  return answer;
}
#endif /* SOLUTION_HPP_ */
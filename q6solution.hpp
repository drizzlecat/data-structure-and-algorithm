#ifndef Q6SOLUTION_HPP_
#define Q6SOLUTION_HPP_


#include <iostream>
#include <exception>
#include <fstream>
#include <sstream>
#include <algorithm>                 // std::swap
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "q6utility.hpp"


////////////////////////////////////////////////////////////////////////////////

//Problem 1

//Write this function
template<class T>
void selection_sort(LN<T>* l) {
  for (LN<T>* c = l; c!=nullptr; c=c->next) {
    LN<T>* smallest = c;
    for (LN<T>* s=smallest->next; s!=nullptr; s=s->next)
      if (s->value < smallest->value)
        smallest = s;
    std::swap(c->value,smallest->value);
  }
}


////////////////////////////////////////////////////////////////////////////////

//Problem 2

//Write this function
template<class T>
void merge(T a[], int left_low,  int left_high,
           int right_low, int right_high) {
  int length = right_high-left_low+1;
  T temp[length];
  int left  = left_low;
  int right = right_low;
  for (int i = 0; i< length; ++i) {
    if (left > left_high)
      temp[i] = a[right++];
    else if (right > right_high)
      temp[i] = a[left++];
    else if (a[left] <= a[right])
      temp[i] = a[left++];
    else
      temp[i] = a[right++];
  }
  for (int i=0; i< length; ++i)
    a[left_low++] = temp[i];
}


////////////////////////////////////////////////////////////////////////////////

//Problem 3

int select_digit (int number, int place)
{return number/place % 10;}


//Write this function
void radix_sort(int a[], int length) {
  ics::ArrayQueue<int> buckets[10];
  for (int place = 1; place<=100000; place=place*10) {
    for (int i=0; i<length; i++)
      buckets[select_digit(a[i],place)].enqueue(a[i]);
    int i=0;
    for (int b=0; b<10; b++)
      while (!buckets[b].empty())
        a[i++] = buckets[b].dequeue();
  }
}


////////////////////////////////////////////////////////////////////////////////

//Problem 4

//Modify this function

template<class T>
int selection_sort(T a[], int length) {
  int needed_swaps = 0;
  for (int index_to_update=0; index_to_update<length; ++index_to_update) {
    int index_of_min = index_to_update;
    for (int i = index_to_update+1; i<length; ++i)
      if ( a[i] < a[index_of_min] )
        index_of_min = i;
    if (index_to_update != index_of_min)
      ++needed_swaps;
    std::swap(a[index_to_update], a[index_of_min]);
  }
  return needed_swaps;
}


//Write this function

//Test how often in selection_sort the index_to_update and index_of_min are.
//  different when swapping.
//Test is on an array of the the values 0..length-1, randomly shuffled num_tests times
//Returns the average number of different indexes.
double test_swapping(int length, int num_tests) {
  int test_array[length];
  for (int i=0; i <length; ++i)
    test_array[i] = i;

  int sum_swaps = 0;

  for (int i=0; i<num_tests; ++i) {
    shuffle(test_array,length);
    int swaps = selection_sort(test_array,length);
    sum_swaps += swaps;
  }
  return double(sum_swaps)/num_tests/length;
}


#endif /* Q6SOLUTION_HPP_ */

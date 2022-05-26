#ifndef LINKED_PRIORITY_QUEUE_HPP_
#define LINKED_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "array_stack.hpp"      //See operator <<


namespace ics {


#ifndef undefinedgtdefined
#define undefinedgtdefined
    template<class T>
    bool undefinedgt (const T& a, const T& b) {return false;}
#endif /* undefinedgtdefined */

//Instantiate the templated class supplying tgt(a,b): true, iff a has higher priority than b.
//If tgt is defaulted to undefinedgt in the template, then a constructor must supply cgt.
//If both tgt and cgt are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedgt value supplied by tgt/cgt is stored in the instance variable gt.
template<class T, bool (*tgt)(const T& a, const T& b) = undefinedgt<T>> class LinkedPriorityQueue {
  public:
    //Destructor/Constructors
    ~LinkedPriorityQueue();

    LinkedPriorityQueue          (bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    LinkedPriorityQueue          (const LinkedPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    explicit LinkedPriorityQueue (const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedPriorityQueue (const Iterable& i, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    LinkedPriorityQueue<T,tgt>& operator = (const LinkedPriorityQueue<T,tgt>& rhs);
    bool operator == (const LinkedPriorityQueue<T,tgt>& rhs) const;
    bool operator != (const LinkedPriorityQueue<T,tgt>& rhs) const;

    template<class T2, bool (*gt2)(const T2& a, const T2& b)>
    friend std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T2,gt2>& pq);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedPriorityQueue<T,tgt>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedPriorityQueue<T,tgt>::Iterator& operator ++ ();
        LinkedPriorityQueue<T,tgt>::Iterator  operator ++ (int);
        bool operator == (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const;
        bool operator != (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T,tgt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedPriorityQueue<T,tgt>::begin () const;
        friend Iterator LinkedPriorityQueue<T,tgt>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev;            //prev should be initalized to the header
        LN*             current;         //current == prev->next
        LinkedPriorityQueue<T,tgt>* ref_pq;
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedPriorityQueue<T,tgt>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next = nullptr;
    };


    bool (*gt) (const T& a, const T& b); // The gt used by enqueue (from template or constructor)
    LN* front     =  new LN();
    int used      =  0;                  //Cache for number of values in linked list
    int mod_count =  0;                  //For sensing concurrent modification

    //Helper methods
    void delete_list(LN*& front);        //Deallocate all LNs, and set front's argument to nullptr;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedPriorityQueue class and related definitions

//Destructor/Constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::~LinkedPriorityQueue() {
  delete_list(front); //Including header node
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(bool (*cgt)(const T& a, const T& b))
: gt(tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> ? tgt : cgt) {
  if (gt == (bool (*)(const T& a, const T& b))undefinedgt<T>) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::default constructor: neither specified");
  }
  if (tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && cgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && tgt != cgt) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::default constructor: both specified");
  }
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const LinkedPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b))
: gt(tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> ? tgt : cgt) {
  if (gt == (bool (*)(const T& a, const T& b))undefinedgt<T>)
    gt = to_copy.gt;//throw TemplateFunctionError("LinkedPriorityQueue::copy constructor: neither specified");
  if (tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && cgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && tgt != cgt) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::copy constructor: both specified");
  }
  if (gt == to_copy.gt) {
    used = to_copy.used;
    for (LN* t = front, *f = to_copy.front->next; f != nullptr; t = t->next, f = f->next)
      t->next = new LN(f->value);
  }else
    for (LN* p = to_copy.front->next; p != nullptr; p = p->next)
      enqueue(p->value);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b))
: gt(tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> ? tgt : cgt) {
  if (gt == (bool (*)(const T& a, const T& b))undefinedgt<T>) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::initializer_list constructor: neither specified");
  }
if (tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && cgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && tgt != cgt) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::initializer_list constructor: both specified");
  }

  for (const T& q_elem : il)
    enqueue(q_elem);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template<class Iterable>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const Iterable& i, bool (*cgt)(const T& a, const T& b))
: gt(tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> ? tgt : cgt) {
  if (gt == (bool (*)(const T& a, const T& b))undefinedgt<T>) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::iterable constructor: neither specified");
  }
if (tgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && cgt != (bool (*)(const T& a, const T& b))undefinedgt<T> && tgt != cgt) {
    delete front; //delete allocated header node to avoid memory leak
    throw TemplateFunctionError("LinkedPriorityQueue::iterable constructor: both specified");
  }

  for (const T& v : i)
    enqueue(v);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::empty() const {
  return used == 0;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int LinkedPriorityQueue<T,tgt>::size() const {
  return used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& LinkedPriorityQueue<T,tgt>::peek () const {
  if (this->empty())
    throw EmptyError("LinkedPriorityQueue::peek");

  return front->next->value;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string LinkedPriorityQueue<T,tgt>::str() const {
  std::ostringstream answer;
  answer << "LinkedPriorityQueue[HEADER";

  if (used != 0) {
    answer << "->" << front->next->value;
    for (LN* p = front->next->next; p != nullptr; p = p->next)
      answer << "->" << p->value;
  }

  answer << "](used=" << used << ",front=" << front << ",mod_count=" << mod_count << ")";
  return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, bool (*tgt)(const T& a, const T& b)>
int LinkedPriorityQueue<T,tgt>::enqueue(const T& element) {
  LN* p = front;
  while (p->next != nullptr && gt(p->next->value,element))
    p = p->next;
  p->next = new LN(element,p->next);
  ++used;
  ++mod_count;
  return 1;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T LinkedPriorityQueue<T,tgt>::dequeue() {
  if (this->empty())
    throw EmptyError("LinkedPriorityQueue::dequeue");

  T answer = front->next->value;
  LN* to_delete = front->next;
  front->next = to_delete->next;
  delete to_delete;
  --used;
  ++mod_count;
  return answer;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
void LinkedPriorityQueue<T,tgt>::clear() {
  delete_list(front->next);
  used = 0;
  ++mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template <class Iterable>
int LinkedPriorityQueue<T,tgt>::enqueue_all (const Iterable& i) {
  int count = 0;
  for (const T& v : i)
     count += enqueue(v);

    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>& LinkedPriorityQueue<T,tgt>::operator = (const LinkedPriorityQueue<T,tgt>& rhs) {
  if (this == &rhs)
    return *this;

  gt = rhs.gt;   // if tgt != nullptr, gts are already equal (or compiler error)
  //Reuse this->front LNs (otherwise create new LNs if not enough)
  used = rhs.used;
  LN** to = &(front->next);
  for (LN* p = rhs.front->next; p != nullptr; to = &((*to)->next), p = p->next)
    if (*to != nullptr)
      (*to)->value = p->value;
    else
      *to = new LN(p->value);
  //Delete all remaining LNs (if this->front had more than rhs)
  if (*to != nullptr)
    delete_list(*to);

  ++mod_count;
  return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::operator == (const LinkedPriorityQueue<T,tgt>& rhs) const {
  if (this == &rhs)
    return true;
  int used = this->size();
  if (used != rhs.size())
    return false;
  if (gt != rhs.gt) //For PriorityQueues to be equal, they need the same gt function, and values
    return false;
  LinkedPriorityQueue<T,tgt>::Iterator rhs_i = rhs.begin();
  for (LN* p = front->next; p != nullptr; p = p->next,++rhs_i)
    if (p->value != *rhs_i)
      return false;

  return true;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::operator != (const LinkedPriorityQueue<T,tgt>& rhs) const {
  return !(*this == rhs);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T,tgt>& pq) {
  outs << "priority_queue[";

  if (!pq.empty()) {
    ArrayStack<T> st;
    for (typename LinkedPriorityQueue<T,tgt>::LN* p = pq.front->next; p != nullptr; p = p->next)
      st.push(p->value);
    outs << st.pop();
    while (!st.empty())
      outs << "," << st.pop();
    }

  outs <<"]:highest";
  return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::begin () const -> LinkedPriorityQueue<T,tgt>::Iterator {
  return Iterator(const_cast<LinkedPriorityQueue<T,tgt>*>(this),front->next);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::end () const -> LinkedPriorityQueue<T,tgt>::Iterator {
  return Iterator(const_cast<LinkedPriorityQueue<T,tgt>*>(this),nullptr);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, bool (*tgt)(const T& a, const T& b)>
void LinkedPriorityQueue<T,tgt>::delete_list(LN*& front) {
  for (LN* p = front; p != nullptr; /*see body*/) {
    LN* to_delete = p;
    p = p->next;
    delete to_delete;
  }
  front = nullptr;
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::Iterator::Iterator(LinkedPriorityQueue<T,tgt>* iterate_over, LN* initial)
: prev(iterate_over->front), current(initial), ref_pq(iterate_over), expected_mod_count(ref_pq->mod_count) {
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::Iterator::~Iterator()
{}


template<class T, bool (*tgt)(const T& a, const T& b)>
T LinkedPriorityQueue<T,tgt>::Iterator::erase() {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::erase");
  if (!can_erase)
    throw CannotEraseError("LinkedPriorityQueue::Iterator::erase Iterator cursor already erased");
  if (current == nullptr)
    throw CannotEraseError("LinkedPriorityQueue::Iterator::erase Iterator cursor beyond data structure");

  can_erase = false;
  T to_return = current->value;

  prev->next = current->next;
  delete current;
  current = prev->next;

  --ref_pq->used;
  expected_mod_count = ++ref_pq->mod_count;
  return to_return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::string LinkedPriorityQueue<T,tgt>::Iterator::str() const {
  std::ostringstream answer;
  answer << ref_pq->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
  return answer.str();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::Iterator::operator ++ () -> LinkedPriorityQueue<T,tgt>::Iterator& {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ++");

  if (current == nullptr)
    return *this;

  if (can_erase) {
    prev = current;
    current = current->next;
  }else
    can_erase = true;

  return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::Iterator::operator ++ (int) -> LinkedPriorityQueue<T,tgt>::Iterator {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ++(int)");

  if (current == nullptr)
    return *this;

  Iterator to_return(*this);

  if (can_erase) {
    prev = current;
    current = current->next;
  }else
    can_erase = true;

  return to_return;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::Iterator::operator == (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("LinkedPriorityQueue::Iterator::operator ==");
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ==");
  if (ref_pq != rhsASI->ref_pq)
    throw ComparingDifferentIteratorsError("LinkedPriorityQueue::Iterator::operator ==");

  return current == rhsASI->current;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::Iterator::operator != (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const {
  const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
  if (rhsASI == 0)
    throw IteratorTypeError("LinkedPriorityQueue::Iterator::operator !=");
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator !=");
  if (ref_pq != rhsASI->ref_pq)
    throw ComparingDifferentIteratorsError("LinkedPriorityQueue::Iterator::operator !=");

  return current != rhsASI->current;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
T& LinkedPriorityQueue<T,tgt>::Iterator::operator *() const {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator *");
  if (!can_erase || current == nullptr) {
    std::ostringstream where;
    where << current
          << " when front = " << ref_pq->front;
    throw IteratorPositionIllegal("LinkedPriorityQueue::Iterator::operator * Iterator illegal: "+where.str());
  }

  return current->value;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
T* LinkedPriorityQueue<T,tgt>::Iterator::operator ->() const {
  if (expected_mod_count != ref_pq->mod_count)
    throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator *");
  if (!can_erase || current == nullptr) {
    std::ostringstream where;
    where << current
          << " when front = " << ref_pq->front;
    throw IteratorPositionIllegal("LinkedPriorityQueue::Iterator::operator * Iterator illegal: "+where.str());
  }

  return &(current->value);
}


}

#endif /* LINKED_PRIORITY_QUEUE_HPP_ */

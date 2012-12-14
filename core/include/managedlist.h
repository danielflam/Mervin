#ifndef _MANAGEDLIST_H
#define _MANAGEDLIST_H

#include <algorithm>

template <class T>
struct managedList 
{
public:
	typename std::vector<T*> list;
	typedef typename std::vector<T*>::iterator iterator; 
	typedef typename std::vector<T*>::const_iterator const_iterator; 
	typedef typename std::vector<T*>::reverse_iterator reverse_iterator; 
	typedef typename std::vector<T*>::const_reverse_iterator const_reverse_iterator; 

	~managedList()
	{
		clear();
	}

	void clear()
	{
		for (std::vector<T*>::iterator it = list.begin(); it!= list.end(); it++)
		{
			(*it)->release();
		}
		list.clear();
	}

	iterator remove(T* p)
	{
		iterator res = list.end();
		iterator it = std::find(list.begin(), list.end(), p);
		if (p)
		{
			res = list.erase(it);
			p->release();
		}

		return res;
	}

	iterator remove(iterator it)
	{
		T * p = *it;
		iterator res = list.erase(it);
		p->release();
		return res;
	}

	void erase(int i)
	{
		T* p = list[i];
		list[i] = 0;
		p->release();
	}

	void push_back(T* p)
	{
		list.push_back(p);
	}
	
	void pop_front()
	{
		T * p = list.front();
		if (p)
			remove(p);
	}

	T & front()
	{
		return *(list.front());
	}

	T & back()
	{
		return *(list.back());
	}

	unsigned int size()
	{
		return list.size();
	}



	template<class Pr1>
	void erase_if( Pr1 Pred)
	{	// erase each element satisfying _Pr1
		
		iterator last = list.end();
		for (iterator it = begin(); it != last; it++)
		{
			if (Pred(*it))
			{
				(*it)->release();
			}
		}

		iterator it = std::remove_if(list.begin(), list.end(), Pred);
		list.erase(it, list.end());
	}

	inline std::vector<T*> getList(){return list;}

	T * operator[](int index){return list[index];}



	iterator begin(){ return list.begin();}
	iterator end(){ return list.end();}
	reverse_iterator rbegin(){ return list.rbegin;}
	reverse_iterator rend(){ return list.rend();}

};


#endif
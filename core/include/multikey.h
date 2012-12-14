#ifndef _MULTIKEY_H
#define _MULTIKEY_H

// The single entity case is handled by default by the STL
/*

Usage:

typedef MultiKey<int, double, int> SomeKey;

std::map<SomeKey, string> MyIndex;

MyIndex[SomeKey(2,3.2,3)] = "hello";


*/

template <typename T1, typename T2>
class MultiKey2 {
  public:
    
    MultiKey2(T1 k1, T2 k2)
      : key1(k1), key2(k2) {}  

    bool operator<(const MultiKey2 &right) const 
    {
        if ( key1 == right.key1 ) {
                return key2 < right.key2;
        }
        else {
            return key1 < right.key1;
        }
    }

    MultiKey2 & operator=(const MultiKey2 &right) const 
    {
		key1 = right.key1;
		key2 = right.key2;

		return *this;
    }

    T1 key1;
    T2 key2;
};


template <typename T1, typename T2, typename T3>
class MultiKey3 {
  public:
    
    MultiKey3(T1 k1, T2 k2, T3 k3)
      : key1(k1), key2(k2), key3(k3) {}  

    MultiKey3 & operator=(const MultiKey3 &right) const 
    {
		key1 = right.key1;
		key2 = right.key2;
		key3 = right.key3;

		return *this;
    }

    bool operator<(const MultiKey3 &right) const 
    {
        if ( key1 == right.key1 ) {
            if ( key2 == right.key2 ) {
                    return key3 < right.key3;
            }
            else {
                return key2 < right.key2;
            }
        }
        else {
            return key1 < right.key1;
        }
    }

	T1  key1;
    T2  key2;
    T3  key3;

};

template <typename T1, typename T2, typename T3, typename T4>
class MultiKey4 {
  public:
    
    MultiKey4(T1 k1, T2 k2, T3 k3, T4 k4)
      : key1(k1), key2(k2), key3(k3), key4(k4) {}  

    MultiKey4 & operator=(const MultiKey4 &right) const 
    {
		key1 = right.key1;
		key2 = right.key2;
		key3 = right.key3;
		key4 = right.key4;

		return *this;
    }

    bool operator<(const MultiKey4 &right) const 
    {
        if ( key1 == right.key1 ) {
            if ( key2 == right.key2 ) {
                if ( key3 == right.key3 ) {
                    return key4 < right.key4;
                }
                else {
                    return key3 < right.key3;
                }
            }
            else {
                return key2 < right.key2;
            }
        }
        else {
            return key1 < right.key1;
        }
    }

    T1  key1;
    T2  key2;
    T3  key3;
    T4  key4;

};

template <class T1, class T2, class T3, class T4, class T5>
class MultiKey5 {
  public:
    
    MultiKey5(T1 k1, T2 k2, T3 k3, T4 k4, T5 k5)
      : key1(k1), key2(k2), key3(k3), key4(k4), key5(k5) {}  

    MultiKey5 & operator=(const MultiKey5 &right) const 
    {
		key1 = right.key1;
		key2 = right.key2;
		key3 = right.key3;
		key4 = right.key4;
		key5 = right.key5;

		return *this;
    }

    bool operator<(const MultiKey5 &right) const 
    {
        if ( key1 == right.key1 ) {
            if ( key2 == right.key2 ) {
                if ( key3 == right.key3 ) {
					if (key4 == right.key4 ) {
						return key5 < right.key5
					}
					else {
						return key4 < right.key4;
					}
                }
                else {
                    return key3 < right.key3;
                }
            }
            else {
                return key2 < right.key2;
            }
        }
        else {
            return key1 < right.key1;
        }
    }   

    T1  key1;
    T2  key2;
    T3  key3;
    T4  key4;
	T5  key5;

};

#endif
#ifndef LCDF_HASHMAP_HH
#define LCDF_HASHMAP_HH
#include <assert.h>
#include <lcdf/hashcode.hh>

// K AND V REQUIREMENTS:
//
//		K::K()
//		K::operator bool() const
//			Must have (bool)(K()) == false
//			and no k with (bool)k == false is stored.
// K &		K::operator=(const K &)
//		k1 == k2
// hashcode_t	hashcode(const K &)
//			If hashcode(k1) != hashcode(k2), then k1 != k2.
//			Actually any unsigned integer type may be used.
//
//		V::V()
// V &		V::operator=(const V &)

template <class K, class V> class _HashMap_const_iterator;
template <class K, class V> class _HashMap_iterator;

template <class K, class V>
class HashMap { public:

    HashMap();
    explicit HashMap(const V &);
    HashMap(const HashMap<K, V> &);
    ~HashMap()				{ delete[] _e; }

    int size() const			{ return _n; }
    bool empty() const			{ return _n == 0; }
    int capacity() const		{ return _capacity; }
    const V &default_value() const	{ return _default_value; }
    void set_default_value(const V &v)	{ _default_value = v; }

    typedef _HashMap_const_iterator<K, V> const_iterator;
    typedef _HashMap_iterator<K, V> iterator;

    inline const_iterator begin() const;
    inline iterator begin();
    inline const_iterator end() const;
    inline iterator end();

    inline const V &find(const K &) const;
    inline V *findp(const K &) const;
    inline const V &operator[](const K &k) const;
    V &find_force(const K &, const V &);
    inline V &find_force(const K &);

    bool insert(const K &, const V &);
    void clear();

    HashMap<K, V> &operator=(const HashMap<K, V> &);
    void swap(HashMap<K, V> &);

    void resize(int size)		{ increase(size); }

    struct Pair {
	K key;
	V value;
	Pair()				: key(), value() { }
    };

  private:

    int _capacity;
    int _grow_limit;
    int _n;
    Pair *_e;
    V _default_value;

    void increase(int);
    inline void check_capacity();
    inline int bucket(const K &) const;

    friend class _HashMap_const_iterator<K, V>;
    friend class _HashMap_iterator<K, V>;

};

template <class K, class V>
class _HashMap_const_iterator { public:
    typedef _HashMap_const_iterator const_iterator;
    typedef typename HashMap<K, V>::Pair Pair;

    operator bool() const		{ return _pos < _hm->_capacity; }
    bool operator!() const		{ return _pos >= _hm->_capacity; }

    void operator++(int);
    void operator++()			{ (*this)++; }

    const K &key() const		{ return _hm->_e[_pos].key; }
    const V &value() const		{ return _hm->_e[_pos].value; }
    const Pair &pair() const		{ return _hm->_e[_pos]; }

    inline bool operator==(const const_iterator &) const;
    inline bool operator!=(const const_iterator &) const;

  private:
    const HashMap<K, V> *_hm;
    int _pos;
    _HashMap_const_iterator(const HashMap<K, V> *, int);
    friend class HashMap<K, V>;
    friend class _HashMap_iterator<K, V>;
};

template <class K, class V>
class _HashMap_iterator : public _HashMap_const_iterator<K, V> { public:
    typedef _HashMap_iterator iterator;

    V &value() const		{ return this->_hm->_e[this->_pos].value; }

  private:
    _HashMap_iterator(const HashMap<K, V> *hm, int pos) : _HashMap_const_iterator<K, V>(hm, pos) { }
    friend class HashMap<K, V>;
};


template <class K, class V>
inline int
HashMap<K, V>::bucket(const K &key) const
{
    assert(key);
    hashcode_t hc = hashcode(key);
    int i =   hc       & (_capacity - 1);
    int j = ((hc >> 6) & (_capacity - 1)) | 1;

    while (_e[i].key && !(_e[i].key == key))
	i = (i + j) & (_capacity - 1);

    return i;
}

template <class K, class V>
inline const V &
HashMap<K, V>::find(const K &key) const
{
    int i = bucket(key);
    const V *v = (_e[i].key ? &_e[i].value : &_default_value);
    return *v;
}

template <class K, class V>
inline const V &
HashMap<K, V>::operator[](const K &key) const
{
    return find(key);
}

template <class K, class V>
inline V *
HashMap<K, V>::findp(const K &key) const
{
    int i = bucket(key);
    return _e[i].key ? &_e[i].value : 0;
}

template <class K, class V>
inline V &
HashMap<K, V>::find_force(const K &key)
{
    return find_force(key, _default_value);
}

template <class K, class V>
inline _HashMap_const_iterator<K, V>
HashMap<K, V>::begin() const
{
    return const_iterator(this, 0);
}

template <class K, class V>
inline _HashMap_const_iterator<K, V>
HashMap<K, V>::end() const
{
    return const_iterator(this, _capacity);
}

template <class K, class V>
inline _HashMap_iterator<K, V>
HashMap<K, V>::begin()
{
    return iterator(this, 0);
}

template <class K, class V>
inline _HashMap_iterator<K, V>
HashMap<K, V>::end()
{
    return iterator(this, _capacity);
}

template <class K, class V>
inline bool
_HashMap_const_iterator<K, V>::operator==(const const_iterator &i) const
{
    return _hm == i._hm && _pos == i._pos;
}

template <class K, class V>
inline bool
_HashMap_const_iterator<K, V>::operator!=(const const_iterator &i) const
{
    return _hm != i._hm || _pos != i._pos;
}

#include <lcdf/hashmap.cc>	// necessary to support GCC 3.3
#endif

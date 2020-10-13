#pragma once

#include<unordered_map>
#include<vector>
#include<stdint.h>

#define SIMPLEPOOL_ALLOC_MINSIZE 100
#define SIMPLEPOOL_ALLOC_MAXSIZE 256

template<class T>
class SimplePool
{
private:
	std::unordered_map<uint64_t, T*> poolUsing;
	std::vector<T*> poolFree;
public:
	~SimplePool();
	T* CreateObj(uint64_t objId);
	bool ReleaseObj(uint64_t objId);
	T* FindObj(uint64_t objId);
	void Destroy();

private:
	void AllocObjs();
	void FreeObjs();
};

template<class T>
inline SimplePool<T>::~SimplePool()
{
	Destroy();
}

template<class T>
inline T* SimplePool<T>::CreateObj(uint64_t objId)
{
	auto it = poolUsing.find(objId);
	if (it != poolUsing.end())
	{
		return it->second;
	}
	if (poolFree.empty())
	{
		AllocObjs();
	}
	if (poolFree.empty())
	{
		return nullptr;
	}
	auto itFree = poolFree.back();
	poolFree.pop_back();
	T* ret = itFree;
	ret->id = objId;
	poolUsing[objId] = ret;

	return ret;
}

template<class T>
inline bool SimplePool<T>::ReleaseObj(uint64_t objId)
{
	auto it = poolUsing.find(objId);
	if (it == poolUsing.end())
	{
		return false;
	}

	if (poolFree.size() > SIMPLEPOOL_ALLOC_MAXSIZE * 2)
	{
		FreeObjs();
	}
	it->second->id = 0;
	poolFree.push_back(it->second);
	poolUsing.erase(it);

	return false;
}

template<class T>
inline T* SimplePool<T>::FindObj(uint64_t objId)
{
	auto it = poolUsing.find(objId);
	if (it != poolUsing.end())
	{
		return it->second;
	}
	return nullptr;
}

template<class T>
inline void SimplePool<T>::Destroy()
{
	for (auto& obj : poolFree)
	{
		if (obj != nullptr)
		{
			delete obj;
			obj = nullptr;
		}
	}
	poolFree.clear();

	for (auto it : poolUsing)
	{
		if (it.second != nullptr)
		{
			delete it.second;
			it.second = nullptr;
		}
	}
	poolUsing.clear();
}

template<class T>
inline void SimplePool<T>::AllocObjs()
{
	if (poolFree.empty())
	{
		// Alloc
		auto allocSize = poolUsing.size();
		if (allocSize > 0)
		{
			allocSize /= 10;
		}
		if (allocSize < SIMPLEPOOL_ALLOC_MINSIZE)
		{
			allocSize = SIMPLEPOOL_ALLOC_MINSIZE;
		}
		if (allocSize > SIMPLEPOOL_ALLOC_MAXSIZE)
		{
			allocSize = SIMPLEPOOL_ALLOC_MAXSIZE;
		}
		for (auto i = 0; i < allocSize; i++)
		{
			T* obj = new T();
			obj->id = 0;
			poolFree.push_back(obj);
		}
	}
}

template<class T>
inline void SimplePool<T>::FreeObjs()
{
	if (poolFree.size() >= SIMPLEPOOL_ALLOC_MAXSIZE * 2)
	{
		for (size_t i = 0; i < poolFree.size() - SIMPLEPOOL_ALLOC_MINSIZE; i++)
		{
			auto pobj = poolFree.back();
			poolFree.pop_back();
			delete pobj;
		}
	}
}

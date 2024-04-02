#pragma once
#include "Types.h"
#include "Allocator.h"
#include <array>
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

/// +----------------------------
///			 ARRAY
/// ----------------------------+
template<typename Type, uint32 Size>
using NetArray = std::array<Type, Size>;

/// +----------------------------
///			 VECTOR
/// ----------------------------+
template<typename Type>
using NetVector = std::vector<Type, StlAllocator<Type>>;

/// +----------------------------
///			 LIST
/// ----------------------------+
template<typename Type>
using NetList = std::list<Type, StlAllocator<Type>>;

/// +----------------------------
///			 MAP
/// ----------------------------+
template<typename Key, typename Type, typename Pred = std::less<Key>>
using NetMap = std::map<Key, Type, Pred, StlAllocator<std::pair<const Key, Type>>>;

/// +----------------------------
///			 SET
/// ----------------------------+
template<typename Key, typename Pred = std::less<Key>>
using NetSet = std::set<Key, Pred, StlAllocator<Key>>;

/// +----------------------------
///			 DEQUE
/// ----------------------------+
template<typename Type>
using NetDeque = std::deque<Type, StlAllocator<Type>>;

/// +----------------------------
///			 QUEUE
/// ----------------------------+
template<typename Type, typename Container = NetDeque<Type>>
using NetQueue = std::queue<Type, Container>;

/// +----------------------------
///			 STACK
/// ----------------------------+
template<typename Type, typename Container = NetDeque<Type>>
using NetStack = std::stack<Type, Container>;

/// +----------------------------
///			 PRIORITY QUEUE
/// ----------------------------+
template<typename Type, typename Container = NetVector<Type>, typename Pred = std::less<typename Container::value_type>>
using NetPriorityQueue = std::priority_queue<Type, Container, Pred>;

/// +----------------------------
///			 STRING / WSTRING
/// ----------------------------+
using NetString = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;

using NetWString = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;

/// +----------------------------
///			 HASH MAP
/// ----------------------------+
template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using NetHashMap = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

/// +----------------------------
///			 HASH SET 
/// ----------------------------+
template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using NetHashSet = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;
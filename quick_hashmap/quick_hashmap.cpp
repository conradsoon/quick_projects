#include <type_traits>
#include <functional>
#include <concepts>
#include <vector>
#include <list>
#include <utility>
#include <optional>
#include <cassert>
#include <iostream>
#include <random>

template <typename T>
concept Hashable = requires(T t) {
	{
		std::hash<T>{}(t)
	} -> std::convertible_to<std::size_t>;
};

template <typename K, typename V>
	requires Hashable<K> && std::equality_comparable<V>
class QuickHashMap
{
public:
	static_assert(Hashable<K>, "Key type must be hashable!");
	static_assert(std::equality_comparable<V>, "Key type must be equality comparable!");
	QuickHashMap(size_t cap)
	{
		this->sz = 0;
		this->cap = cap;
		store.resize(cap);
	};
	QuickHashMap() : QuickHashMap(1024){};
	~QuickHashMap(){};
	void insert(K key, V value)
	{
		if (this->sz == this->cap)
		{
			this->resize(this->cap * 2);
		}
		for (auto &pair : store[this->getPos(key)])
		{
			if (pair.first == key)
			{
				pair.second = value;
				return;
			}
		}
		size_t pos = this->getPos(key);
		store[pos].push_back({key, value});
		this->sz++;
	};

	void erase(K key)
	{
		size_t pos = this->getPos(key);
		for (auto itr = store[pos].begin(); itr != store[pos].end(); itr++)
		{
			if (itr->first == key)
			{
				store[pos].erase(itr);
				this->sz--;
				return;
			}
		}
	};

	bool has(K key)
	{
		size_t pos = this->getPos(key);
		for (auto &pair : store[pos])
		{
			if (pair.first == key)
			{
				return true;
			}
		}
		return false;
	};

	std::optional<V> get(const K &key)
	{
		size_t pos = this->getPos(key);
		// iterate through the list and check if the value is there.
		for (auto &pair : store[pos])
		{
			if (pair.first == key)
			{
				return std::optional<V>{pair.second};
			}
		}
		return std::optional<V>();
	};

	void resize(size_t new_size)
	{
		std::vector<std::list<std::pair<K, V>>> newStore(new_size);
		for (const auto &list : store)
		{
			for (const auto &pair : list)
			{
				size_t new_pos = std::hash<K>{}(pair.first) % new_size;
				newStore[new_pos].push_back(pair);
			}
		}
		store.swap(newStore);
		cap = new_size;
	}

	size_t size()
	{
		return this->sz;
	}
	size_t capacity()
	{
		return this->cap;
	}

private:
	size_t getPos(const K &key)
	{
		return std::hash<K>{}(key) % this->cap;
	}
	size_t sz;
	size_t cap;
	std::vector<std::list<std::pair<K, V>>> store;
};

void testInsertAndGet()
{
	QuickHashMap<int, std::string> map;

	map.insert(1, "one");
	map.insert(2, "two");
	map.insert(3, "three");

	assert(map.get(1).has_value());
	assert(map.get(1).value() == "one");
	assert(map.get(2).value() == "two");
	assert(map.get(3).value() == "three");
	assert(!map.get(4).has_value());
}

void testHas()
{
	QuickHashMap<int, std::string> map;

	map.insert(5, "five");
	map.insert(6, "six");

	assert(map.has(5));
	assert(!map.has(7));
}

void testErase()
{
	QuickHashMap<int, std::string> map;

	map.insert(8, "eight");
	assert(map.has(8));

	map.erase(8);
	assert(!map.has(8));
}

void testResize()
{
	QuickHashMap<int, std::string> map(2);

	map.insert(9, "nine");
	map.insert(10, "ten");
	map.insert(11, "eleven"); // This should trigger a resize

	assert(map.capacity() > 2);
	assert(map.get(9).value() == "nine");
	assert(map.get(10).value() == "ten");
	assert(map.get(11).value() == "eleven");
}

void testSizeAndCapacity()
{
	QuickHashMap<int, std::string> map;

	assert(map.size() == 0);
	assert(map.capacity() == 1024); // Default capacity

	map.insert(12, "twelve");
	assert(map.size() == 1);

	map.erase(12);
	assert(map.size() == 0);
}

void testResizing()
{
	const size_t initialCapacity = 10;
	QuickHashMap<int, std::string> map(initialCapacity);
	for (size_t i = 0; i < initialCapacity * 2; ++i)
	{
		map.insert(i, "value" + std::to_string(i));
	}
	assert(map.capacity() > initialCapacity);
	for (size_t i = 0; i < initialCapacity * 2; ++i)
	{
		assert(map.has(i));
	}
}

void testRandomized()
{
	const size_t numOperations = 1000;
	const double insertProbability = 0.7; // 70% chance to insert
	const double removeProbability = 0.2; // 20% chance to remove, 10% chance to do nothing

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);
	std::uniform_int_distribution<> valueDist(0, 10000);

	QuickHashMap<int, std::string> testMap;
	std::unordered_map<int, std::string> referenceMap;

	for (size_t i = 0; i < numOperations; ++i)
	{
		double prob = dis(gen);
		int key = valueDist(gen);
		std::string value = "value" + std::to_string(valueDist(gen));

		if (prob < insertProbability)
		{
			testMap.insert(key, value);
			referenceMap[key] = value;
		}
		else if (prob < insertProbability + removeProbability)
		{
			testMap.erase(key);
			referenceMap.erase(key);
		}
		assert(testMap.size() == referenceMap.size());
		for (const auto &[k, v] : referenceMap)
		{
			assert(testMap.has(k));
		}
	}
}

int main()
{
	std::cout << "Running testInsertAndGet..." << std::endl;
	testInsertAndGet();
	std::cout << "testInsertAndGet passed!" << std::endl;

	std::cout << "Running testHas..." << std::endl;
	testHas();
	std::cout << "testHas passed!" << std::endl;

	std::cout << "Running testErase..." << std::endl;
	testErase();
	std::cout << "testErase passed!" << std::endl;

	std::cout << "Running testResize..." << std::endl;
	testResize();
	std::cout << "testResize passed!" << std::endl;

	std::cout << "Running testSizeAndCapacity..." << std::endl;
	testSizeAndCapacity();
	std::cout << "testSizeAndCapacity passed!" << std::endl;

	std::cout << "Running testRandomized..." << std::endl;
	testRandomized();
	std::cout << "testRandomized passed!" << std::endl;

	std::cout << "All tests passed successfully!" << std::endl;
	return 0;
}

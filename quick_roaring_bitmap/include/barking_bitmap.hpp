#ifndef BARKING_BITMAP_HPP
#define BARKING_BITMAP_HPP

#include <vector>
#include <variant>
#include <bitset>
#include <ranges>

#define BB_BUCKET_SZ 65536
#define BB_BSET_SZ 65536
#define BB_ARRAY_THRESHOLD 4096

// using bb_array = std::array<int, BB_SZ>;
using bb_array = std::vector<uint16_t>;
using bb_bset = std::bitset<BB_BSET_SZ>;
using bb_variant = std::variant<bb_array, bb_bset>;

static auto convert_bset_to_array(bb_bset const &bset) -> bb_array
{
	bb_array result;
	result.reserve(BB_ARRAY_THRESHOLD);
	for (int i = 0; i < BB_BSET_SZ; i++)
	{
		if (bset.test(i))
		{
			result.push_back(i);
		}
	}
	return result;
}

static auto convert_array_to_bset(bb_array const &array) -> bb_bset
{
	bb_bset result;
	for (auto const &i : array)
	{
		result.set(i);
	}
	return result;
}

// is this the best semantic for this?
static auto convertForCardinality(bb_variant const &data) -> bb_variant
{
	return std::visit([](auto &&arg) -> bb_variant
					  {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, bb_array>)
		{
			if(arg.size() >= BB_ARRAY_THRESHOLD)
			{
				return convert_array_to_bset(arg);
			}
			return arg;
		}
		else if constexpr (std::is_same_v<T, bb_bset>)
		{
			if(arg.count() < BB_ARRAY_THRESHOLD)
			{
				return convert_bset_to_array(arg);
			}
			return arg;
		} },
					  data);
}

class BBData
{
public:
	BBData() : sz(0), data(bb_array()){};
	bb_variant data;
	size_t sz;
	void add(const uint16_t value)
	{
		std::visit([value, this](auto &&arg)
				   {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, bb_array>)
			{
				arg.push_back(value);
				
			}
			else if constexpr (std::is_same_v<T, bb_bset>)
			{
				arg.set(value);
			} },
				   data);
	}
	void clear()
	{
		data = bb_array();
	}
	void remove(uint16_t value)
	{
		std::visit([this, value](auto &&arg)
				   {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, bb_array>)
			{
				arg.erase(std::remove(arg.begin(), arg.end(), value), arg.end());
			}
			else if constexpr (std::is_same_v<T, bb_bset>)
			{
				arg.reset(value);
			} },
				   data);
	}
	bool contains(uint16_t value) const
	{
		return std::visit([value](auto &&arg) -> bool
						  {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, bb_array>)
        {
            return std::binary_search(arg.begin(), arg.end(), value);
        }
        else if constexpr (std::is_same_v<T, bb_bset>)
        {
            return arg.test(value);
        }
        else
        {
			//should *not* reach here
            return false; // or handle other types appropriately
        } },
						  data);
		;
	}
	void intersect(const BBData &other)
	{
		data = std::visit([](auto const &arg1, auto const &arg2) -> bb_variant
						  {
		using T1 = std::decay_t<decltype(arg1)>;
		using T2 = std::decay_t<decltype(arg2)>;

		if constexpr (std::is_same_v<T1, bb_array> && std::is_same_v<T2, bb_array>)
		{
			bb_array result;
			std::set_intersection(arg1.begin(), arg1.end(), arg2.begin(), arg2.end(), std::back_inserter(result));
			return result; // skip redundant check
		}
		else if constexpr (std::is_same_v<T1, bb_array> && std::is_same_v<T2, bb_bset>)
		{
			bb_array result;
			for (auto const &i : arg1)
			{
				if (arg2.test(i))
				{
					result.push_back(i);
				}
			}
			return result; // skip redundant check
		}
		else if constexpr (std::is_same_v<T1, bb_bset> && std::is_same_v<T2, bb_array>)
		{
			// handle intersection for bb_bset vs bb_array
			bb_array result;
			result.reserve(BB_ARRAY_THRESHOLD);
			for (auto const &i : arg2)
			{
				if (arg1.test(i))
				{
					result.push_back(i);
				}
			}
			return result; // skip redundant check
		}
		else if constexpr (std::is_same_v<T1, bb_bset> && std::is_same_v<T2, bb_bset>)
		{
			// handle intersection for bb_bset vs bb_bset
			// optimistically assume that overall size will be more than BB_ARRAY_THRESHOLD
			bb_bset result;
			result = arg1 & arg2;
			return convertForCardinality(result);
		} },
						  data, other.data);
	}

	void unite(BBData const &other)
	{
		data = std::visit([](auto const &arg1, auto const &arg2) -> bb_variant
						  {
		using T1 = std::decay_t<decltype(arg1)>;
		using T2 = std::decay_t<decltype(arg2)>;

		if constexpr (std::is_same_v<T1, bb_array> && std::is_same_v<T2, bb_array>)
		{
			//optimistically assume that overall size will be more than BB_ARRAY_THRESHOLD, therefore bitset
			bb_bset result;
			for(auto const &i : arg1) result.set(i);
			for(auto const &i : arg2) result.set(i);
			return convertForCardinality(result);
		}
		else if constexpr (std::is_same_v<T1, bb_array> && std::is_same_v<T2, bb_bset>)
		{
			//lower-bounded by bset, which is guaranteed to be above threshold
			bb_bset result = arg2;
			for(auto const &i : arg1) result.set(i);
			return result; // skip redundant check
		}
		else if constexpr (std::is_same_v<T1, bb_bset> && std::is_same_v<T2, bb_array>)
		{
			bb_bset result = arg1;
			for(auto const &i : arg2) result.set(i);
			return result; //skip redundant check
		}
		else if constexpr (std::is_same_v<T1, bb_bset> && std::is_same_v<T2, bb_bset>)
		{
			bb_bset result = arg1 | arg2;
			return result; //skip redundant check
		} },
						  data, other.data);
	}
};

class BarkingBitmap
{
public:
	BarkingBitmap() = default;
	~BarkingBitmap() = default;

	void add(uint32_t value)
	{
		bb_data[value >> 16].add(value & 0xFFFF);
	}
	void remove(uint32_t value)
	{
		bb_data[value >> 16].remove(value & 0xFFFF);
	}
	bool contains(uint32_t value) const
	{
		return bb_data[value >> 16].contains(value & 0xFFFF);
	}
	void clear()
	{
		for (auto &i : bb_data)
		{
			i.clear();
		}
	}
	void intersect(BarkingBitmap const &other)
	{
		for (size_t i = 0; i < BB_BUCKET_SZ; i++)
		{
			bb_data[i].intersect(other.bb_data[i]);
		}
	}
	void unite(BarkingBitmap const &other)
	{
		for (size_t i = 0; i < BB_BUCKET_SZ; i++)
		{
			bb_data[i].unite(other.bb_data[i]);
		}
	}

private:
	std::array<BBData, BB_BUCKET_SZ> bb_data;
};

#endif // BARKING_BITMAP_HPP

// use union (?) to know what kind it is
// make it modern cpp as possible
// declare move, move-copy constructor, copy constructor, destructor, assignment operator

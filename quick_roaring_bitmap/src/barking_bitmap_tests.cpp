#include "barking_bitmap.hpp"
#include <random>
#include <gtest/gtest.h>

class BarkingBitmapTests : public testing::Test
{
public:
	BarkingBitmap bm;
	std::mt19937_64 rng;

protected:
	void SetUp() override
	{
		rng.seed(std::random_device{}());
	}
	void TearDown() override
	{
	}
};

// basic test
TEST_F(BarkingBitmapTests, TestInitialization)
{
	std::vector<uint32_t> v;
	std::uniform_int_distribution<uint32_t> dist;

	for (int i = 0; i < 1000; ++i)
	{
		uint32_t num = dist(rng);
		v.push_back(num);
		bm.add(num);
	}

	for (uint32_t num : v)
	{
		EXPECT_TRUE(bm.contains(num)) << "BarkingBitmap should contain the number: " << num;
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
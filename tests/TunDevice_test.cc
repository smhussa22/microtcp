// related headers

// c sys headers

// cpp stdlib headers
#include <stdexcept>

// 3rd party headers
#include <gtest/gtest.h>

// project headers
#include "tun/TunDevice.hh"

using namespace microtcp::tun;

// TunDevice constructor throws if the device doesn't exist or can't be opened
TEST(TunDeviceTest, ConstructorThrowsOnInvalidDevice)
{

    EXPECT_THROW(TunDevice { "this_device_does_not_exist" }, std::runtime_error);

}

// TunDevice is not copyable
TEST(TunDeviceTest, NotCopyable)
{

    EXPECT_FALSE(std::is_copy_constructible_v<TunDevice>);
    EXPECT_FALSE(std::is_copy_assignable_v<TunDevice>);

}

// TunDevice is not movable
TEST(TunDeviceTest, NotMovable)
{

    EXPECT_FALSE(std::is_move_constructible_v<TunDevice>);
    EXPECT_FALSE(std::is_move_assignable_v<TunDevice>);

}

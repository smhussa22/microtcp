// related headers

// c sys headers

// cpp stdlib headers
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// 3rd party headers
#include <gtest/gtest.h>

// project headers
#include "net/IPv4.hh"

using namespace microtcp::net;

// checksum of a known valid ipv4 header should match the expected value
TEST(IPv4Test, ChecksumKnownVector)
{

    // TODO: use a captured real ipv4 header (e.g. from wireshark), compute checksum, verify it matches the header's stored checksum
    EXPECT_TRUE(true);

}

// parsing a buffer shorter than 20 bytes should throw
TEST(IPv4Test, ParseThrowsOnTruncatedBuffer)
{

    std::array<std::uint8_t, 10uz> short_buf { };
    EXPECT_THROW(parse_ipv4(short_buf), std::runtime_error);

}

// parsing a buffer with bad version (not 4) should throw
TEST(IPv4Test, ParseThrowsOnBadVersion)
{

    // TODO: create a 20-byte buffer with version field set to 3 or 6, verify parse throws
    EXPECT_TRUE(true);

}

// parsing a buffer with bad ihl (< 5) should throw
TEST(IPv4Test, ParseThrowsOnBadIHL)
{

    // TODO: create a 20-byte buffer with ihl field set to 4, verify parse throws
    EXPECT_TRUE(true);

}

// parsing a valid ipv4 header should extract all fields correctly
TEST(IPv4Test, ParseValidHeader)
{

    // TODO: use a known-good ipv4 header (captured from wireshark or constructed), parse it, verify all fields match expected values
    EXPECT_TRUE(true);

}

// building a packet and parsing it should round-trip
TEST(IPv4Test, BuildAndParseRoundTrip)
{

    // TODO: build a packet with known src/dst/protocol/payload, parse the result, verify header fields match input
    EXPECT_TRUE(true);

}

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
#include "net/Icmp.hh"

using namespace microtcp::net;

// hand-computed valid echo request:
//   type=8, code=0, identifier=0x1234, sequence=1, payload "ab"
//   checksum: sum = 0x0800 + 0x1234 + 0x0001 + 0x6162 = 0x7B97, ~0x7B97 = 0x8468
static constexpr std::array<std::uint8_t, 10uz> valid_request {
    0x08, 0x00, // type = 8 (echo request), code = 0
    0x84, 0x68, // checksum
    0x12, 0x34, // identifier = 0x1234
    0x00, 0x01, // sequence = 1
    0x61, 0x62  // payload = "ab"
};

// checksum computed over a valid message (including its checksum field) equals 0x0000
TEST(IcmpTest, ChecksumOverValidMessageIsZero)
{

    std::uint16_t result { icmp_checksum(valid_request) };
    EXPECT_EQ(result, 0u);

}

// checksum computed with checksum field zeroed matches the hand-computed value
TEST(IcmpTest, ChecksumMatchesKnownVector)
{

    std::array<std::uint8_t, 10uz> message { valid_request };
    message[2] = 0x00;
    message[3] = 0x00;

    std::uint16_t result { icmp_checksum(message) };
    EXPECT_EQ(result, 0x8468u);

}

// odd-length message: trailing byte 0xAB pads to word 0xAB00
//   sum = 0x0800 + 0x0001 + 0x0001 + 0xAB00 = 0xB302, ~0xB302 = 0x4CFD
TEST(IcmpTest, ChecksumHandlesOddLength)
{

    std::array<std::uint8_t, 9uz> message {
        0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0xAB
    };

    std::uint16_t result { icmp_checksum(message) };
    EXPECT_EQ(result, 0x4CFDu);

}

// parsing a buffer shorter than 8 bytes throws
TEST(IcmpTest, ParseThrowsOnTruncatedBuffer)
{

    std::array<std::uint8_t, 4uz> short_buf { };
    EXPECT_THROW(parse_icmp(short_buf), std::runtime_error);

}

// parsing a message with a corrupted checksum throws
TEST(IcmpTest, ParseThrowsOnBadChecksum)
{

    std::array<std::uint8_t, 10uz> message { valid_request };
    message[2] = 0xDE;
    message[3] = 0xAD;
    EXPECT_THROW(parse_icmp(message), std::runtime_error);

}

// parsing the valid request extracts every field correctly
TEST(IcmpTest, ParseValidRequestExtractsFields)
{

    IcmpHeader hdr { parse_icmp(valid_request) };

    EXPECT_EQ(hdr.type, 8u);
    EXPECT_EQ(hdr.code, 0u);
    EXPECT_EQ(hdr.checksum, 0x8468u);
    EXPECT_EQ(hdr.identifier, 0x1234u);
    EXPECT_EQ(hdr.sequence, 1u);

}

// echo reply flips type to 0, keeps identifier/sequence/payload, and re-checksums validly
TEST(IcmpTest, EchoReplyFlipsTypeAndRechecksums)
{

    std::vector<std::uint8_t> reply { build_icmp_echo_reply(valid_request) };

    ASSERT_EQ(reply.size(), valid_request.size());
    EXPECT_EQ(reply[0], 0u);
    EXPECT_EQ(reply[1], 0u);
    EXPECT_EQ(reply[8], 0x61u);
    EXPECT_EQ(reply[9], 0x62u);

    IcmpHeader hdr { parse_icmp(reply) };
    EXPECT_EQ(hdr.type, 0u);
    EXPECT_EQ(hdr.identifier, 0x1234u);
    EXPECT_EQ(hdr.sequence, 1u);

}

// building a reply from a non-request (type != 8) throws
TEST(IcmpTest, EchoReplyThrowsOnNonRequest)
{

    std::array<std::uint8_t, 10uz> message { valid_request };
    message[0] = 0x00;
    EXPECT_THROW(build_icmp_echo_reply(message), std::runtime_error);

}

// building a reply from a truncated buffer throws
TEST(IcmpTest, EchoReplyThrowsOnTruncated)
{

    std::array<std::uint8_t, 4uz> short_buf { };
    EXPECT_THROW(build_icmp_echo_reply(short_buf), std::runtime_error);

}

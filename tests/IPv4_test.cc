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

// hand-computed valid header:
//   src 192.168.0.1 -> dst 192.168.0.2, protocol TCP (6), ttl 64, DF set, no payload
//   checksum: 0xB990 (verified by hand)
static constexpr std::array<std::uint8_t, 20uz> valid_header {
    0x45, 0x00,             // version=4, ihl=5, dscp/ecn=0
    0x00, 0x14,             // total_length = 20
    0x00, 0x00,             // identification = 0
    0x40, 0x00,             // flags = DF, fragment offset = 0
    0x40, 0x06,             // ttl = 64, protocol = TCP (6)
    0xB9, 0x90,             // header checksum
    0xC0, 0xA8, 0x00, 0x01, // src ip = 192.168.0.1
    0xC0, 0xA8, 0x00, 0x02  // dst ip = 192.168.0.2
};

// checksum computed over a valid header (including its checksum field) equals 0x0000
TEST(IPv4Test, ChecksumOverValidHeaderIsZero)
{

    std::uint16_t result { ipv4_header_checksum(valid_header) };
    EXPECT_EQ(result, 0u);

}

// checksum computed with checksum field zeroed matches the on-wire value
TEST(IPv4Test, ChecksumMatchesKnownVector)
{

    std::array<std::uint8_t, 20uz> header { valid_header };
    header[10] = 0x00;
    header[11] = 0x00;

    std::uint16_t result { ipv4_header_checksum(header) };
    EXPECT_EQ(result, 0xB990u);

}

// parsing a buffer shorter than 20 bytes throws
TEST(IPv4Test, ParseThrowsOnTruncatedBuffer)
{

    std::array<std::uint8_t, 10uz> short_buf { };
    EXPECT_THROW(parse_ipv4(short_buf), std::runtime_error);

}

// parsing a buffer with version != 4 throws
TEST(IPv4Test, ParseThrowsOnBadVersion)
{

    std::array<std::uint8_t, 20uz> header { valid_header };
    header[0] = 0x65;  // version = 6, ihl = 5
    EXPECT_THROW(parse_ipv4(header), std::runtime_error);

}

// parsing a buffer with ihl < 5 throws
TEST(IPv4Test, ParseThrowsOnBadIHL)
{

    std::array<std::uint8_t, 20uz> header { valid_header };
    header[0] = 0x44;  // version = 4, ihl = 4
    EXPECT_THROW(parse_ipv4(header), std::runtime_error);

}

// parsing the valid header extracts every field correctly
TEST(IPv4Test, ParseValidHeaderExtractsFields)
{

    IPv4Header hdr { parse_ipv4(valid_header) };

    EXPECT_EQ(hdr.version_ihl, 0x45u);
    EXPECT_EQ(hdr.dscp_ecn, 0x00u);
    EXPECT_EQ(hdr.total_length, 20u);
    EXPECT_EQ(hdr.identification, 0u);
    EXPECT_EQ(hdr.flags_frag_offset, 0x4000u);
    EXPECT_EQ(hdr.ttl, 64u);
    EXPECT_EQ(hdr.protocol, 6u);
    EXPECT_EQ(hdr.header_checksum, 0xB990u);
    EXPECT_EQ(hdr.src_ip, 0xC0A80001u);
    EXPECT_EQ(hdr.dst_ip, 0xC0A80002u);

}

// parsing a header with a corrupted checksum throws
TEST(IPv4Test, ParseThrowsOnBadChecksum)
{

    std::array<std::uint8_t, 20uz> header { valid_header };
    header[10] = 0xDE;
    header[11] = 0xAD;
    EXPECT_THROW(parse_ipv4(header), std::runtime_error);

}

// build produces a packet parse can round-trip cleanly
TEST(IPv4Test, BuildAndParseRoundTrip)
{

    std::array<std::uint8_t, 4uz> payload { 0xDE, 0xAD, 0xBE, 0xEF };
    std::vector<std::uint8_t> packet { build_ipv4(0xC0A80101u, 0x0A00000Au, 6u, payload) };

    IPv4Header hdr { parse_ipv4(packet) };

    EXPECT_EQ(hdr.src_ip, 0xC0A80101u);
    EXPECT_EQ(hdr.dst_ip, 0x0A00000Au);
    EXPECT_EQ(hdr.protocol, 6u);
    EXPECT_EQ(hdr.total_length, 24u);  // 20 header + 4 payload

}

// build packet with empty payload still parses correctly
TEST(IPv4Test, BuildEmptyPayloadRoundTrip)
{

    std::vector<std::uint8_t> packet { build_ipv4(0xC0A80001u, 0xC0A80002u, 1u, { }) };

    IPv4Header hdr { parse_ipv4(packet) };

    EXPECT_EQ(hdr.total_length, 20u);
    EXPECT_EQ(hdr.protocol, 1u);  // ICMP

}

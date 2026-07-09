#ifndef MICROTCP_NET_IPV4_HH
#define MICROTCP_NET_IPV4_HH

// related headers

// c sys headers

// cpp stdlib headers
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

// 3rd party headers

// project headers

namespace microtcp::net
{

    // packed ipv4 header matching the wire format exactly
    struct alignas(1) IPv4Header
    {

        std::uint8_t version_ihl { };           // 4-bit version (4) + 4-bit IHL (header length in 32-bit words)
        std::uint8_t dscp_ecn { };              // 6-bit DSCP + 2-bit ECN
        std::uint16_t total_length { };         // entire packet length in bytes (big-endian)
        std::uint16_t identification { };       // packet id for reassembly (big-endian)
        std::uint16_t flags_frag_offset { };    // 3-bit flags + 13-bit fragment offset (big-endian)
        std::uint8_t ttl { };                   // time to live
        std::uint8_t protocol { };              // upper layer protocol (6=TCP, 1=ICMP, 17=UDP)
        std::uint16_t header_checksum { };      // ones' complement checksum of header (big-endian)
        std::uint32_t src_ip { };               // source ipv4 address (big-endian)
        std::uint32_t dst_ip { };               // destination ipv4 address (big-endian)

    };

    static_assert(sizeof(IPv4Header) == 20uz);

    // parse a buffer as an ipv4 packet; validate version, ihl, checksum, length
    IPv4Header parse_ipv4(std::span<const std::uint8_t> buffer);

    // compute the ipv4 header checksum (ones' complement of ones' complement sum)
    std::uint16_t ipv4_header_checksum(std::span<const std::uint8_t> header);

    // construct an ipv4 packet with correct header, checksum, and payload
    std::vector<std::uint8_t> build_ipv4(
        std::uint32_t src_ip,
        std::uint32_t dst_ip,
        std::uint8_t protocol,
        std::span<const std::uint8_t> payload
    );

}

#endif // MICROTCP_NET_IPV4_HH

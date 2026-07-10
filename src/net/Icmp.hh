#ifndef MICROTCP_NET_ICMP_HH
#define MICROTCP_NET_ICMP_HH

// related headers

// c sys headers
#include <cstddef>
#include <cstdint>

// cpp stdlib headers
#include <span>
#include <vector>
#include <stdexcept>
#include <string>

// 3rd party headers

// project headers

namespace microtcp::net
{

    // packed icmp echo header matching the wire format exactly
    struct alignas(1) IcmpHeader
    {

        IcmpHeader() = default;
        ~IcmpHeader() = default;

        IcmpHeader(const IcmpHeader&) = default;
        IcmpHeader& operator=(const IcmpHeader&) = default;

        IcmpHeader(IcmpHeader&&) = default;
        IcmpHeader& operator=(IcmpHeader&&) = default;

        std::uint8_t type { };          // message type (8 = echo request, 0 = echo reply)
        std::uint8_t code { };          // subtype; always 0 for echo request/reply
        std::uint16_t checksum { };     // ones' complement checksum over entire icmp message (big-endian)
        std::uint16_t identifier { };   // set by sender to match requests with replies (big-endian)
        std::uint16_t sequence { };     // incremented per request to detect loss/reordering (big-endian)

    };

    static_assert(sizeof(IcmpHeader) == 8uz);

    // parse a buffer as an icmp message; validate length and checksum
    IcmpHeader parse_icmp(std::span<const std::uint8_t> buffer);

    // compute the icmp checksum over the entire message (header + payload); handles odd lengths
    std::uint16_t icmp_checksum(std::span<const std::uint8_t> message);

    // given a full echo request message (header + payload), build the matching echo reply
    std::vector<std::uint8_t> build_icmp_echo_reply(std::span<const std::uint8_t> request);

}

#endif // MICROTCP_NET_ICMP_HH

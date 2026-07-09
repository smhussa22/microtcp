// related headers
#include "IPv4.hh"

// c sys headers

// cpp stdlib headers
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

// 3rd party headers

// project headers

namespace microtcp::net
{

    IPv4Header parse_ipv4(std::span<const std::uint8_t> buffer)
    {

        // TODO: validate buffer.size() >= 20; extract version/ihl from first byte; validate version==4, ihl>=5; validate total_length matches buffer or is within; validate checksum; parse all fields; return populated header

        (void)buffer;
        IPv4Header hdr { };
        return hdr;

    }

    std::uint16_t ipv4_header_checksum(std::span<const std::uint8_t> header)
    {

        // TODO: compute ones' complement checksum: sum all 16-bit words (big-endian), fold carries, take ones' complement
        (void)header;
        return 0uz;

    }

    std::vector<std::uint8_t> build_ipv4(
        std::uint32_t src_ip,
        std::uint32_t dst_ip,
        std::uint8_t protocol,
        std::span<const std::uint8_t> payload
    )
    {

        // TODO: fill ipv4header with values, set version=4/ihl=5, total_length=20+payload.size(), compute checksum, copy header then payload into result vector
        (void)src_ip;
        (void)dst_ip;
        (void)protocol;
        (void)payload;
        return { };

    }

}

// related headers

// c sys headers
#include <cstddef>
#include <cstdint>

// cpp stdlib headers
#include <array>
#include <exception>
#include <print>
#include <span>
#include <string_view>

// 3rd party headers

// project headers
#include "net/IPv4.hh"
#include "tun/TunDevice.hh"

void print_ip(std::uint32_t ip);
std::string_view protocol_name(std::uint8_t protocol);

int main()
{

    // attach to tun0; for each incoming packet, parse the ipv4 header and print a summary
    microtcp::tun::TunDevice tun { "tun0" };
    std::array<std::uint8_t, 1500uz> buffer { };

    for (;;)
    {

        std::size_t n { tun.read(buffer) };
        if (n == 0uz) break;

        try
        {

            microtcp::net::IPv4Header hdr { microtcp::net::parse_ipv4(std::span<const std::uint8_t> { buffer.data(), n }) };

            std::print("[ipv4] ");
            print_ip(hdr.src_ip);
            std::print(" -> ");
            print_ip(hdr.dst_ip);
            std::println(" proto={} ttl={} len={}", protocol_name(hdr.protocol), hdr.ttl, hdr.total_length);

        }
        catch (const std::exception& e)
        {

            std::println("[parse error] {}", e.what());

        }

    }

    return 0;

}

void print_ip(std::uint32_t ip)
{

    std::print("{}.{}.{}.{}",
        (ip >> 24) & 0xFFu,
        (ip >> 16) & 0xFFu,
        (ip >> 8) & 0xFFu,
        ip & 0xFFu
    );

}

std::string_view protocol_name(std::uint8_t protocol)
{

    switch (protocol)
    {

        case 1:  return "ICMP";
        case 6:  return "TCP";
        case 17: return "UDP";
        default: return "?";

    }

}

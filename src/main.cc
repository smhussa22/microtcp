// related headers

// c sys headers

// cpp stdlib headers
#include <array>
#include <cstddef>
#include <cstdint>
#include <print>
#include <span>

// 3rd party headers

// project headers
#include "tun/TunDevice.hh"

void hex_dump(std::span<const std::uint8_t> packet);

int main()
{

    // attach to tun0 and read ip packets in a loop, hex-dumping each one
    microtcp::tun::TunDevice tun { "tun0" };
    std::array<std::uint8_t, 1500uz> buffer {};

    for (;;)
    {

        std::size_t n { tun.read(buffer) };
        if (n == 0uz) break;
        hex_dump(std::span<const std::uint8_t> { buffer.data(), n });

    }

    return 0;

}

void hex_dump(std::span<const std::uint8_t> packet)
{

    for (std::size_t i { 0uz }; i < packet.size(); ++i)
    {

        std::print("{:02x} ", packet[i]);
        if ((i + 1uz) % 16uz == 0uz) std::println("");

    }
    if (packet.size() % 16uz != 0uz) std::println("");

}

// related headers
#include "Icmp.hh"

// c sys headers

// cpp stdlib headers

// 3rd party headers

// project headers

namespace microtcp::net
{

    IcmpHeader parse_icmp(std::span<const std::uint8_t> buffer)
    {

        if (buffer.size() < 8uz) throw std::runtime_error("expected at least 8 byte icmp message; got " + std::to_string(buffer.size()));

        // checksum over the whole message (including the stored checksum) must be 0
        std::uint16_t computed { icmp_checksum(buffer) };
        if (computed != 0u) throw std::runtime_error("bad icmp checksum");

        IcmpHeader header { };
        header.type = buffer[0];
        header.code = buffer[1];
        header.checksum = static_cast<std::uint16_t>((buffer[2] << 8) | buffer[3]);
        header.identifier = static_cast<std::uint16_t>((buffer[4] << 8) | buffer[5]);
        header.sequence = static_cast<std::uint16_t>((buffer[6] << 8) | buffer[7]);

        return header;

    }

    std::uint16_t icmp_checksum(std::span<const std::uint8_t> message)
    {

        // sum all 16-bit big-endian words
        std::uint32_t checksum { 0u };
        std::size_t i { 0uz };
        for (; i + 1uz < message.size(); i += 2uz)
        {

            std::uint16_t word { static_cast<std::uint16_t>(message[i] << 8 | message[i + 1uz]) };
            checksum += word;

        }

        // odd-length message: trailing byte is the high byte of a final word padded with 0x00
        if (i < message.size()) checksum += static_cast<std::uint16_t>(message[i] << 8);

        // fold carries back into the low 16 bits
        while ((checksum >> 16) != 0u) checksum = (checksum & 0xFFFFu) + (checksum >> 16);

        return static_cast<std::uint16_t>(~checksum);

    }

    std::vector<std::uint8_t> build_icmp_echo_reply(std::span<const std::uint8_t> request)
    {

        if (request.size() < 8uz) throw std::runtime_error("echo request too short; got " + std::to_string(request.size()));
        if (request[0] != 8u) throw std::runtime_error("expected icmp echo request (type 8); got " + std::to_string(request[0]));

        // copy the request, flip type to echo reply, zero the checksum field
        std::vector<std::uint8_t> reply { request.begin(), request.end() };
        reply[0] = 0u;
        reply[2] = 0u;
        reply[3] = 0u;

        // recompute checksum over the whole message and patch it in (big-endian)
        std::uint16_t checksum { icmp_checksum(reply) };
        reply[2] = static_cast<std::uint8_t>((checksum >> 8) & 0xFFu);
        reply[3] = static_cast<std::uint8_t>(checksum & 0xFFu);

        return reply;

    }

}

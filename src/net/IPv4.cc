// related headers
#include "IPv4.hh"

// c sys headers

// cpp stdlib headers

// 3rd party headers

// project headers

namespace microtcp::net
{

    IPv4Header parse_ipv4(std::span<const std::uint8_t> buffer)
    {

        // 0101 0100
        // 0000 1111
        if (buffer.size() < 20uz) throw std::runtime_error("expected 20 byte ipv4 header; got " + std::to_string(buffer.size()));

        std::uint8_t first_byte { buffer[0] };

        // discard right 4 bits for version
        std::uint8_t version { static_cast<std::uint8_t>(first_byte >> 4) }; 
        if (version != 4) throw std::runtime_error("expected ip version 4 header; got " + std::to_string(version));
        
        // mask first 4 bits and preserve next 4 bits for ihl
        std::uint8_t ihl { static_cast<std::uint8_t>(first_byte & 0x0F) };
        if (ihl < 5) throw std::runtime_error("invalid ip version 4 header length; got " + std::to_string(ihl));

        std::size_t header_size { static_cast<std::size_t>(ihl * 4uz) }; 
        if (buffer.size() < header_size) throw std::runtime_error("buffer smaller than the declared ihl; " + std::to_string(buffer.size()));
        
        // extract 16 bit fields in network byte order; access from big endian
        std::uint16_t total_length { static_cast<std::uint16_t>((buffer[2] << 8) | buffer[3]) };
        if (total_length < header_size) throw std::runtime_error("total length < header size");
        if (total_length > buffer.size()) throw std::runtime_error("total length > buffer size");
        std::uint16_t computed { ipv4_header_checksum(buffer.subspan(0uz, header_size)) };
        if (computed != 0u) throw std::runtime_error("bad ipv4 header checksum");
        
        std::uint16_t identification { static_cast<std::uint16_t>((buffer[4] << 8) | buffer[5]) };
        std::uint16_t flags_frag_offset { static_cast<std::uint16_t>((buffer[6] << 8) | buffer[7]) };
        std::uint16_t header_checksum { static_cast<std::uint16_t>((buffer[10] << 8) | buffer[11]) }; 
        
        // 192.168.1.0
        // buf[15] -> buf[12]
        // 00 01 a8 c0
        std::uint32_t src_ip { 
            static_cast<std::uint32_t>(buffer[12]) << 24 | 
            static_cast<std::uint32_t>(buffer[13]) << 16 |
            static_cast<std::uint32_t>(buffer[14]) << 8  |
            static_cast<std::uint32_t>(buffer[15])
        }; 
        std::uint32_t dst_ip { 
            static_cast<std::uint32_t>(buffer[16]) << 24 | 
            static_cast<std::uint32_t>(buffer[17]) << 16 |
            static_cast<std::uint32_t>(buffer[18]) << 8  |
            static_cast<std::uint32_t>(buffer[19])
        }; 

        IPv4Header header { };
        header.version_ihl = first_byte;
        header.dscp_ecn = buffer[1];
        header.total_length = total_length;
        header.identification = identification;
        header.flags_frag_offset = flags_frag_offset;
        header.header_checksum = header_checksum;
        header.ttl = buffer[8];
        header.protocol = buffer[9];
        header.src_ip = src_ip;
        header.dst_ip = dst_ip;
        
        return header;

    }

    std::uint16_t ipv4_header_checksum(std::span<const std::uint8_t> header)
    {

        // TODO: compute ones' complement checksum: sum all 16-bit words (big-endian), fold carries, take ones' complement
        std::uint32_t checksum { 0u };
        for (auto i { 0uz }; i + 1uz < header.size(); i += 2uz)
        {

            std::uint16_t word { static_cast<std::uint16_t>(header[i] << 8 | header[i + 1uz]) };
            checksum += word;

        }

        // fold carries: any bits above bit 15 get added back into the low 16 bits
        // may need to fold twice if the fold itself carries
        // i dont understand this
        while ((checksum >> 16) != 0u) checksum = (checksum & 0xFFFFu) + (checksum >> 16);

        return static_cast<std::uint16_t>(~checksum);

    }

    std::vector<std::uint8_t> build_ipv4(
        std::uint32_t src_ip,
        std::uint32_t dst_ip,
        std::uint8_t protocol,
        std::span<const std::uint8_t> payload
    )
    {

        // TODO: fill ipv4header with values, set version=4/ihl=5, compute checksum, copy header then payload into result vector
        std::uint16_t total_length { static_cast<std::uint16_t>(20uz+payload.size()) };
        std::vector<std::uint8_t> packet (total_length, 0u);
        // byte 0: version (4) in upper nibble + ihl (5) in lower nibble = 0x45
        packet[0] = 0x45;

        // byte 1: dscp + ecn — leave as 0 for best-effort traffic
        packet[1] = 0x00;

        // bytes 2-3: total_length (big-endian)
        packet[2] = static_cast<std::uint8_t>((total_length >> 8) & 0xFFu);
        packet[3] = static_cast<std::uint8_t>(total_length & 0xFFu);

        // bytes 4-5: identification (0 — only matters if we fragment, which we don't)
        packet[4] = 0x00;
        packet[5] = 0x00;

        // bytes 6-7: flags + fragment offset. set DF (don't fragment) bit only
        packet[6] = 0x40;  // 0100 0000 → reserved=0, DF=1, MF=0, offset upper=0
        packet[7] = 0x00;

        // byte 8: ttl (64 is the typical modern default)
        packet[8] = 64;

        // byte 9: upper-layer protocol (1=ICMP, 6=TCP, 17=UDP)
        packet[9] = protocol;

        // bytes 10-11: checksum — leave as 0 for now; we compute over these bytes below
        packet[10] = 0x00;
        packet[11] = 0x00;

        // bytes 12-15: src_ip (big-endian)
        packet[12] = static_cast<std::uint8_t>((src_ip >> 24) & 0xFFu);
        packet[13] = static_cast<std::uint8_t>((src_ip >> 16) & 0xFFu);
        packet[14] = static_cast<std::uint8_t>((src_ip >> 8) & 0xFFu);
        packet[15] = static_cast<std::uint8_t>(src_ip & 0xFFu);

        // bytes 16-19: dst_ip (big-endian)
        packet[16] = static_cast<std::uint8_t>((dst_ip >> 24) & 0xFFu);
        packet[17] = static_cast<std::uint8_t>((dst_ip >> 16) & 0xFFu);
        packet[18] = static_cast<std::uint8_t>((dst_ip >> 8) & 0xFFu);
        packet[19] = static_cast<std::uint8_t>(dst_ip & 0xFFu);

        // now compute checksum over the 20 header bytes (with checksum field currently 0)
        std::uint16_t checksum { ipv4_header_checksum(std::span<const std::uint8_t> { packet.data(), 20uz }) };

        // patch the computed checksum into bytes 10-11 (big-endian)
        packet[10] = static_cast<std::uint8_t>((checksum >> 8) & 0xFFu);
        packet[11] = static_cast<std::uint8_t>(checksum & 0xFFu);

        // append payload after the header
        std::copy(payload.begin(), payload.end(), packet.begin() + 20);

        return packet;

    }

}

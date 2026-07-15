// related headers
#include "Tcp.hh"

// c sys headers

// cpp stdlib headers

// 3rd party headers

// project headers

namespace microtcp::net
{

    TcpHeader parse_tcp(std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment)
    {

        // TODO:
        //       validate checksum via tcp_checksum == 0;
        //       extract all fields big-endian (src_port, dst_port, seq, ack, flags, window, etc.)
        if (segment.size() < 20uz) throw std::runtime_error("expected 20 byte segment, got: " + std::to_string(segment.size()));
        
        std::uint8_t data_offset { static_cast<std::uint8_t>(segment[12] >> 4) };
        if (data_offset < 5) throw std::runtime_error("expected 5 bit data offset; got " + std::to_string(data_offset));

        std::size_t header_size { static_cast<std::size_t>(data_offset * 4uz)};
        if (segment.size() < header_size) throw std::runtime_error("segment smaller than declared data offset; " + std::to_string(segment.size()));
        
        std::uint16_t computed_checksum { tcp_checksum(src_ip, dst_ip, segment) };
        if (computed_checksum != 0u) throw std::runtime_error("checksum not valid");
        
        // extract 16-bit fields big-endian
        std::uint16_t src_port { static_cast<std::uint16_t>((segment[0] << 8) | segment[1]) };
        std::uint16_t dst_port { static_cast<std::uint16_t>((segment[2] << 8) | segment[3]) };
        std::uint16_t window_size { static_cast<std::uint16_t>((segment[14] << 8) | segment[15]) };
        std::uint16_t checksum { static_cast<std::uint16_t>((segment[16] << 8) | segment[17]) };
        std::uint16_t urgent_ptr { static_cast<std::uint16_t>((segment[18] << 8) | segment[19]) };

        // extract 32-bit fields big-endian; cast BEFORE shifting to avoid promoted-int UB
        std::uint32_t seq_num {
            static_cast<std::uint32_t>(segment[4]) << 24 |
            static_cast<std::uint32_t>(segment[5]) << 16 |
            static_cast<std::uint32_t>(segment[6]) << 8  |
            static_cast<std::uint32_t>(segment[7])
        };
        std::uint32_t ack_num {
            static_cast<std::uint32_t>(segment[8]) << 24 |
            static_cast<std::uint32_t>(segment[9]) << 16 |
            static_cast<std::uint32_t>(segment[10]) << 8 |
            static_cast<std::uint32_t>(segment[11])
        };

        TcpHeader header { };
        header.src_port = src_port;
        header.dst_port = dst_port;
        header.seq_num = seq_num;
        header.ack_num = ack_num;
        header.data_offset = segment[12];
        header.flags = segment[13];
        header.window_size = window_size;
        header.checksum = checksum;
        header.urgent_ptr = urgent_ptr;

        return header;

    }

    std::uint16_t tcp_checksum(std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment)
    {

        // TODO: build the 12-byte ipv4 pseudo-header:
        //   bytes 0-3:   src_ip (big-endian)
        //   bytes 4-7:   dst_ip (big-endian)
        //   byte  8:     0x00 (reserved)
        //   byte  9:     0x06 (protocol = TCP)
        //   bytes 10-11: segment length in bytes (big-endian)
        // then run the same rfc 1071 ones' complement sum over pseudo-header + segment
        // (handle odd-length the same way as icmp_checksum)
        
        std::array<std::uint8_t, 12uz> pseudo_header { };
        pseudo_header[0] = static_cast<std::uint8_t>((src_ip >> 24) & 0xFFu);
        pseudo_header[1] = static_cast<std::uint8_t>((src_ip >> 16) & 0xFFu);
        pseudo_header[2] = static_cast<std::uint8_t>((src_ip >> 8) & 0xFFu);
        pseudo_header[3] = static_cast<std::uint8_t>(src_ip & 0xFFu);
        pseudo_header[4] = static_cast<std::uint8_t>((dst_ip >> 24) & 0xFFu);
        pseudo_header[5] = static_cast<std::uint8_t>((dst_ip >> 16) & 0xFFu);
        pseudo_header[6] = static_cast<std::uint8_t>((dst_ip >> 8) & 0xFFu);
        pseudo_header[7] = static_cast<std::uint8_t>(dst_ip & 0xFFu);
        pseudo_header[8] = 0x00;
        pseudo_header[9] = 0x06;

        std::uint16_t seg_len { static_cast<std::uint16_t>(segment.size()) };
        pseudo_header[10] = static_cast<std::uint8_t>((seg_len >> 8) & 0xFFu);
        pseudo_header[11] = static_cast<std::uint8_t>(seg_len & 0xFFu);

        // sum pseudo-header words (fixed 12 bytes, always even)
        std::uint32_t checksum { 0u };
        for (std::size_t i { 0uz }; i < pseudo_header.size(); i += 2uz)
        {

            checksum += static_cast<std::uint16_t>(pseudo_header[i] << 8 | pseudo_header[i + 1uz]);

        }

        // sum segment words; trailing odd byte is the high byte of a final word padded with 0x00
        std::size_t i { 0uz };
        for (; i + 1uz < segment.size(); i += 2uz)
        {

            checksum += static_cast<std::uint16_t>(segment[i] << 8 | segment[i + 1uz]);

        }
        if (i < segment.size()) checksum += static_cast<std::uint16_t>(segment[i] << 8);

        // fold carries back into the low 16 bits, then ones' complement
        while ((checksum >> 16) != 0u) checksum = (checksum & 0xFFFFu) + (checksum >> 16);

        return static_cast<std::uint16_t>(~checksum);

    }

    std::vector<std::uint8_t> build_tcp(
        std::uint32_t src_ip,
        std::uint32_t dst_ip,
        std::uint16_t src_port,
        std::uint16_t dst_port,
        std::uint32_t seq_num,
        std::uint32_t ack_num,
        std::uint8_t flags,
        std::uint16_t window_size,
        std::span<const std::uint8_t> payload
    )
    {

        // TODO: allocate 20 + payload.size() bytes; fill header fields big-endian;
        //       set data_offset byte to 0x50 (5 << 4, no options);
        //       zero checksum field; compute tcp_checksum(src_ip, dst_ip, segment);
        //       patch checksum in big-endian; copy payload after byte 20
        std::vector<std::uint8_t> segment (20uz + payload.size(), 0u);

        // bytes 0-3: src and dst ports (big-endian)
        segment[0] = static_cast<std::uint8_t>((src_port >> 8) & 0xFFu);
        segment[1] = static_cast<std::uint8_t>(src_port & 0xFFu);
        segment[2] = static_cast<std::uint8_t>((dst_port >> 8) & 0xFFu);
        segment[3] = static_cast<std::uint8_t>(dst_port & 0xFFu);

        // bytes 4-7: sequence number (big-endian)
        segment[4] = static_cast<std::uint8_t>((seq_num >> 24) & 0xFFu);
        segment[5] = static_cast<std::uint8_t>((seq_num >> 16) & 0xFFu);
        segment[6] = static_cast<std::uint8_t>((seq_num >> 8) & 0xFFu);
        segment[7] = static_cast<std::uint8_t>(seq_num & 0xFFu);

        // bytes 8-11: acknowledgment number (big-endian)
        segment[8] = static_cast<std::uint8_t>((ack_num >> 24) & 0xFFu);
        segment[9] = static_cast<std::uint8_t>((ack_num >> 16) & 0xFFu);
        segment[10] = static_cast<std::uint8_t>((ack_num >> 8) & 0xFFu);
        segment[11] = static_cast<std::uint8_t>(ack_num & 0xFFu);

        // byte 12: data offset 5 (20 bytes, no options) in high nibble; byte 13: flags
        segment[12] = 0x50;
        segment[13] = flags;

        // bytes 14-15: window size (big-endian); bytes 16-17 stay 0 for checksum computation
        segment[14] = static_cast<std::uint8_t>((window_size >> 8) & 0xFFu);
        segment[15] = static_cast<std::uint8_t>(window_size & 0xFFu);

        // copy payload after the header, then compute checksum over the full segment and patch it in
        std::copy(payload.begin(), payload.end(), segment.begin() + 20);
        std::uint16_t checksum { tcp_checksum(src_ip, dst_ip, segment) };
        segment[16] = static_cast<std::uint8_t>((checksum >> 8) & 0xFFu);
        segment[17] = static_cast<std::uint8_t>(checksum & 0xFFu);

        return segment;

    }

    std::vector<std::uint8_t> handle_tcp(TcpConnection& conn, std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment)
    {

        // TODO: parse the incoming segment via parse_tcp;
        //
        // LISTEN: if SYN set and no ACK, record remote seq/port/ip, pick a local ISN,
        //         send SYN-ACK (seq=local_seq, ack=remote_seq+1, flags=SYN|ACK),
        //         advance state to SYN_RECEIVED
        //
        // SYN_RECEIVED: if ACK set and ack_num == local_seq+1,
        //               advance state to ESTABLISHED, return empty (nothing to send)
        //
        // ESTABLISHED: placeholder for phase 5 (data transfer)

        TcpHeader header { parse_tcp(src_ip, dst_ip, segment) };

        switch (conn.state)
        {

            case TcpState::LISTEN:

                if(!(header.flags & TCP_SYN) or (header.flags & TCP_ACK)) return {};
                conn.remote_ip = src_ip;
                conn.local_ip = dst_ip;
                conn.remote_port = header.src_port;
                conn.local_port = header.dst_port;
                conn.remote_seq = header.seq_num;
                conn.local_seq = 1000u;
                
                conn.state = TcpState::SYN_RECEIVED;
                return build_tcp(conn.local_ip, conn.remote_ip, conn.local_port, conn.remote_port, conn.local_seq, conn.remote_seq + 1u, TCP_SYN | TCP_ACK, 65535u, { });

            case TcpState::SYN_RECEIVED:

                // final ack must acknowledge our syn-ack (our isn + 1) to prove they received it
                if ((header.flags & TCP_ACK) && header.ack_num == conn.local_seq + 1u)
                {

                    conn.local_seq += 1u;
                    conn.remote_seq += 1u;

                    conn.state = TcpState::ESTABLISHED;

                }
                return { };

            case TcpState::ESTABLISHED:
                
                // compute header_size from header.data_offset (high nibble * 4) real segments carry no options here (data_offset = 5) 
                std::size_t header_size { static_cast<std::size_t>(static_cast<std::uint8_t>(header.data_offset >> 4) * 4uz ) };

                // slice the payload out of the segment; start @ 1 byte past payload and then grab num bytes of palyoad
                std::span<const std::uint8_t> payload { segment.data() + header_size, segment.size() - header_size };

                // if payload is empty, this is a pure ACK (client acknowledging OUR previous echo). nothing to do — return { }.
                // if we always reply, we ping-pong ACKs forever with no data changing hands.
                if (payload.empty()) return {};

                // does header.seq_num match conn.remote_seq?
                // if YES: they sent the next expected byte. proceed.
                // if NO: this is a retransmission or out-of-order segment. 
                if (header.seq_num != conn.remote_seq)
                {

                    // tell sender what byte we actually need
                    return build_tcp(conn.local_ip, conn.remote_ip, conn.local_port, conn.remote_port, conn.local_seq, conn.remote_seq, TCP_ACK, std::numeric_limits<std::uint16_t>::max(), {});

                }

                // after accepitng incoming data update seq to expect next byte after this payload
                conn.remote_seq += static_cast<std::uint32_t>(payload.size());
                std::vector<std::uint8_t> response { build_tcp(conn.local_ip, conn.remote_ip, conn.local_port, conn.remote_port, conn.local_seq, conn.remote_seq, TCP_ACK | TCP_PSH, std::numeric_limits<std::uint16_t>::max(), payload) };
                conn.local_seq += static_cast<std::uint32_t>(payload.size());
                return response;       
            
        }
        
        return { };

    }

}

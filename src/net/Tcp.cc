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

        // TODO: validate segment >= 20 bytes; extract data_offset (high nibble of byte 12),
        //       validate data_offset >= 5; validate checksum via tcp_checksum == 0;
        //       extract all fields big-endian (src_port, dst_port, seq, ack, flags, window, etc.)
        (void) src_ip;
        (void) dst_ip;
        (void) segment;
        throw std::runtime_error("parse_tcp not implemented");

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
        (void) src_ip;
        (void) dst_ip;
        (void) segment;
        return 0u;

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
        (void) src_ip;
        (void) dst_ip;
        (void) src_port;
        (void) dst_port;
        (void) seq_num;
        (void) ack_num;
        (void) flags;
        (void) window_size;
        (void) payload;
        return { };

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
        (void) conn;
        (void) src_ip;
        (void) dst_ip;
        (void) segment;
        return { };

    }

}

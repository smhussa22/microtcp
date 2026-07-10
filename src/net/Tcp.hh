#ifndef MICROTCP_NET_TCP_HH
#define MICROTCP_NET_TCP_HH

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

    // tcp flag bits in the flags byte (byte 13 of the header)
    constexpr std::uint8_t TCP_FIN { 0x01u }; // finish — sender has no more data
    constexpr std::uint8_t TCP_SYN { 0x02u }; // synchronize — initiate connection
    constexpr std::uint8_t TCP_RST { 0x04u }; // reset — abort connection
    constexpr std::uint8_t TCP_PSH { 0x08u }; // push — deliver data immediately
    constexpr std::uint8_t TCP_ACK { 0x10u }; // acknowledge — ack_num field is valid

    // packed tcp header matching the wire format exactly (no options; minimum 20 bytes)
    struct alignas(1) TcpHeader
    {

        TcpHeader() = default;
        ~TcpHeader() = default;

        TcpHeader(const TcpHeader&) = default;
        TcpHeader& operator=(const TcpHeader&) = default;

        TcpHeader(TcpHeader&&) = default;
        TcpHeader& operator=(TcpHeader&&) = default;

        std::uint16_t src_port { };     // source port number (big-endian)
        std::uint16_t dst_port { };     // destination port number (big-endian)
        std::uint32_t seq_num { };      // sequence number of first data byte in this segment (big-endian)
        std::uint32_t ack_num { };      // next seq number expected from the other side; valid when ACK set (big-endian)
        std::uint8_t data_offset { };   // high nibble = header length in 32-bit words (min 5); low nibble = reserved
        std::uint8_t flags { };         // control bits: FIN SYN RST PSH ACK (low 5 bits used here)
        std::uint16_t window_size { };  // number of bytes the sender is willing to receive (big-endian)
        std::uint16_t checksum { };     // ones' complement checksum over pseudo-header + tcp segment (big-endian)
        std::uint16_t urgent_ptr { };   // only valid when URG flag set; unused in this stack (big-endian)

    };

    static_assert(sizeof(TcpHeader) == 20uz);

    // tracks state of a single tcp connection through the three-way handshake
    enum class TcpState : std::uint8_t
    {

        LISTEN,        // waiting for an incoming SYN
        SYN_RECEIVED,  // sent SYN-ACK; waiting for final ACK to complete handshake
        ESTABLISHED,   // handshake complete; connection is open

    };

    // holds everything needed to track one connection across multiple packets
    struct TcpConnection
    {

        TcpConnection() = default;
        ~TcpConnection() = default;

        TcpConnection(const TcpConnection&) = default;
        TcpConnection& operator=(const TcpConnection&) = default;

        TcpConnection(TcpConnection&&) = default;
        TcpConnection& operator=(TcpConnection&&) = default;

        TcpState state { TcpState::LISTEN }; // current handshake state
        std::uint32_t local_seq { };         // our sequence number (ISN we chose in SYN-ACK)
        std::uint32_t remote_seq { };        // peer's sequence number (from their SYN)
        std::uint16_t local_port { };        // our port (dst port of the incoming SYN)
        std::uint16_t remote_port { };       // peer's port (src port of the incoming SYN)
        std::uint32_t local_ip { };          // our ip address
        std::uint32_t remote_ip { };         // peer's ip address

    };

    // parse a buffer as a tcp segment; validate length, data_offset, and checksum
    TcpHeader parse_tcp(std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment);

    // compute tcp checksum: rfc 1071 ones' complement sum over pseudo-header + tcp segment
    std::uint16_t tcp_checksum(std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment);

    // build a tcp segment with correct header fields and checksum; payload may be empty
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
    );

    // given an incoming tcp segment and current connection state, drive the handshake;
    // returns a segment to send back (empty if nothing should be sent)
    std::vector<std::uint8_t> handle_tcp(TcpConnection& conn, std::uint32_t src_ip, std::uint32_t dst_ip, std::span<const std::uint8_t> segment);

}

#endif // MICROTCP_NET_TCP_HH

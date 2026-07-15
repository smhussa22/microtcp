// related headers

// c sys headers

// cpp stdlib headers
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// 3rd party headers
#include <gtest/gtest.h>

// project headers
#include "net/Tcp.hh"

using namespace microtcp::net;

// ips used for the pseudo-header in every test
static constexpr std::uint32_t remote_ip { 0xC0A80001u }; // 192.168.0.1 (client)
static constexpr std::uint32_t local_ip { 0xC0A80002u };  // 192.168.0.2 (our stack)

// hand-computed valid syn segment: 0x1234 -> 8080, seq=100, window=0xFFFF, no payload
//   pseudo-header sum: 0xC0A8+0x0001+0xC0A8+0x0002+0x0006+0x0014 = 0x1816D
//   segment sum (checksum zeroed): 0x1234+0x1F90+0x0064+0x5002+0xFFFF = 0x18229
//   total 0x30396 -> fold 0x0399 -> ~0x0399 = 0xFC66
static constexpr std::array<std::uint8_t, 20uz> valid_syn {
    0x12, 0x34,             // src_port = 0x1234
    0x1F, 0x90,             // dst_port = 8080
    0x00, 0x00, 0x00, 0x64, // seq_num = 100
    0x00, 0x00, 0x00, 0x00, // ack_num = 0
    0x50, 0x02,             // data_offset = 5, flags = SYN
    0xFF, 0xFF,             // window = 65535
    0xFC, 0x66,             // checksum
    0x00, 0x00              // urgent_ptr = 0
};

// checksum computed over a valid segment (including its checksum field) equals 0x0000
TEST(TcpTest, ChecksumOverValidSegmentIsZero)
{

    std::uint16_t result { tcp_checksum(remote_ip, local_ip, valid_syn) };
    EXPECT_EQ(result, 0u);

}

// checksum computed with checksum field zeroed matches the hand-computed value
TEST(TcpTest, ChecksumMatchesKnownVector)
{

    std::array<std::uint8_t, 20uz> segment { valid_syn };
    segment[16] = 0x00;
    segment[17] = 0x00;

    std::uint16_t result { tcp_checksum(remote_ip, local_ip, segment) };
    EXPECT_EQ(result, 0xFC66u);

}

// same segment bytes but wrong ips must fail validation (pseudo-header is doing its job)
TEST(TcpTest, ChecksumFailsWithWrongIps)
{

    std::uint16_t result { tcp_checksum(remote_ip, 0x0A0A0A0Au, valid_syn) };
    EXPECT_NE(result, 0u);

}

// parsing a buffer shorter than 20 bytes throws
TEST(TcpTest, ParseThrowsOnTruncatedBuffer)
{

    std::array<std::uint8_t, 10uz> short_buf { };
    EXPECT_THROW(parse_tcp(remote_ip, local_ip, short_buf), std::runtime_error);

}

// parsing a segment with data offset < 5 throws
TEST(TcpTest, ParseThrowsOnBadDataOffset)
{

    std::array<std::uint8_t, 20uz> segment { valid_syn };
    segment[12] = 0x40; // data offset = 4
    EXPECT_THROW(parse_tcp(remote_ip, local_ip, segment), std::runtime_error);

}

// parsing a segment with a corrupted checksum throws
TEST(TcpTest, ParseThrowsOnBadChecksum)
{

    std::array<std::uint8_t, 20uz> segment { valid_syn };
    segment[16] = 0xDE;
    segment[17] = 0xAD;
    EXPECT_THROW(parse_tcp(remote_ip, local_ip, segment), std::runtime_error);

}

// parsing the valid syn extracts every field correctly
TEST(TcpTest, ParseValidSegmentExtractsFields)
{

    TcpHeader hdr { parse_tcp(remote_ip, local_ip, valid_syn) };

    EXPECT_EQ(hdr.src_port, 0x1234u);
    EXPECT_EQ(hdr.dst_port, 8080u);
    EXPECT_EQ(hdr.seq_num, 100u);
    EXPECT_EQ(hdr.ack_num, 0u);
    EXPECT_EQ(hdr.data_offset, 0x50u);
    EXPECT_EQ(hdr.flags, TCP_SYN);
    EXPECT_EQ(hdr.window_size, 0xFFFFu);
    EXPECT_EQ(hdr.checksum, 0xFC66u);
    EXPECT_EQ(hdr.urgent_ptr, 0u);

}

// build produces a segment parse can round-trip cleanly, payload included
TEST(TcpTest, BuildAndParseRoundTrip)
{

    std::array<std::uint8_t, 4uz> payload { 0xDE, 0xAD, 0xBE, 0xEF };
    std::vector<std::uint8_t> segment { build_tcp(local_ip, remote_ip, 8080u, 0x1234u, 1000u, 101u, TCP_ACK, 65535u, payload) };

    TcpHeader hdr { parse_tcp(local_ip, remote_ip, segment) };

    EXPECT_EQ(segment.size(), 24uz);
    EXPECT_EQ(hdr.src_port, 8080u);
    EXPECT_EQ(hdr.dst_port, 0x1234u);
    EXPECT_EQ(hdr.seq_num, 1000u);
    EXPECT_EQ(hdr.ack_num, 101u);
    EXPECT_EQ(hdr.flags, TCP_ACK);

}

// a syn in LISTEN produces a syn-ack with correct seq/ack and advances state
TEST(TcpTest, HandshakeSynGetsSynAck)
{

    TcpConnection conn { };
    std::vector<std::uint8_t> reply { handle_tcp(conn, remote_ip, local_ip, valid_syn) };

    ASSERT_FALSE(reply.empty());
    EXPECT_EQ(conn.state, TcpState::SYN_RECEIVED);
    EXPECT_EQ(conn.remote_seq, 100u);

    // reply travels local -> remote, so the pseudo-header ips flip
    TcpHeader hdr { parse_tcp(local_ip, remote_ip, reply) };
    EXPECT_EQ(hdr.flags, TCP_SYN | TCP_ACK);
    EXPECT_EQ(hdr.src_port, 8080u);
    EXPECT_EQ(hdr.dst_port, 0x1234u);
    EXPECT_EQ(hdr.seq_num, conn.local_seq);
    EXPECT_EQ(hdr.ack_num, 101u); // their isn + 1; the syn consumed one seq number

}

// the final ack (ack = our isn + 1) completes the handshake
TEST(TcpTest, HandshakeAckEstablishes)
{

    TcpConnection conn { };
    handle_tcp(conn, remote_ip, local_ip, valid_syn);

    std::vector<std::uint8_t> ack { build_tcp(remote_ip, local_ip, 0x1234u, 8080u, 101u, conn.local_seq + 1u, TCP_ACK, 65535u, { }) };
    std::vector<std::uint8_t> reply { handle_tcp(conn, remote_ip, local_ip, ack) };

    EXPECT_TRUE(reply.empty());
    EXPECT_EQ(conn.state, TcpState::ESTABLISHED);

}

// an ack with the wrong ack_num must not establish the connection
TEST(TcpTest, HandshakeRejectsWrongAckNum)
{

    TcpConnection conn { };
    handle_tcp(conn, remote_ip, local_ip, valid_syn);

    std::vector<std::uint8_t> bad_ack { build_tcp(remote_ip, local_ip, 0x1234u, 8080u, 101u, conn.local_seq + 999u, TCP_ACK, 65535u, { }) };
    handle_tcp(conn, remote_ip, local_ip, bad_ack);

    EXPECT_EQ(conn.state, TcpState::SYN_RECEIVED);

}

// a non-syn segment in LISTEN is ignored and produces no reply
TEST(TcpTest, ListenIgnoresNonSyn)
{

    TcpConnection conn { };
    std::vector<std::uint8_t> stray_ack { build_tcp(remote_ip, local_ip, 0x1234u, 8080u, 100u, 1u, TCP_ACK, 65535u, { }) };
    std::vector<std::uint8_t> reply { handle_tcp(conn, remote_ip, local_ip, stray_ack) };

    EXPECT_TRUE(reply.empty());
    EXPECT_EQ(conn.state, TcpState::LISTEN);

}

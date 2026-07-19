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
#include <vector>

// 3rd party headers

// project headers
#include "net/IPv4.hh"
#include "net/Icmp.hh"
#include "net/Tcp.hh"
#include "tun/TunDevice.hh"

void print_ip(std::uint32_t ip);
std::string_view protocol_name(std::uint8_t protocol);

int main()
{

    // attach to tun0; for each incoming packet, parse the ipv4 header and print a summary
    microtcp::tun::TunDevice tun { "tun0" };
    std::array<std::uint8_t, 1500uz> buffer { };

    // single-connection tcp state; must outlive the loop since the handshake spans multiple packets
    microtcp::net::TcpConnection conn { };

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

            // reply to icmp echo requests addressed to our stack (10.0.0.2)
            std::size_t header_size { static_cast<std::size_t>((hdr.version_ihl & 0x0F) * 4) };
            if (hdr.protocol == 1u && hdr.dst_ip == 0x0A000002u && buffer[header_size] == 8u)
            {

                std::span<const std::uint8_t> icmp { buffer.data() + header_size, hdr.total_length - header_size };
                std::vector<std::uint8_t> reply { microtcp::net::build_icmp_echo_reply(icmp) };
                std::vector<std::uint8_t> packet { microtcp::net::build_ipv4(hdr.dst_ip, hdr.src_ip, 1u, reply) };
                tun.write(packet);
                std::println("[icmp] echo reply sent (seq={})", (icmp[6] << 8) | icmp[7]);

            }

            // drive the tcp handshake for segments addressed to our stack (10.0.0.2)
            if (hdr.protocol == 6u && hdr.dst_ip == 0x0A000002u)
            {

                std::span<const std::uint8_t> segment { buffer.data() + header_size, hdr.total_length - header_size };
                microtcp::net::TcpState before { conn.state };
                std::vector<std::uint8_t> reply { microtcp::net::handle_tcp(conn, hdr.src_ip, hdr.dst_ip, segment) };

                if (!reply.empty())
                {

                    std::vector<std::uint8_t> packet { microtcp::net::build_ipv4(hdr.dst_ip, hdr.src_ip, 6u, reply) };
                    tun.write(packet);

                    // TODO(phase 5): update this print to reflect what was sent, not just "syn-ack".
                    //                after phase 5, a non-empty reply might be a syn-ack (during handshake)
                    //                OR a data segment (during echo). options:
                    //                  a) print based on `before` state: LISTEN->syn-ack, ESTABLISHED->data reply
                    //                  b) print the payload size: "[tcp] sent {} bytes" ({} = reply.size() - 20)
                    //                  c) both — one message per case
                    std::println("[tcp] syn received; syn-ack sent");

                }

                if (before != conn.state && conn.state == microtcp::net::TcpState::ESTABLISHED) std::println("[tcp] connection established");
                if (before == microtcp::net::TcpState::ESTABLISHED && conn.state == microtcp::net::TcpState::LAST_ACK) std::println("[tcp] fin received; fin-ack sent");
                if (before == microtcp::net::TcpState::LAST_ACK && conn.state == microtcp::net::TcpState::LISTEN) std::println("[tcp] connection closed; ready for next");

            }

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

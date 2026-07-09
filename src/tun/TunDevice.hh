#ifndef MICROTCP_TUN_TUN_DEVICE_HH
#define MICROTCP_TUN_TUN_DEVICE_HH

// related headers

// c sys headers
#include <cstring>
#include <cstddef>
#include <cstdint>

// cpp stdlib headers
#include <span>
#include <string>
#include <string_view>
#include <stdexcept>

// 3rd party headers
#include <fcntl.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <unistd.h>

// project headers

namespace microtcp::tun
{

    // raii wrapper around /dev/net/tun attached to a specific tun interface
    class TunDevice
    {

    public:

        // opens /dev/net/tun and attaches this process to the given interface name
        explicit TunDevice(std::string_view device_name);
        ~TunDevice();

        TunDevice(const TunDevice&) = delete;
        TunDevice& operator=(const TunDevice&) = delete;
        TunDevice(TunDevice&&) = delete;
        TunDevice& operator=(TunDevice&&) = delete;

        // blocks until the kernel hands us a packet; returns bytes read into buffer
        std::size_t read(std::span<std::uint8_t> buffer);

        // writes a raw ip packet back to the tun device; returns bytes written
        std::size_t write(std::span<const std::uint8_t> buffer);

        // exposes the underlying fd for future use with select/poll/epoll
        int fd() const noexcept;

    private:

        int m_fd { -1 };           // file descriptor for /dev/net/tun, -1 if unopened
        std::string m_device_name; // interface name we are attached to, e.g. "tun0"

    };

}

#endif // MICROTCP_TUN_TUN_DEVICE_HH

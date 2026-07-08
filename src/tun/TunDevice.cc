// related headers
#include "TunDevice.hh"

// c sys headers

// cpp stdlib headers
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

// 3rd party headers

// project headers

namespace microtcp::tun
{

    TunDevice::TunDevice(std::string_view device_name) : m_device_name { device_name }
    {

        // TODO: open("/dev/net/tun", O_RDWR), fill ifreq with IFF_TUN | IFF_NO_PI, ioctl(TUNSETIFF)

    }

    TunDevice::~TunDevice()
    {

        // TODO: if (m_fd >= 0) ::close(m_fd)

    }

    std::size_t TunDevice::read(std::span<std::uint8_t> buffer)
    {

        // TODO: ssize_t n = ::read(m_fd, buffer.data(), buffer.size()); handle error; return static_cast<size_t>(n)
        (void)buffer;
        return 0uz;

    }

    std::size_t TunDevice::write(std::span<const std::uint8_t> buffer)
    {

        // TODO: ssize_t n = ::write(m_fd, buffer.data(), buffer.size()); handle short write; return bytes written
        (void)buffer;
        return 0uz;

    }

    int TunDevice::fd() const noexcept
    {

        return m_fd;

    }

}

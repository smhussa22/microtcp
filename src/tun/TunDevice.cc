// related headers
#include "TunDevice.hh"

// c sys headers

// cpp stdlib headers

// 3rd party headers

// project headers

namespace microtcp::tun
{

    TunDevice::TunDevice(std::string_view device_name) : m_device_name { device_name }
    {

        // open tun driver
        int tun_fd { ::open("/dev/net/tun", O_RDWR) };
        if (tun_fd < 0) throw std::runtime_error(strerror(errno));

        // create interface request for settings to give kernel
        ::ifreq ifr { };

        // we want to attach to interface named tun0 in the kernel file descriptor table
        strncpy(ifr.ifr_name, m_device_name.data(), IFNAMSIZ - 1);
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI; // create tun device | dont add the 4 byte packet info header 

        // set our tun driver with these stetings
        if (::ioctl(tun_fd, TUNSETIFF, &ifr) < 0) 
        {

            ::close(tun_fd);
            throw std::runtime_error(strerror(errno));

        }

        m_fd = tun_fd;

    }

    TunDevice::~TunDevice()
    {

        if (m_fd >= 0) ::close(m_fd);

    }

    std::size_t TunDevice::read(std::span<std::uint8_t> buffer)
    {
        

        ssize_t n_bytes_read { ::read(m_fd, buffer.data(), buffer.size()) };
        if (n_bytes_read < 0) throw std::runtime_error(strerror(errno));
        return static_cast<std::size_t>(n_bytes_read);

    }

    std::size_t TunDevice::write(std::span<const std::uint8_t> buffer)
    {

        ssize_t n_bytes_wrote { ::write(m_fd, buffer.data(), buffer.size()) };
        
        // no bytes written at all
        if(n_bytes_wrote < 0) throw std::runtime_error(strerror(errno));
        // short writes
        if (static_cast<std::size_t>(n_bytes_wrote) != buffer.size()) throw std::runtime_error("short write");
        return static_cast<std::size_t>(n_bytes_wrote);

    }

    int TunDevice::fd() const noexcept
    {

        return m_fd;

    }

}

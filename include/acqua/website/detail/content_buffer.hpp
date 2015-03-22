#pragma once

namespace acqua { namespace website { namespace detail {

class no_content_buffer
{
public:
    friend std::ostream & operator<<(std::ostream & os, no_content_buffer const &)
    {
        os << "\r\n";
        return os;
    }
};

template <typename T>
class mapped_content_buffer
{
public:
    explicit mapped_content_buffer(T const & t)
        : t_(t) {}

    friend std::ostream & operator<<(std::ostream & os, mapped_content_buffer const & rhs)
    {
        boost::asio::streambuf buf;
        std::ostream oss(&buf);

        auto it = rhs.t_.begin();
        if (it != rhs.t_.end()) {
            oss << it->first << '=' << it->second;
            while(++it != rhs.t_.end()) {
                oss << '&' << it->first << '=' << it->second;
            }
        }

        os <<
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: " << buf.size() << "\r\n"
            "\r\n" << &buf;

        return os;
    }

private:
    T const & t_;
};

} } }

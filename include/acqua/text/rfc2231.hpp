#pragma once

namespace acqua { namespace text {

template <typename CharT>
class rfc2231_encoder
{
public:
    explicit rfc2231_encoder(std::basic_string<CharT> const & key, std::string const & charset = "US-ASCII",
                             linefeed_type linefeed = linefeed_type::crln, std::size_t indent = 0, std::size_t width = 80)
        : key_(key), charset_(charset), linefeed_(linefeed) {}

    void put(std::basic_string<CharT> const & str)
    {
    }

    std::string str()
    {
        return oss_.str();
    }

private:
    std::basic_string<CharT> key_;
    std::string charset_;
    linefeed_type linefeed_;
    std::ostringstream oss_;
};

template <typename CharT>
class rfc2231_decoder
{
public:
    explicit rfc2231_decoder(std::basic_string<CharT> const & key, std::string const & charset = "US-ASCII")
        : key_(key), charset_(charset) {}

    void put(std::string const & str)
    {
    }

    std::basic_string<CharT> str()
    {
        return boost::locale::conv::to_utf<CharT>(oss_.str(), charset_);
    }

private:
    std::basic_string<CharT> key_;
    std::string charset_;
    std::ostringstream oss_;
};

} }

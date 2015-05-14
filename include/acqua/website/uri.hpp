#pragma once

#include <boost/blank.hpp>
#include <acqua/website/client_impl/socket_base.hpp>

namespace acqua { namespace website {

class uri_base {};

namespace client_impl {

template <typename Uri, typename Query = boost::blank, typename Header = boost::blank>
class non_encoded_uri
    : public uri_base
{
public:
    using const_iterator = typename std::decay<Uri>::type::const_iterator;

    explicit non_encoded_uri(Uri uri)
        : uri_(uri) {}

    const_iterator begin() const
    {
        return uri_.begin();
    }

    const_iterator end() const
    {
        return uri_.end();
    }

    void query(std::ostream & os) const
    {
        write_query(os, query_);
    }

    void header(std::ostream & os) const
    {
        write_header(os, header_);
    }

private:
    void write_query(std::ostream &, boost::blank) const
    {
    }

    template <typename Map, typename Map::mapped_type * = nullptr>
    void write_query(std::ostream & os, Map const & map) const
    {
        auto it = map.begin();
        if (it != map.end()) {
            os << '?';
            os << it->first << '=' << it->second;
            while(++it != map.end()) {
                os << '&';
                os << it->first << '=' << it->second;
            }
        }
    }

    void write_query(std::ostream & os, char const * str) const
    {
        if (str && *str != '\0') {
            os << '?';
            os << str;
        }
    }

    void write_header(std::ostream &, boost::blank) const
    {
    }

private:
    Uri uri_;
    Query query_;
    Header header_;
};

}

inline client_impl::non_encoded_uri<std::string const &> uri(std::string const & uri)
{
    return client_impl::non_encoded_uri<std::string const &>(uri);
}


} }

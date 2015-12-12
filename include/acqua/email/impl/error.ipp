#pragma once

#include <acqua/email/error.hpp>

namespace acqua { namespace email { namespace error {

namespace detail {

class address_category
    : public boost::system::error_category
{
public:
    char const * name() const noexcept override
    {
        return "email.address";
    }

    std::string message(int ev) const override
    {
        switch(static_cast<enum address_errors>(ev)) {
            case error::not_address:
                return "Not address";
            default:
                return "error";
        }
    }
};

} // detail

inline boost::system::error_category const & get_address_category()
{
    static detail::address_category instance;
    return instance;
}

} } }

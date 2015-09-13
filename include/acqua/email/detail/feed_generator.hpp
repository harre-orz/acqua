#pragma once

namespace acqua { namespace email { namespace detail {

template <typename Mail>
class feed_generator
{
    class impl;

public:
    using char_type = typename Mail::char_type;
    using traits_type = typename Mail::traits_type;

private:
    using const_iterator = typename Mail::const_iterator;

public:
    feed_generator(Mail & mail)
        : impl_(new impl(mail))
        , line_()
        , it_(line_.end())
    {
    }

    bool is_terminated() const
    {
        return impl_->is_terminated() && it_ == line_.end();
    }

    feed_generator & generate(char & ch)
    {
        if (it_ == line_.end()) {
            line_.clear();
            if (!impl_->do_generate_line(line_))
                return *this;
            it_ = line_.begin();

        }

        ch = *it_++;
        return *this;
    }

    friend std::ostream & operator<<(std::ostream & os, feed_generator & rhs)
    {
        char ch;
        while(!rhs.is_terminated()) {
            rhs.generate(ch);
            os.put(ch);
        }
        return os;
    }

private:
    std::unique_ptr<impl> impl_;
    std::string line_;
    std::string::iterator it_;
};

} } }

#include <acqua/email/detail/impl/feed_generator_impl.ipp>

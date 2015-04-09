#pragma once

#include <iostream>
#include <boost/mpl/list_c.hpp>
#include <boost/mpl/for_each.hpp>


namespace acqua { namespace text {

//! 改行を行わない
class no_line_wrap
{
public:
    static constexpr std::size_t count = std::numeric_limits<std::size_t>::max();

public:
    no_line_wrap & operator++() noexcept { return *this; }
    no_line_wrap & operator++(int) noexcept { return *this; }
    no_line_wrap & operator+=(std::size_t) noexcept { return *this; }

    template <typename T>
    bool operator()(T &) { return false; }
};

//! N行ごとに改行コードを挿入するクラス
template <std::size_t N, typename LineChars>
class line_wrap
{
public:
    static constexpr std::size_t count = N;

public:
    line_wrap & operator++() noexcept { ++n_; return *this; }
    line_wrap & operator++(int) noexcept { ++n_; return *this; }
    line_wrap & operator+=(std::size_t) noexcept { return *this; }

    template <typename OutputIterator>
    bool operator()(OutputIterator & it) noexcept
    {
        static_assert(N > 0, "N > 0");

        if (n_ < N)
            return false;
        n_ = 0;
        boost::mpl::for_each<LineChars>([&it](char ch) { it++ = ch; });
        return true;
    }

    template <typename Ch, typename Tr>
    bool operator()(std::basic_ostream<Ch, Tr> & os)
    {
        static_assert(N > 0, "N > 0");

        if (n_ < N)
            return false;
        n_ = 0;
        boost::mpl::for_each<LineChars>([&os](char ch) { os << ch; });
        return true;
    }

    template <typename Ch, typename Tr, typename A>
    bool operator()(std::basic_string<Ch, Tr, A> & str)
    {
        static_assert(N > 0, "N > 0");

        if (n_ < N)
            return false;
        n_ = 0;
        boost::mpl::for_each<LineChars>([&str](char ch) { str += ch; });
        return true;
    }

private:
    std::size_t n_;
};

//! N行ごとに CRコードを挿入するクラス
template <std::size_t N>
using cr_line_wrap = line_wrap<N, boost::mpl::list_c<char, '\r'> >;

//! N行ごとに LNコードを挿入するクラス
template <std::size_t N>
using ln_line_wrap = line_wrap<N, boost::mpl::list_c<char, '\n'> >;

//! N行ごとに CRLNコードを挿入するクラス
template <std::size_t N>
using crln_line_wrap = line_wrap<N, boost::mpl::list_c<char, '\r', '\n'> >;

} }

#pragma once

namespace acqua { namespace email { namespace detail {

template <typename CharT>
class literals
{
protected:
    static CharT const * const content_type;
    static CharT const * const content_transfer_encoding;
    static CharT const * const subject;
    static CharT const * const from;
    static CharT const * const to;
    static CharT const * const cc;
    static CharT const * const bcc;
    static CharT const * const boundary;
    static CharT const * const charset;
    static CharT const * const base64;
    static CharT const * const quoted_printable;
    static CharT const * const name;
    static CharT const * const filename;
    static CharT const * const text_;
    static CharT const * const format;
    static CharT const * const flowed;
    static CharT const * const delsp;
    static CharT const * const yes;
};

template<> char const * const literals<char>::content_type = "Content-Type";
template<> char const * const literals<char>::content_transfer_encoding = "Content-Transfer-Encoding";
template<> char const * const literals<char>::subject = "Subject";
template<> char const * const literals<char>::from = "From";
template<> char const * const literals<char>::to = "To";
template<> char const * const literals<char>::cc = "Cc";
template<> char const * const literals<char>::bcc = "Bcc";
template<> char const * const literals<char>::boundary = "boundary";
template<> char const * const literals<char>::charset = "charset";
template<> char const * const literals<char>::base64 = "base64";
template<> char const * const literals<char>::quoted_printable = "quoted-printable";
template<> char const * const literals<char>::text_ = "text/";
template<> char const * const literals<char>::format = "format";
template<> char const * const literals<char>::flowed = "flowed";
template<> char const * const literals<char>::delsp = "delsp";
template<> char const * const literals<char>::yes = "yes";

} } }

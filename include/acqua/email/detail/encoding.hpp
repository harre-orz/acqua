#pragma once

#include <acqua/email/headers.hpp>
#include <iostream>

namespace acqua { namespace email { namespace detail {

enum class encoding { ascii, qprint, base64 };

std::ostream & operator<<(std::ostream & os, encoding enc)
{
    switch(enc) {
        case encoding::ascii:
            return os << "7bit or 8bit";
        case encoding::qprint:
            return os << "quoted-printable";
        case encoding::base64:
            return os << "base64";
    }
    return os;
}


inline void get_encoding_params(headers const & h, std::string & charset, std::string const *& boundary,
                                encoding & enc, bool & text_mode,
                                bool & is_format_flowed, bool & is_delete_space)
{
    charset = "US-ASCII";
    boundary = nullptr;
    enc = encoding::ascii;
    text_mode = false;
    is_format_flowed = false;
    is_delete_space = false;

    auto it = h.find("Content-Type");
    if (it != h.end()) {
        auto & contenttype = it->second;
        // バウンダリを探す
        auto it2 = contenttype.find("boundary");
        if (it2 != contenttype.end())
            boundary = &it2->second;

        // Content-Type による改行コードの自動変換
        if ((text_mode = boost::algorithm::istarts_with(contenttype.str(), "text/"))) {
            it2 = contenttype.find("charset");
            if (it2 != contenttype.end())
                charset = it2->second;

            // RFC3676 対応
            it2 = contenttype.find("format");
            if (it2 != contenttype.end())
                is_format_flowed = boost::algorithm::iequals(it2->second, "flowed");
            it2 = contenttype.find("delsp");
            if (it2 != contenttype.end())
                is_delete_space = boost::algorithm::iequals(it2->second, "yes");
        }
    }

    it = h.find("Content-Transfer-Encoding");
    if (it != h.end()) {
        auto & content_transfer_encoding = it->second;
        if (boost::algorithm::iequals(content_transfer_encoding.str(), "quoted-printable")) {
            enc = encoding::qprint;
            text_mode = true;
        }
    }
}

} } }

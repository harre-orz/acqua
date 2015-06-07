#define BOOST_TEST_MAIN    // main関数を定義

#include <acqua/text/rfc2047.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(rfc2047)

// BOOST_AUTO_TEST_CASE(rfc2047_encode)
// {
//     acqua::text::rfc2047_encoder<char> enc("iso-2022-jp", acqua::text::crln, 10, 20);
//     enc.push("あいうえおあいうえおあいうえおあいうえお");
//     std::cout << enc.str() << std::endl;

//     acqua::text::rfc2047_decoder<char> dec;
//     dec.push(enc.str());
//     std::cout << dec.str() << std::endl;
// }

BOOST_AUTO_TEST_CASE(rfc2047_decoder)
{
    acqua::text::rfc2047_decoder dec;
    dec.parse("=?utf-8?B?aGVsbG8K?=");
}

BOOST_AUTO_TEST_SUITE_END()

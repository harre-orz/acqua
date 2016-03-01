#include <acqua/iostreams/base64_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(base64_filter)

namespace io = acqua::iostreams;

template <typename Filter>
std::string fi(Filter filter, std::string const & text)
{
    std::ostringstream oss;
    std::istringstream iss(text);
    boost::iostreams::filtering_istream in;
    in.push(filter);
    in.push(iss);
    boost::iostreams::copy(in, oss);
    in.reset();
    return oss.str();
}

template <typename Filter>
std::string fo(Filter filter, std::string const & text)
{
    std::ostringstream oss;
    boost::iostreams::filtering_ostream out;
    out.push(filter);
    out.push(oss);
    out << text;
    out.reset();
    return oss.str();
}

BOOST_AUTO_TEST_CASE(encode_tiny_text)
{
    BOOST_TEST(fi(io::base64_encoder(), "hello world!") == "aGVsbG8gd29ybGQh");
    BOOST_TEST(fo(io::base64_encoder(), "hello world!") == "aGVsbG8gd29ybGQh");

    BOOST_TEST(fi(io::base64_encoder(), "hello") == "aGVsbG8=");
    BOOST_TEST(fo(io::base64_encoder(), "hello") == "aGVsbG8=");

    BOOST_TEST(fi(io::base64_encoder(), "1234567890") == "MTIzNDU2Nzg5MA==");
    BOOST_TEST(fo(io::base64_encoder(), "1234567890") == "MTIzNDU2Nzg5MA==");

}

BOOST_AUTO_TEST_CASE(encode_short_text)
{
    BOOST_TEST(fi(io::base64_encoder(io::newline::none, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp"
               "dmluZy4=");
    BOOST_TEST(fo(io::base64_encoder(io::newline::none, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp"
               "dmluZy4=");


    BOOST_TEST(fi(io::base64_encoder(io::newline::ln, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\n"
               "dmluZy4=\n");
    BOOST_TEST(fo(io::base64_encoder(io::newline::ln, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\n"
               "dmluZy4=\n");


    BOOST_TEST(fi(io::base64_encoder(io::newline::cr, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r"
               "dmluZy4=\r");
    BOOST_TEST(fo(io::base64_encoder(io::newline::cr, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r"
               "dmluZy4=\r");


    BOOST_TEST(fi(io::base64_encoder(io::newline::crln, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\n"
               "dmluZy4=\r\n");
    BOOST_TEST(fo(io::base64_encoder(io::newline::crln, 80),
                  "I don't dream at night, I dream all day; I dream for a living.") ==
               "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\n"
               "dmluZy4=\r\n");
}

BOOST_AUTO_TEST_CASE(encode_empty)
{
    BOOST_TEST(fi(io::base64_encoder(io::newline::none), "") == "");
    BOOST_TEST(fo(io::base64_encoder(io::newline::none), "") == "");

    BOOST_TEST(fi(io::base64_encoder(io::newline::ln), "") == "");
    BOOST_TEST(fo(io::base64_encoder(io::newline::ln), "") == "");

    BOOST_TEST(fi(io::base64_encoder(io::newline::cr), "") == "");
    BOOST_TEST(fo(io::base64_encoder(io::newline::cr), "") == "");

    BOOST_TEST(fi(io::base64_encoder(io::newline::crln), "") == "");
    BOOST_TEST(fo(io::base64_encoder(io::newline::crln), "") == "");
}

BOOST_AUTO_TEST_CASE(encode_min_size)
{
    BOOST_TEST(fi(io::base64_encoder(io::newline::none, 0), "000") == "MDAw");
    BOOST_TEST(fo(io::base64_encoder(io::newline::none, 0), "000") == "MDAw");

    BOOST_TEST(fi(io::base64_encoder(io::newline::ln, 0), "000") == "MDAw\n");
    BOOST_TEST(fo(io::base64_encoder(io::newline::ln, 0), "000") == "MDAw\n");

    BOOST_TEST(fi(io::base64_encoder(io::newline::cr, 0), "000") == "MDAw\r");
    BOOST_TEST(fo(io::base64_encoder(io::newline::cr, 0), "000") == "MDAw\r");

    BOOST_TEST(fi(io::base64_encoder(io::newline::crln, 0), "000") == "MDAw\r\n");
    BOOST_TEST(fo(io::base64_encoder(io::newline::crln, 0), "000") == "MDAw\r\n");
}

BOOST_AUTO_TEST_CASE(decode_tiny_text)
{
    BOOST_TEST(fi(io::base64_decoder(), "aGVsbG8gd29ybGQh") == "hello world!");
    BOOST_TEST(fo(io::base64_decoder(), "aGVsbG8gd29ybGQh") == "hello world!");

    BOOST_TEST(fi(io::base64_decoder(), "aGVsbG8=") == "hello");
    BOOST_TEST(fo(io::base64_decoder(), "aGVsbG8=") == "hello");

    BOOST_TEST(fi(io::base64_decoder(), "MTIzNDU2Nzg5MA==") == "1234567890");
    BOOST_TEST(fo(io::base64_decoder(), "MTIzNDU2Nzg5MA==") == "1234567890");
}

BOOST_AUTO_TEST_CASE(decode_short_text)
{
    BOOST_TEST(fi(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp"
                  "dmluZy4=") == "I don't dream at night, I dream all day; I dream for a living.");
    BOOST_TEST(fo(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp"
                  "dmluZy4=") == "I don't dream at night, I dream all day; I dream for a living.");


    BOOST_TEST(fi(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\n"
                  "dmluZy4=\n") == "I don't dream at night, I dream all day; I dream for a living.");
    BOOST_TEST(fo(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\n"
                  "dmluZy4=\n") == "I don't dream at night, I dream all day; I dream for a living.");


    BOOST_TEST(fi(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r"
                  "dmluZy4=\r") == "I don't dream at night, I dream all day; I dream for a living.");
    BOOST_TEST(fo(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r"
                  "dmluZy4=\r") == "I don't dream at night, I dream all day; I dream for a living.");


    BOOST_TEST(fi(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\n"
                  "dmluZy4=\r\n") == "I don't dream at night, I dream all day; I dream for a living.");
    BOOST_TEST(fo(io::base64_decoder(),
                  "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\n"
                  "dmluZy4=\r\n") == "I don't dream at night, I dream all day; I dream for a living.");
}

BOOST_AUTO_TEST_CASE(decode_empty)
{
    BOOST_TEST(fi(io::base64_decoder(), "") == "");
    BOOST_TEST(fo(io::base64_decoder(), "") == "");

    BOOST_TEST(fi(io::base64_decoder(), "\n") == "");
    BOOST_TEST(fo(io::base64_decoder(), "\n") == "");

    BOOST_TEST(fi(io::base64_decoder(), "\r") == "");
    BOOST_TEST(fo(io::base64_decoder(), "\r") == "");

    BOOST_TEST(fi(io::base64_decoder(), "\r\n") == "");
    BOOST_TEST(fo(io::base64_decoder(), "\r\n") == "");
}

BOOST_AUTO_TEST_CASE(convert_long_text)
{
    std::string str =
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. "
        "Aenean commodo ligula eget dolor. Aenean massa. "
        "Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.\n"
        "Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. "
        "Nulla consequat massa quis enim. "
        "Donec pede justo, \"fringilla vel\", aliquet nec, vulputate eget, arcu. "
        "In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo.\n"
        "\n"
        "Nullam dictum felis eu pede mollis pretium. "
        "Integer tincidunt. 'Cras' dapibus. "
        "Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. "
        "Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. "
        "Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus.\n"
        "Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. "
        "Aenean imperdiet. Etiam ultricies nisi vel augue. "
        "Curabitur ullamcorper ultricies nisi. Nam eget dui. "
        "\"Etiam\" rhoncus.\n"
        "Maecenas tempus, tellus eget condimentum rhoncus, "
        "sem quam semper libero, sit amet adipiscing sem neque sed ipsum. "
        "Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. "
        "Maecenas nec odio et ante tincidunt tempus. "
        "Donec vitae sapien ut libero venenatis faucibus. "
        "Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. "
        "Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. "
        "Sed consequat, leo eget bibendum sodales, augue velit cursus nunc,\n";

    BOOST_TEST(fo(io::base64_decoder(), fi(io::base64_encoder(io::newline::none, 80), str)) == str);
    BOOST_TEST(fo(io::base64_decoder(), fi(io::base64_encoder(io::newline::ln, 80), str)) == str);
    BOOST_TEST(fo(io::base64_decoder(), fi(io::base64_encoder(io::newline::cr, 80), str)) == str);
    BOOST_TEST(fo(io::base64_decoder(), fi(io::base64_encoder(io::newline::crln, 80), str)) == str);
}

BOOST_AUTO_TEST_CASE(convert_binary)
{
    std::string binary;
    for(int i = 0; i < 256; ++i) binary += static_cast<char>(i);

    std::string encoded =
        "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+"
        "P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+"
        "AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/"
        "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==";

    BOOST_TEST(fi(io::base64_encoder(), binary) == encoded);
    BOOST_TEST(fo(io::base64_encoder(), binary) == encoded);

    BOOST_TEST(fi(io::base64_decoder(), encoded) == binary);
    BOOST_TEST(fo(io::base64_decoder(), encoded) == binary);

    // base64_url (パディングなし、+ => -、/ => _)
    boost::algorithm::replace_all(encoded, "=", "");
    boost::algorithm::replace_all(encoded, "+", "-");
    boost::algorithm::replace_all(encoded, "/", "_");

    BOOST_TEST(fi(io::base64_url_encoder(), binary) == encoded);
    BOOST_TEST(fo(io::base64_url_encoder(), binary) == encoded);

    BOOST_TEST(fi(io::base64_url_decoder(), encoded) == binary);
    BOOST_TEST(fo(io::base64_url_decoder(), encoded) == binary);
}

BOOST_AUTO_TEST_SUITE_END()

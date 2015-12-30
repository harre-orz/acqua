#include <acqua/iostreams/base64_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(base64_filter)

BOOST_AUTO_TEST_CASE(encoder)
{
    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::base64_encoder(acqua::iostreams::newline::ln, 76));
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() ==
                   "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\n"
                   "dmluZy4=\n");
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::base64_encoder(acqua::iostreams::newline::cr, 76));
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() ==
                   "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r"
                   "dmluZy4=\r");
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::base64_encoder(acqua::iostreams::newline::crln, 76));
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() ==
                   "SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\n"
                   "dmluZy4=\r\n");
    } while(false);
}

BOOST_AUTO_TEST_CASE(decoder)
{
    do {
        std::istringstream iss("SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\ndmluZy4=\n");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::base64_decoder());
        in.push(iss);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == "I don't dream at night, I dream all day; I dream for a living.");
    } while(false);

    do {
        std::istringstream iss("SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\rdmluZy4=\r");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::base64_decoder());
        in.push(iss);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == "I don't dream at night, I dream all day; I dream for a living.");
    } while(false);

    do {
        std::istringstream iss("SSBkb24ndCBkcmVhbSBhdCBuaWdodCwgSSBkcmVhbSBhbGwgZGF5OyBJIGRyZWFtIGZvciBhIGxp\r\ndmluZy4=\r\n");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::base64_decoder());
        in.push(iss);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == "I don't dream at night, I dream all day; I dream for a living.");
    } while(false);
}

BOOST_AUTO_TEST_CASE(multiline)
{
    std::string str =
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.\n"
        "Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, \"fringilla vel\", aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo.\n"
        "\n"
        "Nullam dictum felis eu pede mollis pretium. Integer tincidunt. 'Cras' dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus.\n"
        "Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. \"Etiam\" rhoncus.\n"
        "Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit cursus nunc,\n";

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::base64_encoder(acqua::iostreams::newline::ln, 76));
        out.push(oss);
        out << str;
        boost::iostreams::close(out);

        std::istringstream iss(oss.str());
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::base64_decoder());
        in.push(iss);

        oss.str("");
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == str);
    } while(false);
}

BOOST_AUTO_TEST_SUITE_END()

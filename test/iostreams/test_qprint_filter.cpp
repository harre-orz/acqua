#include <acqua/iostreams/qprint_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(qprint_filter)


BOOST_AUTO_TEST_CASE(encoder)
{
    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::ln, 20));  // 20文字ごとに改行コード LN を挟む
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() == "I don't dream at nig=\nht, I dream all day;=\n I dream for a livin=\ng.");
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::cr, 20));  // 20文字ごとに改行コード CR を挟む
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() == "I don't dream at nig=\rht, I dream all day;=\r I dream for a livin=\rg.");
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::crln, 20));  // 20文字ごとに改行コード CRLN を挟む
        out.push(oss);
        out << "I don't dream at night, I dream all day; I dream for a living.";
        boost::iostreams::close(out);
        BOOST_TEST(oss.str() == "I don't dream at nig=\r\nht, I dream all day;=\r\n I dream for a livin=\r\ng.");
    } while(false);
}

BOOST_AUTO_TEST_CASE(decoder)
{
    do {
        // LN
        std::istringstream iss("I don't dream at nig=\nht, I dream all day;=\n I dream for a livin=\ng.");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
        in.push(iss);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == "I don't dream at night, I dream all day; I dream for a living.");
    } while(false);

    do {
        // CR
        std::istringstream iss("I don't dream at nig=\rht, I dream all day;=\r I dream for a livin=\rg.");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
        in.push(iss);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == "I don't dream at night, I dream all day; I dream for a living.");
    } while(false);

    do {
        // CRLN
        std::istringstream iss("I don't dream at nig=\r\nht, I dream all day;=\r\n I dream for a livin=\r\ng.");
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
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
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::ln, 14));
        out.push(oss);
        out << str;
        boost::iostreams::close(out);

        std::istringstream iss(oss.str());
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
        in.push(iss);
        oss.str("");
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == str);
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::cr, 47));
        out.push(oss);
        out << str;
        boost::iostreams::close(out);

        std::istringstream iss(oss.str());
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
        in.push(iss);
        oss.str("");
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == str);
    } while(false);

    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::qprint_encoder(acqua::iostreams::newline::crln, 77));
        out.push(oss);
        out << str;
        boost::iostreams::close(out);

        std::istringstream iss(oss.str());
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::qprint_decoder());
        in.push(iss);
        oss.str("");
        boost::iostreams::copy(in, oss);
        BOOST_TEST(oss.str() == str);
    } while(false);
}

BOOST_AUTO_TEST_SUITE_END()

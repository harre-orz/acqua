#include <iomanip>
#include <chrono>
#include <acqua/asio/internet_server.hpp>
#include <acqua/asio/read_until.hpp>
#include <acqua/asio/write.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/iostreams/md5_filter.hpp>
#include <acqua/iostreams/sha256_filter.hpp>
#include <acqua/iostreams/transferred_counter.hpp>
#include <boost/blank.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

enum class compress_type { none, zlib, gzip, bzip2 };
enum class checksum_type { none, md5, sha256 };

static boost::spirit::qi::symbols<char, compress_type> compress;
static boost::spirit::qi::symbols<char, checksum_type> checksum;


/*!
  概要 fcopy
  ファイルを転送する俺々プロトコル。asio と iostreams の連携の練習
  とりあえず、サーバが受け、クライアントが送信だけを行う。

  プロトコル
   Step1. ファイル情報を1行(CRLN)でクライアントからサーバへ転送する
   「PUT FILE=XXXXXX SIZE=NNN PERM=777 COMPRESS=gzip CHECK=md5\r\n」
   - FILE は base64 エンコードされたファイルパス(ディレクトリを含んでもよい)
   - SIZE は ファイルサイズ
   - PERM は UNIXにおける8進数パーミッションコード。0 のときは受け側の任意で決められる
   - COMPRESS は圧縮アルゴリズム (none, zlib, gzip, bzip2)
   - CHECKSUM は チェックサムアルゴリズム (none, md5, sha256)
   Step2. ファイルをバイナリで転送する１チャンクサイズを転送する
   -「NNN\r\n」
   - 0サイズのときは、終了。Step4 へ
   Step3. チャンクサイズ分のファイルを転送する
   - COMPRESS 指定されていると圧縮される
   - Step2 へ戻る
   Step3. チェックサム結果を1行(CRLN)で転送
   - CHECKSUM 指定されていない場合でも、改行コードは入る
   Step4. Step1 を待つ
 */
template <typename Socket>
struct fcopy
    : std::enable_shared_from_this< fcopy<Socket> >
{
    using self_type = fcopy;
    using socket_type = Socket;
    using lowest_layer_type = typename Socket::lowest_layer_type;

    explicit fcopy(boost::asio::io_service & io_service)
        : socket_(io_service) {}

    lowest_layer_type & socket() { return socket_; }

    void start()
    {
        boost::asio::async_read_until(
            socket_, recvbuf_, "\r\n",
            std::bind(&self_type::on_step1, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void disconnect(char const * location)
    {
        std::cout << "loc: " << location << " :: "
                  << std::string(boost::asio::buffer_cast<char const *>(recvbuf_.data()), recvbuf_.size()) << std::endl;
        boost::system::error_code ec;
        socket_.cancel(ec);
    }

    void on_step1(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            disconnect("on_step1");
            return;
        }

        std::cout << '<' << std::string(boost::asio::buffer_cast<char const *>(recvbuf_.data()), size);

        auto beg = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        auto end = beg + size;
        if (!parse_fileinfo(beg+4, end)) {
            disconnect("parse_fileinfo");
            return;
        }
        recvbuf_.consume(size);

        firsttime_ = std::chrono::steady_clock::now();
        total_transfer_size_ = 0;
        boost::asio::async_read_until(
            socket_, recvbuf_, "\r\n",
            std::bind(&self_type::on_step2, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    void on_step2(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            disconnect("on_step2");
            return;
        }

        chunked_size_ = static_cast<std::size_t>(std::atol(boost::asio::buffer_cast<char const *>(recvbuf_.data())));
        recvbuf_.consume(size);
        total_transfer_size_ += size;

        // チャンクサイズが、ファイルサイズを超えていたらエラーにする
        if (chunked_size_ > filesize_) {
            disconnect("chunksize too long");
            return;
        }

        if (chunked_size_ == 0) {
            boost::asio::async_read_until(
                socket_, recvbuf_, "\r\n",
                std::bind(&self_type::on_step4, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        } else {
            boost::asio::async_read_until(
                socket_, recvbuf_, std::min<std::size_t>(65536, chunked_size_),
                std::bind(&self_type::on_step3, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    void on_step3(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            std::cout << error.message() << std::endl;
            disconnect("on_step3");
            return;
        }

        out_.write(boost::asio::buffer_cast<char const *>(recvbuf_.data()), static_cast<std::streamsize>(size));
        recvbuf_.consume(size);
        chunked_size_ -= size;
        total_transfer_size_ += size;

        // デコード後のファイルサイズが、ファイルサイズを超えていたらエラーにする
        if (decompressed_size_ > filesize_) {
            disconnect("decompressed-size too long");
            return;
        }

        if (chunked_size_ == 0) {
            boost::asio::async_read_until(
                socket_, recvbuf_, "\r\n",
                std::bind(&self_type::on_step2, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        } else {
            boost::asio::async_read_until(
                socket_, recvbuf_, std::min<std::size_t>(65536, chunked_size_),
                std::bind(&self_type::on_step3, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    void on_step4(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            disconnect("on_step3");
            return;
        }

        out_.reset();
        // デコード後のファイルサイズが、ファイルサイズを超えていたらエラーにする
        if (decompressed_size_ > filesize_) {
            disconnect("decompressed-size too long");
            return;
        }

        bool verify = verify_check(boost::asio::buffer_cast<char const *>(recvbuf_.data()), size-2);
        recvbuf_.consume(size);
        total_transfer_size_ += size;
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - firsttime_).count();

        std::cout << "verify:" << (verify ? "Success" : "Failure")
                  << " (" << filesize_ << " bytes"
                  << ", " << time_ms   << " ms"
                  << ", " << (static_cast<double>(filesize_ * 8 * 60) / static_cast<double>(time_ms * 1000 * 1000)) << " Mbps"
                  << ")(" << total_transfer_size_ << " bytes"
                  << ", " << (static_cast<double>(filesize_) / static_cast<double>(total_transfer_size_)) << " rate"
                  << ", " << (static_cast<double>(total_transfer_size_ * 8 * 60) / static_cast<double>(time_ms * 1000 * 1000)) << " Mbps"
                  << ")" << std::endl;
    }

    bool verify_check(char const * s, std::size_t n)
    {
        switch(checksum_) {
            case checksum_type::md5:
                if (n == 16 && std::memcmp(s, verify_.data(), 16) == 0)
                    return true;
            case checksum_type::sha256:
                if (n == 32 && std::memcmp(s, verify_.data(), 32) == 0)
                    return true;
            default:
                return true;
        }

        // エラーの時はベリファイ結果を表示しておく
        std::cout << std::hex;
        for(auto ch : verify_)
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
        std::cout << std::dec << std::endl;
        return false;
    }

    template <typename It>
    bool parse_fileinfo(It beg, It end)
    {
        namespace qi = boost::spirit::qi;

        std::string encoded_file;
        compress_type compress_;
        int permisson;

        // boost::spirit::istream_iterator beg(is);
        // boost::spirit::istream_iterator end;
        if (!qi::parse(beg, end,
                       "FILE=" >> *(qi::char_ - ' ') >> ' ' >>
                       "SIZE=" >> qi::uint_ >> ' ' >>
                       "PERM=" >> qi::int_ >> ' ' >>
                       "COMPRESS=" >> compress >> ' ' >>
                       "CHECKSUM=" >> checksum >>
                       "\r\n", encoded_file, filesize_, permisson, compress_, checksum_))
            return false;

        // ファイル名をBASE64デコード
        filename_.clear();
        do {
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_url_decoder());
            out.push(boost::iostreams::back_inserter(filename_));
            out << encoded_file;
        } while(false);

        // 圧縮アルゴリズムを登録
        switch (compress_) {
            case compress_type::zlib:
                out_.push(boost::iostreams::zlib_decompressor());
                break;
            case compress_type::gzip:
                out_.push(boost::iostreams::gzip_decompressor());
                break;
            case compress_type::bzip2:
                out_.push(boost::iostreams::bzip2_decompressor());
                break;
            default:
                break;
        }

        // チェックサムを登録
        switch(checksum_) {
            case checksum_type::md5:
                out_.push(acqua::iostreams::md5_filter(verify_));
                break;
            case checksum_type::sha256:
                out_.push(acqua::iostreams::sha256_filter(verify_));
                break;
            default:
                break;
        }

        // デコード後のファイルサイズ取得を登録
        out_.push(acqua::iostreams::transferred_counter(decompressed_size_));

        // 保存先ファイルを登録
        // とりあえずダミー
        oss_.str("");
        out_.push(oss_);

        return true;
    }

private:
    socket_type socket_;
    boost::asio::streambuf recvbuf_;
    boost::iostreams::filtering_ostream out_;
    std::ostringstream oss_;
    checksum_type checksum_;
    std::array<unsigned char, 32> verify_;
    std::string filename_;
    std::size_t filesize_;      // ファイルサイズ
    std::size_t chunked_size_;  // チャンクサイズ
    std::size_t decompressed_size_;  // デコードされたサイズ
    std::size_t total_transfer_size_; // ネットワーク上を転送したデータサイズ
    std::chrono::steady_clock::time_point firsttime_;  // 転送を開始した時間
};


int main(int argc, char ** argv)
{
    // グローバル変数を初期化
    compress.add
        ("none", compress_type::none)
        ("zlib", compress_type::zlib)
        ("gzip", compress_type::gzip)
        ("bzip2", compress_type::bzip2);
    checksum.add
        ("none", checksum_type::none)
        ("md5", checksum_type::md5)
        ("sha256", checksum_type::sha256);

    boost::asio::io_service io_service;
    acqua::asio::internet_server< fcopy<boost::asio::ip::tcp::socket> > sv(io_service);
    sv.listen(12345);
    sv.start();
    io_service.run();
}

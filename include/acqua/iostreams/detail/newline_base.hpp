#pragma once

#include <limits>
#include <boost/iostreams/operations.hpp>

namespace acqua { namespace iostreams {

enum class newline { none, cr, ln, crln };

namespace detail {

/*!
  一定行数以上を書き込んだときに、自動的に改行を行う基底クラス.
 */
template <typename Derived>
class newline_base
{
protected:
    using base_type = newline_base;

protected:
    /*!
      コンストラクタ.
      @param nl 改行コード
      @param max 自動改行の目安となる文字数
     */
    explicit newline_base(newline nl, std::size_t max)
        : nl_(nl), max_(nl == newline::none ? std::numeric_limits<std::size_t>::max() : max) {}

    ~newline_base() = default;

    //! 改行してからの書き込み文字数を返す.
    std::size_t count() const { return cnt_; }

    //! 改行してからの書き込み文字数を n 個だけ増やす.
    void incr(std::size_t n) { cnt_ += n; }

    //! 改行してからの書き込み文字数を 0 にする.
    void clear() { cnt_ = 0; }

    //! 改行文字を sink に書き込んで、改行してからの書き込み文字数を 0 にする.
    template <typename Sink>
    bool put_ln(Sink & sink)
    {
        cnt_ = 0;
        switch(nl_) {
            case newline::cr:
                return boost::iostreams::put(sink, '\r');
            case newline::ln:
                return boost::iostreams::put(sink, '\n');
            case newline::crln:
                return boost::iostreams::put(sink, '\r')
                    && boost::iostreams::put(sink, '\n');
            case newline::none:
                return true;
        }
    }

    //! 自動改行を行わずに、文字 ch を sink に書き込む.
    //! ただし、改行してからの書き込み文字数はカウントされる.
    template <typename Sink>
    bool put_nobreak(Sink & sink, char ch)
    {
        cnt_++;
        return boost::iostreams::put(sink, ch);
    }

    //! 文字 ch を sink に書き込むが、改行の目安を超えていれば、先に改行文字を書き込む.
    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        if (cnt_ >= max_)
            if (!static_cast<Derived *>(this)->line_break(sink))
                return false;
        return put_nobreak(sink, ch);
    }

public:
    //! コンストラクタで指定した改行コードを用いて、改行文字を sink に書き込む.
    //! [オーバーライド可能] 自動改行が発生したときに、呼び出される処理
    template <typename Sink>
    bool line_break(Sink & sink) { return put_ln(sink); }

private:
    newline  nl_;
    std::size_t max_;
    std::size_t cnt_ = 0;
};

} } }

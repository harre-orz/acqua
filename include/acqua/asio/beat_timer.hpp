#pragma once

#include <mutex>
#include <functional>
#include <boost/system/error_code.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/container/flat_set.hpp>
#include <acqua/asio/timer_traits.hpp>

namespace acqua { namespace asio {

/*!
  async_run() する時刻になったら Derived::run() を実行するクラス
 */
template <typename Derived, typename AsioTimer>
class beat_timer
{
public:
    using timer_type = AsioTimer;
    using traits_type = timer_traits<AsioTimer>;
    using time_point_type = typename traits_type::time_point_type;

protected:
    explicit beat_timer(boost::asio::io_service & io_service)
        : timer_(io_service) {}

    ~beat_timer() {}

public:
    void async_run(time_point_type const & trigger_time = traits_type::now())
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        auto it = set_.emplace_hint(set_.begin(), trigger_time);
        if (it == set_.begin()) {
            timer_.expires_at(trigger_time);
            timer_.async_wait(std::bind(&beat_timer::on_wait, this, std::placeholders::_1));
        }
    }

    void cancel()
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        set_.clear();
        timer_.cancel();
    }

private:
    void on_wait(boost::system::error_code const & error)
    {
        if (!error) {
            do {
                static_cast<Derived *>(this)->run();

                std::lock_guard<decltype(mutex_)> lock(mutex_);
                auto it = set_.begin();
                time_point_type time = *it;
                it = set_.erase(it);
                if (it != set_.end()) {
                    if (*it <= time)
                        continue;
                    timer_.expires_at(*it);
                    timer_.async_wait(std::bind(&beat_timer::on_wait, this, std::placeholders::_1));
                }
            } while(0);
        }
    }

private:
    timer_type timer_;
    boost::container::flat_multiset<time_point_type> set_;
    std::mutex mutex_;
};

} }

#pragma once

#include <memory>
#include <mutex>
#include <boost/system/error_code.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/container/flat_map.hpp>
#include <acqua/asio/timer_traits.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_fused.hpp>

namespace acqua { namespace asio {

/*!
  async_run(Args...) する時刻になったら Derived::on_run(Args...) を実行するクラス
 */
template <typename Derived, typename Timer = boost::asio::deadline_timer>
class beat_timer
{
private:
    struct placeholder
    {
        virtual ~placeholder() {}
        virtual void on_run(beat_timer * this_, boost::system::error_code const & error) = 0;
    };


    template <typename... Args>
    struct holder
        : placeholder
    {
        explicit holder(Args&&... args)
            : args_(std::forward<Args>(args)...) {}

        virtual void on_run(beat_timer * this_, boost::system::error_code const & error) override
        {
            boost::fusion::make_fused(&beat_timer::on_run_impl<Args...>)(
                boost::fusion::push_front(boost::fusion::push_front(args_, std::cref(error)), this_));
        }

        boost::fusion::vector<Args...> args_;
    };

protected:
    ~beat_timer() = default;

public:
    using base_type = beat_timer;
    using timer_type = Timer;
    using traits_type = timer_traits<Timer>;
    using time_point  = typename traits_type::time_point;
    using duration = typename traits_type::duration;

    explicit beat_timer(boost::asio::io_service & io_service)
        : timer_(io_service) {}

    template <typename... Args>
    void async_wait(time_point const & time, Args&&... args)
    {
        std::unique_ptr<placeholder> ptr(new holder<Args...>(std::forward<Args>(args)...));

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = map_.emplace_hint(map_.begin(), time, std::move(ptr));
        if (it == map_.begin() && !cancel_request_) {  // キャンセルリクエスト中でも追加はできないと、コールバックが呼ばれなくなる
            timer_.expires_at(it->first);
            timer_.async_wait(std::bind(&beat_timer::on_wait, this, std::placeholders::_1));
        }
    }

    template <typename... Args>
    void async_wait(duration const & time, Args&&... args)
    {
        async_wait(traits_type::now() + time, std::forward<Args>(args)...);
    }

    void cancel(boost::system::error_code & ec)
    {
        timer_.cancel(ec);
        cancel_request_ = true;
    }

    void cancel()
    {
        timer_.cancel();
        cancel_request_ = true;
    }

private:
    void on_wait(boost::system::error_code const & error)
    {
        if (error) {
            if (cancel_request_) {
                std::lock_guard<decltype(mutex_)> lock(mutex_);
                for(auto it = map_.begin(); it != map_.end();) {
                    auto ptr = std::move(it->second);
                    map_.erase(it);
                    mutex_.unlock();
                    ptr->on_run(this, error);
                    mutex_.lock();
                    it = map_.begin();
                }
                cancel_request_ = false;
            }
            return;
        }

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = map_.begin();
        if (it == map_.end())
            return;
        auto ptr = std::move(it->second);
        map_.erase(it);
        mutex_.unlock();
        ptr->on_run(this, error);
        mutex_.lock();
        it = map_.begin();
        if (it == map_.end())
            return;
        timer_.expires_at(it->first);
        timer_.async_wait(std::bind(&beat_timer::on_wait, this, std::placeholders::_1));
    }

    template <typename... Args>
    static void on_run_impl(beat_timer * this_, boost::system::error_code const & error, Args&&... args)
    {
        static_cast<Derived *>(this_)->on_run(error, std::forward<Args>(args)...);
    }

private:
    timer_type timer_;
    boost::container::flat_multimap< time_point, std::unique_ptr<placeholder> > map_;
    std::mutex mutex_;
    bool cancel_request_ = false;
};

} }

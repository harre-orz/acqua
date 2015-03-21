#pragma once

extern "C" {
#include <sys/inotify.h>
}

#include <memory>
#include <array>
#include <mutex>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <acqua/container/pointer_container_iterator.hpp>
#include <acqua/mref.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace asio {

class inotify_service
    : public boost::asio::io_service::service
{
public:
    class event
    {
    public:
        static const int none = 0;
        static const int access = IN_ACCESS;
        static const int attribute = IN_ATTRIB;
        static const int close_write = IN_CLOSE_WRITE;
        static const int close_nonwrite = IN_CLOSE_WRITE;
        static const int close = IN_CLOSE;
        static const int modify = IN_MODIFY;
        static const int remove = IN_DELETE_SELF;
        static const int open = IN_OPEN;
        static const int move = IN_MOVE_SELF;
        static const int moved_from = IN_MOVED_FROM;
        static const int moved_to = IN_MOVED_TO;
        static const int create_path = IN_CREATE;
        static const int remove_path = IN_DELETE;
        static const int ignored = IN_IGNORED;
        static const int all_events = IN_ALL_EVENTS;

        explicit event(int mask, std::string const & name, std::string const & in_name)
            : mask_(mask), name_(name), in_name_(in_name) {}

        int type() const noexcept
        {
            return mask_;
        }

        bool is_directory() const noexcept
        {
            return mask_ & IN_ISDIR;
        }

        std::string const & name() const noexcept
        {
            return name_;
        }

        std::string const & in_name() const noexcept
        {
            return in_name_;
        }

    private:
        int mask_;
        std::string name_;
        std::string in_name_;
    };

    using iterator = acqua::container::unique_container_iterator< std::vector<event> >;

private:
    class implementation
        : boost::noncopyable
    {
        friend class inotify_service;

        std::unique_ptr<boost::asio::posix::stream_descriptor> input_;
        std::array<char, 4096> buffer_;
        std::array<char, 4096>::iterator beg_, end_;
        std::unordered_map<int, std::string> files_;
        std::unordered_map<int, event> cookies_;
        std::mutex mutex_;
    };

public:
    using implementation_type = implementation;

    static boost::asio::io_service::id id;

    explicit inotify_service(boost::asio::io_service & io_service)
        : boost::asio::io_service::service(io_service)
    {
    }

    void shutdown_service()
    {
    }

    void construct(implementation & impl)
    {
        int fd = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (fd < 0) {
            acqua::exception::throw_error(
                boost::system::error_code(errno, boost::system::get_generic_category()),
                "inotify_init1"
            );
        }

        impl.input_.reset(new boost::asio::posix::stream_descriptor(get_io_service(), fd));
        impl.beg_ = impl.end_ = impl.buffer_.begin();
    }

    void destroy(implementation & impl)
    {
        impl.files_.clear();
        impl.input_.reset();
    }

    void cancel(implementation & impl)
    {
        impl.beg_ = impl.end_;
        impl.input_->cancel();
    }

    void add(implementation & impl, std::string const & filename, int flags, boost::system::error_code & ec)
    {
        int wd = ::inotify_add_watch(impl.input_->native_handle(), filename.c_str(), flags | IN_MASK_ADD);
        if (wd >= 0) {
            std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
            impl.files_.emplace(wd, filename);
        } else {
            ec.assign(errno, boost::system::get_generic_category());
        }
    }

    void remove(implementation & impl, std::string const & filename, boost::system::error_code & ec)
    {
        std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
        for(auto it = impl.files_.begin(); it != impl.files_.end(); ++it) {
            if (it->second == filename) {
                if (::inotify_rm_watch(impl.input_->native_handle(), it->first) == 0) {
                    impl.files_.erase(it);
                    return;
                }
            } else {
                ec.assign(errno, boost::system::get_generic_category());
            }
        }

        ec.assign(ENXIO, boost::system::get_system_category());
    }

    template <typename Handler>
    void async_notify(implementation & impl, Handler handler)
    {
        if ((std::size_t)(impl.end_ - impl.beg_) < sizeof(::inotify_event)) {
            impl.input_->async_read_some(
                boost::asio::buffer(impl.buffer_),
                boost::bind(
                    &inotify_service::handler_watch<Handler>,
                    this,
                    std::ref(impl),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    handler
                )
            );
        } else {
            std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
            do_notify(impl, boost::system::error_code(), handler);
        }
    }

private:
    template <typename Handler>
    void async_notify_nolock(implementation & impl, Handler handler)
    {
        if ((std::size_t)(impl.end_ - impl.beg_) < sizeof(::inotify_event)) {
            impl.input_->async_read_some(
                boost::asio::buffer(impl.buffer_),
                boost::bind(
                    &inotify_service::handler_watch<Handler>,
                    this,
                    std::ref(impl),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred,
                    handler
                )
            );
        } else {
            do_notify(impl, boost::system::error_code(), handler);
        }
    }

    template <typename Handler>
    void handler_watch(implementation & impl, boost::system::error_code const & error, std::size_t size, Handler handler)
    {
        if (error) {
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            get_io_service().post(boost::asio::detail::bind_handler(handler, error, acqua::mref(iterator())));
        } else if (size < sizeof(::inotify_event)) {
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            get_io_service().post(boost::asio::detail::bind_handler(handler, boost::system::error_code(EIO, boost::system::get_generic_category()), acqua::mref(iterator())));
        } else {
            impl.beg_ = impl.buffer_.begin();
            impl.end_ = impl.buffer_.begin() + size;

            std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
            do_notify(impl, error, handler);
        }
    }

    template <typename Handler>
    void do_notify(implementation & impl, boost::system::error_code const & error, Handler handler)
    {
        ::inotify_event *iev = reinterpret_cast<::inotify_event *>(impl.beg_);
        impl.beg_ += sizeof(*iev) + iev->len;

        std::unique_ptr< std::vector<event> > ev(new std::vector<event>);
        if (iev->mask & 0x0fff) {
            ev->emplace_back(iev->mask, impl.files_[iev->wd], iev->name);
            if (iev->mask & IN_MOVE && iev->cookie != 0) {
                decltype(impl.cookies_.end()) it;
                bool is_new;
                std::tie(it, is_new) = impl.cookies_.emplace(iev->cookie, ev->front());
                if (is_new) {
                    if (is_bytes_readable(impl, 1)) {
                        async_notify_nolock(impl, handler);
                        return;
                    }
                } else {
                    ev->emplace_back(it->second);
                }

                impl.cookies_.erase(it);
            }

            get_io_service().post(boost::asio::detail::bind_handler(handler, error, acqua::mref(iterator(std::move(ev)))));
        } else if(iev->mask & IN_IGNORED) {
            auto it = impl.files_.find(iev->wd);
            if (it != impl.files_.end()) {
                ev->emplace_back(iev->mask, it->second, iev->name);
                impl.files_.erase(it);
                get_io_service().post(boost::asio::detail::bind_handler(handler, error, acqua::mref(iterator(std::move(ev)))));
            } else {
                async_notify_nolock(impl, handler);
            }
        }

        if (!impl.cookies_.empty() && !is_bytes_readable(impl, 1)) {
            // moved_cookies に残っているものは、バッファに戻す
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            for(auto it = impl.cookies_.begin(); it != impl.cookies_.end();) {
                for(auto const & e : impl.files_) {
                    if (e.second == it->second.name()) {
                        iev = reinterpret_cast<struct ::inotify_event *>(impl.end_);
                        iev->mask = IN_MOVED_FROM;
                        iev->wd = e.first;
                        iev->cookie = 0;
                        iev->len = it->second.in_name().size();
                        std::copy_n(it->second.in_name().begin(), iev->len, iev->name);
                        iev->name[iev->len++] = '\0';
                        impl.end_ += sizeof(*iev) + iev->len;
                    }
                }

                it = impl.cookies_.erase(it);
            }
        }
    }

    bool is_bytes_readable(implementation & impl, int timeout) const
    {
        if (impl.beg_ != impl.end_)
            return true;

        if (timeout > 0) {
            struct ::pollfd fd = {
                .fd = impl.input_->native_handle(),
                .events = POLLIN,
                .revents = 0
            };

            do {
                switch(poll(&fd, 1, timeout)) {
                    case -1: if (errno == EINTR) continue;
                    case  0: return false;
                    default: return true;
                }
            } while(0);
        }

        return false;
    }

private:
    implementation_type implementation_;
};

extern boost::asio::io_service::id inotify_service::id;

} }


#include <acqua/asio/basic_notify.hpp>

namespace acqua { namespace asio {

typedef basic_notify<inotify_service> inotify;

} }

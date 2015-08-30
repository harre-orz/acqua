/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <sys/inotify.h>
}

#include <algorithm>
#include <mutex>
#include <array>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <acqua/container/unique_container_iterator.hpp>

namespace acqua { namespace asio { namespace detail {

class inotify_service
    : public boost::asio::detail::service_base<inotify_service>
{
    using descriptor_type = boost::asio::posix::stream_descriptor;

public:
    static boost::asio::io_service::id id;

    using native_handle_type = typename descriptor_type::native_handle_type;
    using event_flags = int;
    static const event_flags all_events = IN_ALL_EVENTS;

    class event_type
    {
        friend inotify_service;

    public:
        enum event_code {
            none = 0,
            access = IN_ACCESS,
            attribute = IN_ATTRIB,
            close_write = IN_CLOSE_WRITE,
            close_nowrite = IN_CLOSE_WRITE,
            close = IN_CLOSE,
            modify = IN_MODIFY,
            remove = IN_DELETE_SELF,
            open = IN_OPEN,
            move = IN_MOVE_SELF,
            moved_from = IN_MOVED_FROM,
            moved_to = IN_MOVED_TO,
            create_path = IN_CREATE,
            remove_path = IN_DELETE,
            ignored = IN_IGNORED,
        };

    private:
        event_type(int mask, std::string const & name, std::string const & in_name)
            : mask_(mask), name_(name), in_name_(in_name)
        {
        }

    public:
        int type() const
        {
            return mask_;
        }

        bool is(enum event_code code) const
        {
            return mask_ & (int)code;
        }

        bool is_directory() const
        {
            return mask_ & IN_ISDIR;
        }

        std::string const & name() const
        {
            return name_;
        }

        std::string const & in_name() const
        {
            return in_name_;
        }

    private:
        int mask_;
        std::string name_;
        std::string in_name_;
    };

    class implementation_type
        : private boost::noncopyable
    {
        friend inotify_service;
        using buffer_type = std::array<char, 4096>;
        using buffer_iter = typename buffer_type::iterator;

        boost::optional<descriptor_type> fd_;
        buffer_type buffer_;
        buffer_iter beg_, end_;
        std::unordered_map<native_handle_type, std::string> files_;
        std::unordered_map<native_handle_type, event_type> cookies_;
        std::mutex mutex_;
    };

    using iterator_type = acqua::container::unique_container_iterator<std::vector<event_type> const>;

public:
    explicit inotify_service(boost::asio::io_service & io_service)
        : boost::asio::detail::service_base<inotify_service>(io_service)
    {
    }

    void shutdown_service()
    {
    }

    void construct(implementation_type & impl)
    {
        impl.fd_ = descriptor_type(this->get_io_service());
        impl.beg_ = impl.end_ = impl.buffer_.begin();
    }

    void destroy(implementation_type & impl)
    {
        impl.fd_ = boost::none;
    }

    void move_construct(implementation_type & impl, implementation_type & other_impl)
    {
        impl.fd_ = std::move(other_impl.fd_);
        std::copy(other_impl.beg_, other_impl.end_, impl.buffer_.data());
        impl.beg_ = other_impl.beg_;
        impl.end_ = other_impl.end_;
        impl.files_ = std::move(other_impl.files_);
        impl.cookies_ = std::move(other_impl.cookies_);
    }

    void move_assign(implementation_type & impl, inotify_service &, implementation_type & other_impl)
    {
        impl.fd_ = std::move(other_impl.fd_);
        std::copy(other_impl.beg_, other_impl.end_, impl.buffer_.data());
        impl.beg_ = other_impl.beg_;
        impl.end_ = other_impl.end_;
        impl.files_ = std::move(other_impl.files_);
        impl.cookies_ = std::move(other_impl.cookies_);
    }

    bool is_open(implementation_type & impl)
    {
        return (bool)impl.fd_;
    }

    boost::system::error_code assign(implementation_type & impl, native_handle_type const & handle, boost::system::error_code & ec)
    {
        return impl.fd_->assign(handle, ec);
    }

    boost::system::error_code open(implementation_type & impl, boost::system::error_code & ec)
    {
        if (impl.fd_->is_open()) {
            errno = EISCONN;
        } else {
            int fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
            if (fd >= 0) {
                return impl.fd_->assign(fd, ec);
            }
        }
        ec.assign(errno, boost::system::generic_category());
        return ec;
    }

    boost::system::error_code close(implementation_type & impl, boost::system::error_code & ec)
    {
        return impl.fd_->close(ec);
    }

    native_handle_type native_handle(implementation_type & impl) const
    {
        return impl.fd_->native_handle();
    }

    boost::system::error_code cancel(implementation_type & impl, boost::system::error_code & ec)
    {
        return impl.fd_->cancel(ec);
    }

    iterator_type wait(implementation_type & impl, boost::system::error_code & ec)
    {
        size_t size;
        if ((std::size_t)(impl.end_ - impl.beg_) < sizeof(::inotify_event)) {
            size = impl.fd_->read_some(boost::asio::buffer(impl.buffer_), ec);
            if (ec) return iterator_type();

            impl.beg_ = impl.buffer_.begin();
            impl.end_ = impl.beg_ + size;
        }

        std::unique_ptr< std::vector<event_type> > res(new std::vector<event_type>);
        do_notify(impl, *res);
        return iterator_type(res->begin(), std::move(res));;
    }

    template <typename Handler>
    void async_wait(implementation_type & impl, Handler handler)
    {
        if ((std::size_t)(impl.end_ - impl.beg_) < sizeof(::inotify_event)) {
            impl.fd_->async_read_some(
                boost::asio::buffer(impl.buffer_),
                std::bind(&inotify_service::watch_handle<Handler>,
                          std::ref(impl),
                          std::placeholders::_1,
                          std::placeholders::_2,
                          handler)
            );
        } else {
            std::unique_ptr< std::vector<event_type> > res(new std::vector<event_type>);
            do_notify(impl, *res);
            handler(boost::system::error_code(), iterator_type(res->begin(), std::move(res)));
        }
    }

    bool exists(implementation_type & impl, std::string const & path) const
    {
        std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
        for(auto it = impl.files_.begin(); it != impl.files_.end(); ++it)
            if (it->second == path)
                return true;
        return false;
    }

    boost::system::error_code add(implementation_type & impl, std::string const & path, event_flags flags, boost::system::error_code & ec)
    {
        int wd = ::inotify_add_watch(impl.fd_->native_handle(), path.c_str(), flags | IN_MASK_ADD);
        if (wd >= 0) {
            std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
            impl.files_.emplace(wd, path);
        } else {
            ec.assign(errno, boost::system::generic_category());
        }
        return ec;
    }

    boost::system::error_code remove(implementation_type & impl, std::string const & path, boost::system::error_code & ec)
    {
        std::lock_guard<decltype(impl.mutex_)> lock(impl.mutex_);
        for(auto it = impl.files_.begin(); it != impl.files_.end(); ++it) {
            if (it->second == path) {
                if (::inotify_rm_watch(impl.fd_->native_handle(), it->first) == 0) {
                    impl.files_.erase(it);
                } else {
                    ec.assign(errno, boost::system::get_generic_category());
                }
                return ec;
            }
        }

        ec.assign(ENXIO, boost::system::get_generic_category());
        return ec;
    }

private:
    template <typename Handler>
    static void watch_handle(implementation_type & impl, boost::system::error_code const & error, std::size_t size, Handler handler)
    {
        if (error) {
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            handler(error, iterator_type());
            return;
        } else if (size < sizeof(::inotify_event)) {
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            handler(boost::system::error_code(EIO, boost::system::get_generic_category()), iterator_type());
        } else {
            impl.beg_ = impl.buffer_.begin();
            impl.end_ = impl.buffer_.begin() + size;

            std::unique_ptr< std::vector<event_type> > res(new std::vector<event_type>);
            do_notify(impl, *res);
            handler(boost::system::error_code(), iterator_type(res->begin(), std::move(res)));
        }
    }

    static void do_notify(implementation_type & impl, std::vector<event_type> & vec)
    {
        ::inotify_event * iev = reinterpret_cast<::inotify_event *>(impl.beg_);
        impl.beg_ += sizeof(*iev) + iev->len;

        auto it = impl.files_.find(iev->wd);
        if (it == impl.files_.end()) {
            return;
        }

        if (iev->mask & (0x0fff | IN_IGNORED)) {
            vec.emplace_back(event_type(iev->mask, it->second, iev->name));
            if (iev->mask & IN_MOVE && iev->cookie != 0) {
                decltype(impl.cookies_.begin()) it;
                bool is_new;
                std::tie(it, is_new) = impl.cookies_.emplace(iev->cookie, vec.front());
                if (is_new) {
                    if ((impl.end_ - impl.beg_) >= (std::ptrdiff_t)sizeof(*iev)) {
                        do_notify(impl, vec);
                        return;
                    }
                } else {
                    vec.emplace_back(it->second);
                }
                impl.cookies_.erase(it);
            }

            if (iev->mask & IN_IGNORED) {
                impl.files_.erase(it);
            }
        }

        if (!impl.cookies_.empty() && (impl.end_ - impl.beg_) < (std::ptrdiff_t)sizeof(*iev)) {
            // moved_cookies に残っているデータを、バッファに戻しておき、次の wait 時に取得できるようにする
            impl.beg_ = impl.end_ = impl.buffer_.begin();
            for(auto it = impl.cookies_.begin(); it != impl.cookies_.end(); it = impl.cookies_.erase(it)) {
                for(auto const & elem : impl.files_) {
                    if (elem.second == it->second.name()) {
                        iev = reinterpret_cast<struct ::inotify_event *>(impl.end_);
                        iev->mask = it->second.mask_;
                        iev->wd = elem.first;
                        iev->cookie = 0;
                        iev->len = it->second.in_name().size();
                        std::copy_n(it->second.in_name().begin(), iev->len, iev->name);
                        iev->name[iev->len++] = '\0';
                        impl.end_ += sizeof(*iev) + iev->len;
                    }
                }
            }
        }
    }
};


//     template <typename Handler>
//     void do_notify(implementation & impl, boost::system::error_code const & error, Handler handler)
//     {
//         ::inotify_event *iev = reinterpret_cast<::inotify_event *>(impl.beg_);
//         impl.beg_ += sizeof(*iev) + iev->len;

//         std::unique_ptr< std::vector<event> > ev(new std::vector<event>);
//         if (iev->mask & 0x0fff) {
//             ev->emplace_back(iev->mask, impl.files_[iev->wd], iev->name);
//             if (iev->mask & IN_MOVE && iev->cookie != 0) {
//                 decltype(impl.cookies_.end()) it;
//                 bool is_new;
//                 std::tie(it, is_new) = impl.cookies_.emplace(iev->cookie, ev->front());
//                 if (is_new) {
//                     if (is_bytes_readable(impl, 1)) {
//                         async_notify_nolock(impl, handler);
//                         return;
//                     }
//                 } else {
//                     ev->emplace_back(it->second);
//                 }

//                 impl.cookies_.erase(it);
//             }

//             get_io_service().post(boost::asio::detail::bind_handler(handler, error, acqua::mref(iterator(std::move(ev)))));
//         } else if(iev->mask & IN_IGNORED) {
//             auto it = impl.files_.find(iev->wd);
//             if (it != impl.files_.end()) {
//                 ev->emplace_back(iev->mask, it->second, iev->name);
//                 impl.files_.erase(it);
//                 get_io_service().post(boost::asio::detail::bind_handler(handler, error, acqua::mref(iterator(std::move(ev)))));
//             } else {
//                 async_notify_nolock(impl, handler);
//             }
//         }

//         if (!impl.cookies_.empty() && !is_bytes_readable(impl, 1)) {
//             // moved_cookies に残っているものは、バッファに戻す
//             impl.beg_ = impl.end_ = impl.buffer_.begin();
//             for(auto it = impl.cookies_.begin(); it != impl.cookies_.end();) {
//                 for(auto const & e : impl.files_) {
//                     if (e.second == it->second.name()) {
//                         iev = reinterpret_cast<struct ::inotify_event *>(impl.end_);
//                         iev->mask = IN_MOVED_FROM;
//                         iev->wd = e.first;
//                         iev->cookie = 0;
//                         iev->len = it->second.in_name().size();
//                         std::copy_n(it->second.in_name().begin(), iev->len, iev->name);
//                         iev->name[iev->len++] = '\0';
//                         impl.end_ += sizeof(*iev) + iev->len;
//                     }
//                 }

//                 it = impl.cookies_.erase(it);
//             }
//         }
//     }

//     bool is_bytes_readable(implementation & impl, int timeout) const
//     {
//         if (impl.beg_ != impl.end_)
//             return true;

//         if (timeout > 0) {
//             struct ::pollfd fd = {
//                 .fd = impl.input_->native_handle(),
//                 .events = POLLIN,
//                 .revents = 0
//             };

//             do {
//                 switch(poll(&fd, 1, timeout)) {
//                     case -1: if (errno == EINTR) continue;
//                     case  0: return false;
//                     default: return true;
//                 }
//             } while(0);
//         }

//         return false;
//     }

// private:
//     implementation_type implementation_;
// };

boost::asio::io_service::id inotify_service::id;

} } }

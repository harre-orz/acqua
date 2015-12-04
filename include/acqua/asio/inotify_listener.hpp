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

#include <array>
#include <mutex>
#include <boost/bind.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace acqua { namespace asio {

template <typename Derived>
class inotify_listener
    : private boost::noncopyable
{
    using descriptor_type = boost::asio::posix::stream_descriptor;

protected:
    using base_type = inotify_listener<Derived>;

public:
    explicit inotify_listener(boost::asio::io_service & io_service)
        : fd_(io_service)
    {
    }

    void open(boost::system::error_code & ec)
    {
        if (fd_.is_open()) {
            ec = make_error_code(boost::asio::error::already_open);
        } else {
            int fd = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
            if (fd >= 0)
                fd_.assign(fd, ec);
            else
                ec.assign(errno, boost::system::generic_category());
        }
    }

    void start(boost::system::error_code & ec)
    {
        if (!fd_.is_open()) {
            open(ec);
            if (ec) return;
        }

        async_read();
    }

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec);

    }

    void close(boost::system::error_code & ec)
    {
        fd_.close(ec);
    }

    void close()
    {
        boost::system::error_code ec;
        close(ec);
        boost::asio::detail::throw_error(ec);
    }

    void add(std::string const & path, int flags, boost::system::error_code & ec)
    {
        int wd = ::inotify_add_watch(fd_.native_handle(), path.c_str(), static_cast<std::uint32_t>(flags) | IN_MASK_ADD);
        if (wd >= 0) {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            files_.emplace(wd, path);
        } else {
            ec.assign(errno, boost::system::generic_category());
        }
    }

    void add(std::string const & path)
    {
        boost::system::error_code ec;
        add(path, IN_ALL_EVENTS, ec);
        boost::asio::detail::throw_error(ec);
    }

    void remove(std::string const & path, boost::system::error_code & ec)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto it = files_.begin(); it != files_.end(); ++it) {
            if (it->second == path) {
                if (::inotify_rm_watch(fd_.native_handle(), it->first) == 0)
                    files_.erase(it);
                else
                    ec.assign(errno, boost::system::generic_category());
                return;
            }
        }
    }

    void remove(std::string const & path)
    {
        boost::system::error_code ec;
        remove(path, ec);
        boost::asio::detail::throw_error(ec);
    }

private:
    void async_read()
    {
        fd_.async_read_some(boost::asio::buffer(buffer_), boost::bind(&inotify_listener::on_read, this, _1, _2));
    }

    void on_read(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            static_cast<Derived *>(this)->on_error(error);
            return;
        }

        char const * it = buffer_.data();
        char const * end = it + size;
        while(it < end) {
            auto * iev = reinterpret_cast<::inotify_event const *>(it);
            do_notify(iev);
            it += sizeof(*iev) + iev->len;
        }

        async_read();
    }

    void do_notify(inotify_event const * iev)
    {
        std::string name;
        do {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            auto it = files_.find(iev->wd);
            if (it == files_.end())
                return;
            name = it->second;
            if (iev->mask & IN_IGNORED)
                files_.erase(it);
        } while(false);

        std::string(iev->name, std::strlen(iev->name));
        static_cast<Derived *>(this)->on_event(name);
        if (iev->mask & IN_ACCESS)
            static_cast<Derived *>(this)->on_access(name);
        if (iev->mask & IN_ATTRIB)
            static_cast<Derived *>(this)->on_attribute(name);
        if (iev->mask & IN_CLOSE_WRITE)
            static_cast<Derived *>(this)->on_close_write(name);
        if (iev->mask & IN_CLOSE_NOWRITE)
            static_cast<Derived *>(this)->on_close_nowrite(name);
        if (iev->mask & IN_CLOSE)
            static_cast<Derived *>(this)->on_close(name);
        if (iev->mask & IN_MODIFY)
            static_cast<Derived *>(this)->on_modify(name);
        if (iev->mask & IN_OPEN)
            static_cast<Derived *>(this)->on_open(name);
        if (iev->mask & IN_MOVE_SELF)
            static_cast<Derived *>(this)->on_move(name);
        if (iev->mask * IN_DELETE_SELF)
            static_cast<Derived *>(this)->on_delete(name);
        if (iev->mask & IN_MOVED_FROM)
            static_cast<Derived *>(this)->on_moved_from(name, in_name);
        if (iev->mask & IN_MOVED_TO)
            static_cast<Derived *>(this)->on_moved_to(name, in_name);
        if (iev->mask & IN_CREATE)
            static_cast<Derived *>(this)->on_create_path(name, in_name);
        if (iev->mask & IN_DELETE)
            static_cast<Derived *>(this)->on_remove_path(name, in_name);
        if (iev->mask & IN_IGNORED)
            static_cast<Derived *>(this)->on_dispose(name);
    }

private:
    void on_event(std::string const &) {}
    void on_access(std::string const &) {}
    void on_attribute(std::string const &) {}
    void on_close_write(std::string const &) {}
    void on_close_nowrite(std::string const &) {}
    void on_close(std::string const &) {}
    void on_modify(std::string const &) {}
    void on_open(std::string const &) {}
    void on_move(std::string const &) {}
    void on_delete(std::string const &) {}
    void on_moved_from(std::string const &, std::string const &) {}
    void on_moved_to(std::string const &, std::string const &) {}
    void on_create_path(std::string const &, std::string const &) {}
    void on_remove_path(std::string const &, std::string const &) {}
    void on_disposed(std::string const &) {}
    void on_error(boost::system::error_code const &) {}

private:
    descriptor_type fd_;
    std::array<char, 4096> buffer_;
    boost::container::flat_map<int, std::string> files_;
    std::mutex mutex_;
};

} }

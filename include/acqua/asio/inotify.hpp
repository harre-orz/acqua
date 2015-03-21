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


#include <acqua/asio/basic_notify.hpp>
#include <acqua/asio/detail/inotify_service.hpp>

namespace acqua { namespace asio {

typedef basic_notify<detail::inotify_service> inotify;

} }

#pragma once

#include <acqua/asio/basic_pseudo_terminal.hpp>
#include <acqua/asio/pseudo_terminal_service.hpp>

namespace acqua { namespace asio {

using pseudo_terminal_master = basic_pseudo_terminal_master<pseudo_terminal_service>;
using pseudo_terminal_slave = basic_pseudo_terminal_slave<pseudo_terminal_service>;

} }

//
// write.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/write.hpp"

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <cstring>
#include <vector>
#include "asio/io_service.hpp"
#include "asio/placeholders.hpp"
#include "unit_test.hpp"

using namespace std; // For memcmp, memcpy and memset.

class test_stream
  : private boost::noncopyable
{
public:
  typedef asio::io_service io_service_type;

  test_stream(asio::io_service& io_service)
    : io_service_(io_service),
      length_(max_length),
      position_(0),
      next_write_length_(max_length)
  {
    memset(data_, 0, max_length);
  }

  io_service_type& get_io_service()
  {
    return io_service_;
  }

  void reset(size_t length = max_length)
  {
    BOOST_CHECK(length <= max_length);

    memset(data_, 0, max_length);
    length_ = length;
    position_ = 0;
    next_write_length_ = length;
  }

  void next_write_length(size_t length)
  {
    next_write_length_ = length;
  }

  template <typename Const_Buffers>
  bool check_buffers(const Const_Buffers& buffers, size_t length)
  {
    if (length != position_)
      return false;

    typename Const_Buffers::const_iterator iter = buffers.begin();
    typename Const_Buffers::const_iterator end = buffers.end();
    size_t checked_length = 0;
    for (; iter != end && checked_length < length; ++iter)
    {
      size_t buffer_length = asio::buffer_size(*iter);
      if (buffer_length > length - checked_length)
        buffer_length = length - checked_length;
      if (memcmp(data_ + checked_length,
            asio::buffer_cast<const void*>(*iter), buffer_length) != 0)
        return false;
      checked_length += buffer_length;
    }

    return true;
  }

  template <typename Const_Buffers>
  size_t write_some(const Const_Buffers& buffers)
  {
    size_t total_length = 0;

    typename Const_Buffers::const_iterator iter = buffers.begin();
    typename Const_Buffers::const_iterator end = buffers.end();
    for (; iter != end && total_length < next_write_length_; ++iter)
    {
      size_t length = asio::buffer_size(*iter);
      if (length > length_ - position_)
        length = length_ - position_;

      if (length > next_write_length_ - total_length)
        length = next_write_length_ - total_length;

      memcpy(data_ + position_,
          asio::buffer_cast<const void*>(*iter), length);
      position_ += length;
      total_length += length;
    }

    return total_length;
  }

  template <typename Const_Buffers>
  size_t write_some(const Const_Buffers& buffers, asio::error_code& ec)
  {
    ec = asio::error_code();
    return write_some(buffers);
  }

  template <typename Const_Buffers, typename Handler>
  void async_write_some(const Const_Buffers& buffers, Handler handler)
  {
    size_t bytes_transferred = write_some(buffers);
    io_service_.post(asio::detail::bind_handler(
          handler, asio::error_code(), bytes_transferred));
  }

private:
  io_service_type& io_service_;
  enum { max_length = 8192 };
  char data_[max_length];
  size_t length_;
  size_t position_;
  size_t next_write_length_;
};

static const char write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char mutable_write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_2_arg_zero_buffers_write()
{
  asio::io_service ios;
  test_stream s(ios);
  std::vector<asio::const_buffer> buffers;

  size_t bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == 0);
}

void test_2_arg_const_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::const_buffers_1 buffers
    = asio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_2_arg_mutable_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::mutable_buffers_1 buffers
    = asio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_2_arg_multi_buffers_write()
{
  asio::io_service ios;
  test_stream s(ios);
  boost::array<asio::const_buffer, 2> buffers = { {
    asio::buffer(write_data, 32),
    asio::buffer(write_data) + 32 } };

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

bool old_style_transfer_all(const asio::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec;
}

size_t short_transfer(const asio::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec ? 0 : 3;
}

void test_3_arg_const_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::const_buffers_1 buffers
    = asio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::mutable_buffers_1 buffers
    = asio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_3_arg_multi_buffers_write()
{
  asio::io_service ios;
  test_stream s(ios);
  boost::array<asio::const_buffer, 2> buffers = { {
    asio::buffer(write_data, 32),
    asio::buffer(write_data) + 32 } };

  s.reset();
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all());
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10));
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42));
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, old_style_transfer_all);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = asio::write(s, buffers, short_transfer);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_const_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::const_buffers_1 buffers
    = asio::buffer(write_data, sizeof(write_data));

  s.reset();
  asio::error_code error;
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);
}

void test_4_arg_mutable_buffers_1_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::mutable_buffers_1 buffers
    = asio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  asio::error_code error;
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(mutable_write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  BOOST_CHECK(!error);
}

void test_4_arg_multi_buffers_write()
{
  asio::io_service ios;
  test_stream s(ios);
  boost::array<asio::const_buffer, 2> buffers = { {
    asio::buffer(write_data, 32),
    asio::buffer(write_data) + 32 } };

  s.reset();
  asio::error_code error;
  size_t bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_all(), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 1);
  BOOST_CHECK(s.check_buffers(buffers, 1));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(1), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(10), error);
  BOOST_CHECK(bytes_transferred == 10);
  BOOST_CHECK(s.check_buffers(buffers, 10));
  BOOST_CHECK(!error);

  s.reset();
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 42);
  BOOST_CHECK(s.check_buffers(buffers, 42));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      asio::transfer_at_least(42), error);
  BOOST_CHECK(bytes_transferred == 50);
  BOOST_CHECK(s.check_buffers(buffers, 50));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers,
      old_style_transfer_all, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = asio::error_code();
  bytes_transferred = asio::write(s, buffers, short_transfer, error);
  BOOST_CHECK(bytes_transferred == sizeof(write_data));
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  BOOST_CHECK(!error);
}

void async_write_handler(const asio::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  BOOST_CHECK(!e);
  BOOST_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_3_arg_const_buffers_1_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::const_buffers_1 buffers
    = asio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffers_1_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::mutable_buffers_1 buffers
    = asio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_3_arg_multi_buffers_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  boost::array<asio::const_buffer, 2> buffers = { {
    asio::buffer(write_data, 32),
    asio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_const_buffers_1_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::const_buffers_1 buffers
    = asio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        1, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        42, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        50, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffers_1_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  asio::mutable_buffers_1 buffers
    = asio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        1, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        42, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        50, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(mutable_write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_4_arg_multi_buffers_async_write()
{
  asio::io_service ios;
  test_stream s(ios);
  boost::array<asio::const_buffer, 2> buffers = { {
    asio::buffer(write_data, 32),
    asio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_all(),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        1, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(1),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(10),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        10, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        42, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, asio::transfer_at_least(42),
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        50, &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, old_style_transfer_all,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  asio::async_write(s, buffers, short_transfer,
      boost::bind(async_write_handler,
        asio::placeholders::error,
        asio::placeholders::bytes_transferred,
        sizeof(write_data), &called));
  ios.reset();
  ios.run();
  BOOST_CHECK(called);
  BOOST_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

test_suite* init_unit_test_suite(int, char*[])
{
  test_suite* test = BOOST_TEST_SUITE("write");
  test->add(BOOST_TEST_CASE(&test_2_arg_zero_buffers_write));
  test->add(BOOST_TEST_CASE(&test_2_arg_const_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_2_arg_mutable_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_2_arg_multi_buffers_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_const_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_mutable_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_multi_buffers_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_const_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_mutable_buffers_1_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_multi_buffers_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_const_buffers_1_async_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_mutable_buffers_1_async_write));
  test->add(BOOST_TEST_CASE(&test_3_arg_multi_buffers_async_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_const_buffers_1_async_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_mutable_buffers_1_async_write));
  test->add(BOOST_TEST_CASE(&test_4_arg_multi_buffers_async_write));
  return test;
}
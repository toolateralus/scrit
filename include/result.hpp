
#pragma once

#include <cstdio>
#include <functional>
#include <string>

struct None {};

template <typename _Err, typename _Ok> struct Result {
  static auto Ok(_Ok ok) -> Result<_Err, _Ok> { return {ok}; }
  static auto Err(_Err err) -> Result<_Err, _Ok> { return {err}; }
  Result(_Err err) {
    this->err = new _Err(err);
    this->ok = nullptr;
  }
  Result(_Ok ok) {
    this->ok = new _Ok(ok);
    this->err = nullptr;
  }
  auto is_ok() -> bool { return ok != nullptr; }
  auto is_err() -> bool { return err != nullptr; }

  auto match(std::function<void(_Err)> err,
             std::function<void(_Ok)> ok) -> void {
    if (is_ok()) {
      ok(*this->ok);
    } else {
      err(*this->err);
    }
  }

  auto get_ok_unsafe() -> _Ok * { return ok; }

  auto get_err_unsafe() -> _Err * { return err; }
  ~Result() {
    if (is_err()) {
      delete err;
    } else {
      delete ok;
    }
  }

private:
  const _Err *err;
  const _Ok *ok;
};

auto f() {
  using Result = Result<None, std::string>;

  auto result = Result::Ok("something");

  result.match([](None) { printf("%s", "none"); },
               [](std::string s) { printf("%s", s.c_str()); });
}

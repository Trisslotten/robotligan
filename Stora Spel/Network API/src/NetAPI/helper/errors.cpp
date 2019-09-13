#include <NetAPI/helper/errors.hpp>

std::string NetAPI::Helper::PrintWSAError(int err) {
  std::string ret_str;
  wchar_t msgbuf[256];
  msgbuf[0] = '\0';
  FormatMessage(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  // flags
      NULL,                                                        // lpsource
      err,                                                         // message id
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),                   // languageid
      msgbuf,          // output buffer
      sizeof(msgbuf),  // size of msgbuf, bytes
      NULL);
  if (!*msgbuf) {
    ret_str = std::to_string(err);
  }
  return ret_str;
}

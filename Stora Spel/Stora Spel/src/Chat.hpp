#ifndef CHAT_HPP_
#define CHAT_HPP_

#include <glob/graphics.hpp>
#include <string>
#include <vector>

struct RowMessage {
  std::string name = "";
  unsigned int message_from;
  std::string message = "";
  float offset = 0.f;
};

class Chat {
 private:
  std::vector<RowMessage> messages_;
  std::string current_message_;
  float close_timer_;
  int row_length_ = 50;
  bool show_chat_ = false;
  bool send_message_ = false;
  bool close_chat_ = false;
  glm::vec2 draw_pos_;
  public : 
  void AddMessage(std::string name, std::string text, unsigned int message_from);
  //void AddToCurrentMessage(std::string text);
  void Update(float dt);
  void SubmitText(glob::Font2DHandle font);
  void SetShowChat();
  void SetSendMessage(bool send);
  void CloseChat();
  std::string GetCurrentMessage();
  bool IsVisable();
  bool IsClosing();
  bool IsTakingChatInput();
  void SetPosition(glm::vec2 pos) { draw_pos_ = pos; }
  glm::vec2 GetPosition() { return draw_pos_; }
};

#endif  // CHAT_HPP_
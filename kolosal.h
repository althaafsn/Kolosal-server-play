#ifndef KOLASAL_H
#define KOLASAL_H

#include <string>
#include <vector>
#include <chrono>
#include "imgui.h"

// Forward declaration of GLFWwindow to avoid including GLFW/glfw3.h
struct GLFWwindow;

// Message class with timestamp
class Message {
public:
    Message(const std::string &content, bool isUser);
    std::string getContent() const;
    bool isUserMessage() const;
    std::string getTimestamp() const;

private:
    std::string content;
    bool isUser;
    std::chrono::system_clock::time_point timestamp;
};

// Manages the history of chat messages
class ChatHistory {
public:
    void addMessage(const Message &message);
    const std::vector<Message> &getMessages() const;

private:
    std::vector<Message> messages;
};

// Handles simple bot responses
class ChatBot {
public:
    ChatBot();
    void processUserInput(const std::string &input);
    const ChatHistory &getChatHistory() const;

private:
    ChatHistory chatHistory;
};

// Global chat bot instance
extern ChatBot chatBot;

// Function prototypes for UI rendering and OpenGL/GLFW setup
bool initializeGLFW();
GLFWwindow* createWindow();
bool initializeGLAD();
void setupImGui(GLFWwindow* window);
void mainLoop(GLFWwindow* window);
void cleanup(GLFWwindow* window);

// UI-specific rendering functions
void renderChatWindow(bool &focusInputField, float inputHeight);
void renderChatHistory(const ChatHistory &chatHistory);
void renderMessage(const Message &msg, int index);
void pushIDAndColors(const Message &msg, int index);
std::tuple<float, float, float, float> calculateDimensions(const Message &msg);
void renderMessageContent(const Message &msg, float bubbleWidth, float bubblePadding);
void renderTimestamp(const Message &msg, float bubblePadding);
void renderButtons(const Message &msg, int index, float bubbleWidth, float bubblePadding);
void renderInputField(bool &focusInputField,
                      float inputHeight = 100.0f,
                      float maxInputWidth = 750.0f,
                      float placeholderPaddingX = 0.0f,
                      float placeholderPaddingY = 0.0f);
void renderPlaceholderText(float inputHeight);
void setInputFieldStyle();
void restoreInputFieldStyle();
void handleInputSubmission(char *inputText, bool &focusInputField);

#endif // KOLASAL_H

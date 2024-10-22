#include "kolosal.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Define global instance of ChatBot
ChatBot chatBot;

// Global constants for padding
static const float framePaddingX = 10.0f;
static const float framePaddingY = 10.0f;

/**
 * @brief Constructs a new Message object.
 * 
 * @param content The content of the message.
 * @param isUser Indicates if the message is from a user.
 */
Message::Message(const std::string &content, bool isUser)
    : content(content), isUser(isUser), timestamp(std::chrono::system_clock::now()) {}

/**
 * @brief Gets the content of the message.
 * 
 * @return std::string The content of the message.
 */
std::string Message::getContent() const {
    return content;
}

/**
 * @brief Checks if the message is from a user.
 * 
 * @return bool True if the message is from a user, false otherwise.
 */
bool Message::isUserMessage() const {
    return isUser;
}

/**
 * @brief Gets the timestamp of the message in a formatted string.
 * 
 * @return std::string The formatted timestamp of the message.
 */
std::string Message::getTimestamp() const {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm;

#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/**
 * @brief Adds a message to the chat history.
 * @param message The message to add.
 */
void ChatHistory::addMessage(const Message &message) {
    messages.push_back(message);
}

/**
 * @brief Retrieves the chat history.
 * @return A constant reference to the vector of messages.
 */
const std::vector<Message> &ChatHistory::getMessages() const {
    return messages;
}

/**
 * @brief Default constructor for ChatBot.
 */
ChatBot::ChatBot() {}

/**
 * @brief Processes user input and generates a response.
 * 
 * @param input The user's input string.
 */
void ChatBot::processUserInput(const std::string &input) {
    chatHistory.addMessage(Message(input, true));
    std::string response = "Bot: " + input;
    chatHistory.addMessage(Message(response, false));
}

/**
 * @brief Retrieves the chat history.
 * 
 * @return const ChatHistory& Reference to the chat history.
 */
const ChatHistory &ChatBot::getChatHistory() const {
    return chatHistory;
}

/**
 * @brief Initializes the GLFW library.
 * 
 * @return true if the GLFW library is successfully initialized, false otherwise.
 */
bool initializeGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Creates a GLFW window with an OpenGL context.
 * 
 * @return A pointer to the created GLFW window, or nullptr if creation fails.
 */
GLFWwindow* createWindow() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Chatbot", NULL, NULL);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    return window;
}

/**
 * @brief Initializes the GLAD library to load OpenGL function pointers.
 * 
 * @return true if GLAD is successfully initialized, false otherwise.
 */
bool initializeGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Sets up the ImGui context and initializes the platform/renderer backends.
 * 
 * @param window A pointer to the GLFW window.
 */
void setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImFont* interFont = io.Fonts->AddFontFromFileTTF(IMGUI_FONT_PATH, 16.0f);
    if (interFont == nullptr) {
        std::cerr << "Failed to load font: " << IMGUI_FONT_PATH << std::endl;
    } else {
        io.FontDefault = interFont;
    }
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

/**
 * @brief The main loop of the application, which handles rendering and event polling.
 * 
 * @param window A pointer to the GLFW window.
 */
void mainLoop(GLFWwindow* window) {
    bool focusInputField = true;
    float inputHeight = 100.0f; // Set your desired input field height here

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderChatWindow(focusInputField, inputHeight);
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}


/**
 * @brief Cleans up ImGui and GLFW resources before exiting the application.
 * 
 * @param window A pointer to the GLFW window to be destroyed.
 */
void cleanup(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

/**
 * @brief Pushes an ID and sets the colors for the ImGui context based on the message type.
 * 
 * @param msg The message object containing information about the message.
 * @param index The index used to set the ImGui ID.
 */
void pushIDAndColors(const Message &msg, int index) {
    ImGui::PushID(index);

    ImVec4 userColor = ImVec4(47/255.0f, 47/255.0f, 47/255.0f, 1.0f); // #2f2f2f for user
    ImVec4 transparentColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);         // Transparent for bot

    ImVec4 bgColor = msg.isUserMessage() ? userColor : transparentColor;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // White text
}

/**
 * @brief Calculates the dimensions for the message bubble.
 * 
 * @param msg The message object containing information about the message.
 * @return A tuple containing:
 *         - windowWidth: The width of the ImGui window content region.
 *         - bubbleWidth: The width of the message bubble.
 *         - bubblePadding: The padding inside the message bubble.
 *         - paddingX: The horizontal padding to align the bubble.
 */
std::tuple<float, float, float, float> calculateDimensions(const Message &msg) {
    float windowWidth = ImGui::GetWindowContentRegionMax().x;
    float bubbleWidth = windowWidth * 0.75f; // 75% width for both user and bot
    float bubblePadding = 15.0f; // Padding inside the bubble
    float rightPadding = 20.0f;  // Right padding for both bubbles
    float paddingX = msg.isUserMessage() ? (windowWidth - bubbleWidth - rightPadding) : 20.0f; // Align bot bubble with user

    return {windowWidth, bubbleWidth, bubblePadding, paddingX};
}

/**
 * @brief Renders the message content inside the bubble.
 * 
 * @param msg The message object containing the content to be displayed.
 * @param bubbleWidth The width of the message bubble.
 * @param bubblePadding The padding inside the message bubble.
 */
void renderMessageContent(const Message &msg, float bubbleWidth, float bubblePadding) {
    ImGui::SetCursorPosX(bubblePadding);
    ImGui::SetCursorPosY(bubblePadding);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + bubbleWidth - (bubblePadding * 2));
    ImGui::TextWrapped("%s", msg.getContent().c_str());
    ImGui::PopTextWrapPos();
}

/**
 * @brief Renders the timestamp of a message in the UI.
 * 
 * @param msg The message object containing the timestamp to render.
 * @param bubblePadding The padding to apply on the left side of the timestamp.
 */
void renderTimestamp(const Message &msg, float bubblePadding) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); // Light gray for timestamp
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 5.0f); // Align timestamp at the bottom
    ImGui::SetCursorPosX(bubblePadding); // Align timestamp to the left
    ImGui::TextWrapped("%s", msg.getTimestamp().c_str());
    ImGui::PopStyleColor();
}

/**
 * @brief Renders interactive buttons for a message bubble.
 * 
 * @param msg The message object containing the content and metadata.
 * @param index The index of the message in the message list.
 * @param bubbleWidth The width of the message bubble.
 * @param bubblePadding The padding inside the message bubble.
 */
void renderButtons(const Message &msg, int index, float bubbleWidth, float bubblePadding) {
    if (msg.isUserMessage()) {
        float buttonWidth = 60.0f;
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 5.0f); // Align button at the bottom
        ImGui::SetCursorPosX(bubbleWidth - bubblePadding - buttonWidth); // Align to right inside bubble

        if (ImGui::Button("Copy", ImVec2(buttonWidth, 0))) {
            ImGui::SetClipboardText(msg.getContent().c_str()); // Copy message content to clipboard
            std::cout << "Copy button clicked for message index " << index << std::endl;
        }
    } else {
        float buttonWidth = 60.0f;
        float spacing = 10.0f;
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 5.0f); // Align buttons at the bottom
        ImGui::SetCursorPosX(bubbleWidth - bubblePadding - (2 * buttonWidth + spacing)); // Align to right inside bubble

        if (ImGui::Button("Like", ImVec2(buttonWidth, 0))) {
            std::cout << "Like button clicked for message index " << index << std::endl;
        }

        ImGui::SameLine(0.0f, spacing);

        if (ImGui::Button("Dislike", ImVec2(buttonWidth, 0))) {
            std::cout << "Dislike button clicked for message index " << index << std::endl;
        }
    }
}

/**
 * @brief Renders a message bubble with associated UI elements.
 * 
 * @param msg The message object to be rendered.
 * @param index The index of the message in the list.
 */
void renderMessage(const Message &msg, int index) {
    pushIDAndColors(msg, index);

    auto [windowWidth, bubbleWidth, bubblePadding, paddingX] = calculateDimensions(msg);

    ImVec2 textSize = ImGui::CalcTextSize(msg.getContent().c_str(), NULL, true, bubbleWidth - bubblePadding * 2);
    float estimatedHeight = textSize.y + bubblePadding * 2 + ImGui::GetTextLineHeightWithSpacing(); // Add padding and spacing for buttons

    ImGui::SetCursorPosX(paddingX);

    if (msg.isUserMessage()) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f); // Adjust the rounding as desired
    }

    ImGui::BeginGroup();
    ImGui::BeginChild(("MessageCard" + std::to_string(index)).c_str(), ImVec2(bubbleWidth, estimatedHeight), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    renderMessageContent(msg, bubbleWidth, bubblePadding);
    renderTimestamp(msg, bubblePadding);
    renderButtons(msg, index, bubbleWidth, bubblePadding);

    ImGui::EndChild();
    ImGui::EndGroup();

    if (msg.isUserMessage()) {
        ImGui::PopStyleVar(); // Pop ChildRounding
    }

    ImGui::PopStyleColor(2); // Pop Text and ChildBg colors
    ImGui::PopID();
    ImGui::Spacing(); // Add spacing between messages
}

/**
 * @brief Renders the chat history with an auto-scroll feature.
 * 
 * @param chatHistory The chat history object containing the messages to be displayed.
 * @param autoScroll A boolean flag indicating whether auto-scroll is enabled.
 */
void renderChatHistory(const ChatHistory &chatHistory) {
    static size_t lastMessageCount = 0;
    size_t currentMessageCount = chatHistory.getMessages().size();

    // Check if new messages have been added
    bool newMessageAdded = currentMessageCount > lastMessageCount;

    // Save the scroll position before rendering
    float scrollY = ImGui::GetScrollY();
    float scrollMaxY = ImGui::GetScrollMaxY();
    bool isAtBottom = (scrollMaxY <= 0.0f) || (scrollY >= scrollMaxY - 1.0f);

    // Render messages
    const auto &messages = chatHistory.getMessages();
    for (size_t i = 0; i < messages.size(); ++i) {
        renderMessage(messages[i], static_cast<int>(i));
    }

    // If the user was at the bottom and new messages were added, scroll to bottom
    if (newMessageAdded && isAtBottom) {
        ImGui::SetScrollHereY(1.0f);
    }

    // Update the last message count
    lastMessageCount = currentMessageCount;
}

/**
 * @brief Sets the custom style for the input field.
 */
void setInputFieldStyle() {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f); // Rounded corners
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(framePaddingX, framePaddingY)); // Padding inside input field
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f)); // Background color of the input field
}

/**
 * @brief Restores the original style settings for the input field.
 */
void restoreInputFieldStyle() {
    ImGui::PopStyleColor(1); // Restore FrameBg
    ImGui::PopStyleVar(2);   // Restore frame rounding and padding
}

/**
 * @brief Handles the submission of the input text.
 * 
 * @param inputText The input text buffer.
 * @param focusInputField A reference to the focus input field flag.
 */
void handleInputSubmission(char *inputText, bool &focusInputField) {
    std::string inputStr(inputText);
    inputStr.erase(0, inputStr.find_first_not_of(" \n\r\t"));
    inputStr.erase(inputStr.find_last_not_of(" \n\r\t") + 1);

    if (!inputStr.empty()) {
        chatBot.processUserInput(inputStr);
        memset(inputText, 0, sizeof(char) * 1024); // Empty the input after submission
    }

    focusInputField = true;
}

/**
 * @brief Renders the input field with text wrapping and no horizontal scrolling.
 * 
 * @param focusInputField A reference to the focus input field flag.
 * @param inputHeight The height of the input field.
 * @param maxInputWidth The maximum width of the input field.
 * @param placeholderPaddingX Horizontal padding for the placeholder text.
 * @param placeholderPaddingY Vertical padding for the placeholder text.
 */
void renderInputField(bool &focusInputField,
                      float inputHeight,
                      float maxInputWidth,
                      float placeholderPaddingX,
                      float placeholderPaddingY) {
    setInputFieldStyle();

    static char inputText[1024] = "";

    if (focusInputField) {
        ImGui::SetKeyboardFocusHere();
        focusInputField = false;
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                ImGuiInputTextFlags_CtrlEnterForNewLine;

    float availableWidth = ImGui::GetContentRegionAvail().x;
    float inputWidth = std::min(maxInputWidth, availableWidth); // Ensure the input doesn't exceed the available width
    float paddingX = (availableWidth - inputWidth) / 2.0f;

    if (paddingX > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + paddingX); // Center horizontally
    }

    ImVec2 inputFieldPos = ImGui::GetCursorScreenPos();
    ImVec2 inputSize = ImVec2(inputWidth, inputHeight);

    bool isEmpty = (strlen(inputText) == 0);

    if (ImGui::InputTextMultiline("##input", inputText, IM_ARRAYSIZE(inputText), inputSize, flags)) {
        handleInputSubmission(inputText, focusInputField);
    }

    if (isEmpty) {
        const ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 textPos = inputFieldPos;
        textPos.x += style.FramePadding.x + placeholderPaddingX;
        textPos.y += style.FramePadding.y + placeholderPaddingY;
        ImGui::SetCursorScreenPos(textPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Light gray
        ImGui::TextUnformatted("Type a message and press Enter to send (Ctrl+Enter for new line)");
        ImGui::PopStyleColor();
        ImGui::SetCursorScreenPos(inputFieldPos);
    }

    restoreInputFieldStyle();
}

/**
 * @brief Renders the chat window to cover the full width and height of the application window.
 * 
 * @param autoScroll A boolean reference to control automatic scrolling of the chat history.
 * @param focusInputField A boolean reference to control the focus state of the input field.
 * @param inputHeight The height of the input field.
 */
void renderChatWindow(bool &focusInputField, float inputHeight) {
    ImGuiIO &io = ImGui::GetIO();

    // Set window to cover the entire display
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Chatbot", nullptr, window_flags);

    // Calculate available height for the input field and chat history
    float inputFieldHeight = inputHeight;
    float bottomMargin = 10.0f;
    float availableHeight = ImGui::GetContentRegionAvail().y;

    // Adjust the height of the scrolling region
    float scrollingRegionHeight = availableHeight - inputFieldHeight - bottomMargin;

    // Ensure the scrolling region height is not negative
    if (scrollingRegionHeight < 0.0f) {
        scrollingRegionHeight = 0.0f;
    }

    // Begin the child window for the chat history
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, scrollingRegionHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Render chat history
    renderChatHistory(chatBot.getChatHistory());

    ImGui::EndChild();

    // Render the input field at the bottom
    renderInputField(focusInputField, inputHeight);

    ImGui::End();
}
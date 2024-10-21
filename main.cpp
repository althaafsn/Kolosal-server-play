/**
 * @file main.cpp
 * @brief Kolosal AI dekstop application using ImGui and GLFW with OpenGL.
 * 
 * This application demonstrates a chat interface where users can input messages,
 * and a simple bot responds to those messages. The chat interface includes features
 * like message bubbles, timestamps, and buttons for copying messages and liking/disliking bot responses.
 */

#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

#ifndef IMGUI_FONT_PATH
#define IMGUI_FONT_PATH "path/to/font.ttf" // Fallback definition if not provided by CMake
#endif

/**
 * @brief Message class with timestamp.
 */
class Message
{
public:
    /**
     * @brief Constructs a new Message object.
     * 
     * @param content The content of the message.
     * @param isUser Indicates if the message is from a user.
     */
    Message(const std::string &content, bool isUser)
        : content(content), isUser(isUser), timestamp(std::chrono::system_clock::now()) {}

    /**
     * @brief Gets the content of the message.
     * 
     * @return std::string The content of the message.
     */
    std::string getContent() const { return content; }

    /**
     * @brief Checks if the message is from a user.
     * 
     * @return bool True if the message is from a user, false otherwise.
     */
    bool isUserMessage() const { return isUser; }

    /**
     * @brief Gets the timestamp of the message in a formatted string.
     * 
     * @return std::string The formatted timestamp of the message.
     */
    std::string getTimestamp() const
    {
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

private:
    std::string content; ///< The content of the message.
    bool isUser; ///< Indicates if the message is from a user.
    std::chrono::system_clock::time_point timestamp; ///< The timestamp of the message.
};

/**
 * @brief Manages the history of chat messages.
 */
class ChatHistory
{
public:
    /**
     * @brief Adds a message to the chat history.
     * @param message The message to add.
     */
    void addMessage(const Message &message) { messages.push_back(message); }

    /**
     * @brief Retrieves the chat history.
     * @return A constant reference to the vector of messages.
     */
    const std::vector<Message> &getMessages() const { return messages; }

private:
    std::vector<Message> messages; ///< Container for storing chat messages.
};

class ChatBot
{
public:
    /**
     * @brief Default constructor for ChatBot.
     */
    ChatBot() {}

    /**
     * @brief Processes user input and generates a response.
     * 
     * @param input The user's input string.
     */
    void processUserInput(const std::string &input)
    {
        chatHistory.addMessage(Message(input, true));
        // Simple response logic
        std::string response = "Bot: " + input;
        chatHistory.addMessage(Message(response, false));
    }

    /**
     * @brief Retrieves the chat history.
     * 
     * @return const ChatHistory& Reference to the chat history.
     */
    const ChatHistory &getChatHistory() const { return chatHistory; }

private:
    ChatHistory chatHistory; ///< Stores the chat history.
};

/**
 * @brief Global instance of the ChatBot class.
 */
ChatBot chatBot;

/**
 * @brief Flag to indicate whether input should be submitted.
 */
bool shouldSubmitInput = false;

/**
 * @brief Global flag to control padding for X-axis alignment.
 */
static const float framePaddingX = 10.0f;

/**
 * @brief Global flag to control padding for Y-axis alignment.
 */
static const float framePaddingY = 10.0f;

/**
 * @brief Pushes an ID and sets the colors for the ImGui context based on the message type.
 * 
 * This function sets the ImGui ID and style colors for the background and text
 * based on whether the message is from a user or a bot.
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
 * This function computes the dimensions required for rendering a message bubble
 * within the ImGui window. It adjusts the width and padding based on whether the
 * message is from the user or the bot.
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
 * This function sets up the text wrapping and positions the cursor
 * appropriately to render the message content.
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
 * This function sets the style and position for the timestamp text and renders it
 * at the bottom left of the message bubble with specified padding.
 * 
 * @param msg The message object containing the timestamp to render.
 * @param bubblePadding The padding to apply on the left side of the timestamp.
 */
void renderTimestamp(const Message &msg, float bubblePadding) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));                          // Light gray for timestamp
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 5.0f); // Align timestamp at the bottom
    ImGui::SetCursorPosX(bubblePadding);                                                           // Align timestamp to the left
    ImGui::TextWrapped("%s", msg.getTimestamp().c_str());
    ImGui::PopStyleColor();
}

/**
 * @brief Renders interactive buttons for a message bubble.
 * 
 * This function renders different buttons based on whether the message is a user message or not.
 * For user messages, a "Copy" button is rendered. For other messages, "Like" and "Dislike" buttons are rendered.
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
        ImGui::SetCursorPosX(bubbleWidth - bubblePadding - buttonWidth);                               // Align to right inside bubble

        if (ImGui::Button("Copy", ImVec2(buttonWidth, 0))) {
            ImGui::SetClipboardText(msg.getContent().c_str()); // Copy message content to clipboard
            std::cout << "Copy button clicked for message index " << index << std::endl;
        }
    } else {
        float buttonWidth = 60.0f;
        float spacing = 10.0f;
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 5.0f); // Align buttons at the bottom
        ImGui::SetCursorPosX(bubbleWidth - bubblePadding - (2 * buttonWidth + spacing));               // Align to right inside bubble

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
 * This function handles the rendering of a message bubble, including its
 * timestamp and action buttons. It also manages the necessary ImGui
 * state changes such as pushing and popping IDs and colors.
 * 
 * @param msg The message object to be rendered.
 * @param index The index of the message in the list.
 */
void renderMessage(const Message &msg, int index) {
    pushIDAndColors(msg, index);

    auto [windowWidth, bubbleWidth, bubblePadding, paddingX] = calculateDimensions(msg);

    // Calculate the estimated height for the child window
    ImVec2 textSize = ImGui::CalcTextSize(msg.getContent().c_str(), NULL, true, bubbleWidth - bubblePadding * 2);
    float estimatedHeight = textSize.y + bubblePadding * 2 + ImGui::GetTextLineHeightWithSpacing(); // Add padding and spacing for buttons

    ImGui::SetCursorPosX(paddingX);

    // Set rounding if user message
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
 * This function displays the chat messages in a scrollable region. If auto-scroll is enabled,
 * it will automatically scroll to the bottom when new messages are added.
 * 
 * @param chatHistory The chat history object containing the messages to be displayed.
 * @param autoScroll A boolean flag indicating whether auto-scroll is enabled.
 */
void renderChatHistory(const ChatHistory &chatHistory, bool &autoScroll)
{
    // Define the height reserved for the input field
    float inputFieldHeight = 60.0f; // Adjust based on your UI design

    // Begin the child window with a height that reserves space for the input field
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -inputFieldHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    const auto &messages = chatHistory.getMessages();
    for (size_t i = 0; i < messages.size(); ++i)
    {
        renderMessage(messages[i], static_cast<int>(i));
    }

    // Auto-scroll to bottom if enabled and at the end
    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}

/**
 * @brief Sets the custom style for the input field.
 * 
 * This function customizes the appearance of the input field by setting
 * the frame rounding, padding, and background color using ImGui style variables.
 * 
 * @return void
 */
void setInputFieldStyle()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);                                // Rounded corners
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(framePaddingX, framePaddingY));  // Padding inside input field
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));             // Background color of the input field
}

/**
 * @brief Restores the original style settings for the input field.
 * 
 * This function pops the style color and style variables from the ImGui stack,
 * effectively restoring the previous style settings for the input field.
 */
void restoreInputFieldStyle()
{
    ImGui::PopStyleColor(1); // Restore FrameBg
    ImGui::PopStyleVar(2);   // Restore frame rounding and padding
}

/**
 * @brief Handles the submission of the input text.
 * 
 * This function trims whitespace from the input text, processes the input if it is not empty,
 * clears the input buffer, and ensures the focus remains on the input field.
 * 
 * @param inputText The input text buffer.
 * @param focusInputField A reference to the focus input field flag.
 */
void handleInputSubmission(char *inputText, bool &focusInputField)
{
    // Trim whitespace from input
    std::string inputStr(inputText);
    inputStr.erase(0, inputStr.find_first_not_of(" \n\r\t"));
    inputStr.erase(inputStr.find_last_not_of(" \n\r\t") + 1);

    if (!inputStr.empty())
    {
        // Add the user input to the chat
        chatBot.processUserInput(inputStr);

        // Clear the input buffer after submission
        memset(inputText, 0, sizeof(inputText)); // Empty the input after submission
    }

    // Ensure focus remains on the input field after submitting
    focusInputField = true;
}

/**
 * @brief Renders the placeholder text when the input field is empty.
 * 
 * This function calculates the position for the placeholder text, adds padding,
 * and renders the placeholder text in a light gray color.
 * 
 * @param inputHeight The height of the input field to properly adjust placeholder position.
 */
void renderPlaceholderText(float inputHeight)
{
    ImVec2 cursorPos = ImGui::GetItemRectMin(); // Get the minimum coordinates of the item rectangle
    ImVec2 itemSize = ImGui::GetItemRectSize(); // Get the size of the item rectangle

    // Adjust the padding for the placeholder based on the dynamic input height
    float verticalPadding = (inputHeight - 18.0f) / 2.0f; // Adjust the vertical padding dynamically
    ImVec2 placeholderPadding = ImVec2(10.0f, verticalPadding); // Add padding for text centering

    // Set cursor position to the correct place for the placeholder text
    ImGui::SetCursorPos(ImVec2(cursorPos.x + placeholderPadding.x, cursorPos.y + placeholderPadding.y));

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Set text color to light gray
    ImGui::TextUnformatted("Type a message and press Enter to send (Ctrl+Enter for new line)"); // Render placeholder text
    ImGui::PopStyleColor(); // Restore previous text color
}

/**
 * @brief Renders a custom input field with text wrapping, no horizontal scrolling, 
 *        centered horizontally, with adjustable width, height, and placeholder padding.
 * 
 * @param focusInputField A reference to the focus input field flag. If true, the input field will be focused.
 * @param inputHeight The height of the input field. Allows dynamic height adjustment.
 * @param maxInputWidth The maximum width of the input field. Ensures the input field does not exceed a certain width.
 * @param placeholderPaddingX Horizontal padding for the placeholder text.
 * @param placeholderPaddingY Vertical padding for the placeholder text.
 */
void renderInputField(bool &focusInputField,
                      float inputHeight = 100.0f,
                      float maxInputWidth = 750.0f,
                      float placeholderPaddingX = 0.0f,
                      float placeholderPaddingY = 0.0f)
{
    setInputFieldStyle();

    // Input buffer
    static char inputText[1024] = ""; // Buffer for input text

    // Handle focus input field
    if (focusInputField)
    {
        ImGui::SetKeyboardFocusHere();
        focusInputField = false;
    }

    // Define flags for InputTextMultiline
    // TODO: need to be able to disable horizontal scrolling and enable text wrapping
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                ImGuiInputTextFlags_CtrlEnterForNewLine;

    // Get the available width of the window to calculate horizontal centering
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float inputWidth = std::min(maxInputWidth, availableWidth); // Ensure the input doesn't exceed the available width

    // Calculate the X position to center the input field
    float paddingX = (availableWidth - inputWidth) / 2.0f;
    if (paddingX > 0.0f)
    {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + paddingX); // Center horizontally
    }

    // Save the cursor position (input field position)
    ImVec2 inputFieldPos = ImGui::GetCursorScreenPos();

    // Set the size of the input box with adjustable height
    ImVec2 inputSize = ImVec2(inputWidth, inputHeight); // Input height is now adjustable

    // Check if the input text is empty (before rendering the input box)
    bool isEmpty = (strlen(inputText) == 0);

    // Create multi-line input field
    if (ImGui::InputTextMultiline("##input", inputText, IM_ARRAYSIZE(inputText), inputSize, flags))
    {
        handleInputSubmission(inputText, focusInputField);
    }

    // If the input text is empty, show the placeholder even when the input is focused
    if (isEmpty)
    {
        // Get the style and calculate the position for the placeholder text
        const ImGuiStyle& style = ImGui::GetStyle();

        // Adjust the placeholder position based on frame padding and user-defined padding
        ImVec2 textPos = inputFieldPos;
        textPos.x += style.FramePadding.x + placeholderPaddingX;
        textPos.y += style.FramePadding.y + placeholderPaddingY;

        // Set the cursor position for the placeholder text
        ImGui::SetCursorScreenPos(textPos);

        // Set the text color to a lighter shade for the placeholder
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f)); // Light gray

        // Render the placeholder text
        ImGui::TextUnformatted("Type a message and press Enter to send (Ctrl+Enter for new line)");

        ImGui::PopStyleColor();

        // Reset the cursor position after rendering the placeholder
        ImGui::SetCursorScreenPos(inputFieldPos);
    }

    restoreInputFieldStyle();
}

/**
 * @brief Renders the chat window to cover the full width and height of the application window.
 * 
 * This function sets up an ImGui window that covers the entire display area, 
 * with specific flags to disable interactions that can alter the window size or position.
 * It then renders the chat history and input field within this window.
 * 
 * @param autoScroll A boolean reference to control automatic scrolling of the chat history.
 * @param focusInputField A boolean reference to control the focus state of the input field.
 * @param inputHeight The height of the input field. Ensures consistent layout.
 */
void renderChatWindow(bool &autoScroll, bool &focusInputField, float inputHeight)
{
    // Get ImGui IO
    ImGuiIO &io = ImGui::GetIO();

    // Set the window to cover the entire display
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    // Define window flags to disable interactions that can alter the window size or position
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoScrollbar |         // No scrollbar for main window
                                    ImGuiWindowFlags_NoScrollWithMouse;    // Prevent scrolling of the main window

    // Begin the ImGui window with the specified flags
    ImGui::Begin("Chatbot", nullptr, window_flags);

    // Define the height reserved for the input field
    float inputFieldHeight = inputHeight; // Use the same inputHeight as in renderInputField
    float bottomMargin = 10.0f;     // Add a margin at the bottom to prevent cropping

    // Render the chat history as a scrollable region without borders
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -inputFieldHeight - bottomMargin), false, ImGuiWindowFlags_HorizontalScrollbar);
    renderChatHistory(chatBot.getChatHistory(), autoScroll);
    ImGui::EndChild();

    // Render the input field separately (outside the scrollable region)
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - inputFieldHeight - bottomMargin); // Place the input field at the bottom with margin
    renderInputField(focusInputField, inputHeight); // Pass inputHeight to renderInputField

    // End the ImGui window
    ImGui::End();
}


/**
 * @brief Initializes the GLFW library.
 * 
 * This function attempts to initialize the GLFW library, which is required
 * for creating windows and handling input in OpenGL applications.
 * 
 * @return true if the GLFW library is successfully initialized, false otherwise.
 */
bool initializeGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Creates a GLFW window with an OpenGL context.
 * 
 * This function sets the GLFW window hints for OpenGL version 3.3 and core profile.
 * It then attempts to create a window with the specified dimensions and title.
 * If the window creation fails, an error message is printed and GLFW is terminated.
 * 
 * @return A pointer to the created GLFW window, or nullptr if creation fails.
 */
GLFWwindow* createWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "ImGui Chatbot", NULL, NULL);
    if (window == NULL)
    {
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
 * This function uses glfwGetProcAddress to load the OpenGL function pointers
 * through GLAD. It is essential to call this function before making any OpenGL
 * calls to ensure that all necessary functions are properly loaded.
 * 
 * @return true if GLAD is successfully initialized, false otherwise.
 */
bool initializeGLAD()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    return true;
}

/**
 * @brief Sets up the ImGui context and initializes the platform/renderer backends.
 * 
 * This function initializes the ImGui context, sets up the default font, 
 * applies the dark style, and configures the GLFW and OpenGL backends.
 * 
 * @param window A pointer to the GLFW window.
 */
void setupImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImFont* interFont = io.Fonts->AddFontFromFileTTF(IMGUI_FONT_PATH, 16.0f);
    if (interFont == NULL)
    {
        std::cerr << "Failed to load font: " << IMGUI_FONT_PATH << std::endl;
    }
    else
    {
        io.FontDefault = interFont;
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup clipboard functions for ImGui
    io.SetClipboardTextFn = [](void* user_data, const char* text)
    { glfwSetClipboardString((GLFWwindow*)user_data, text); };
    io.GetClipboardTextFn = [](void* user_data)
    { return glfwGetClipboardString((GLFWwindow*)user_data); };
    io.ClipboardUserData = window; // Set the window as user data for clipboard access
}

/**
 * @brief The main loop of the application, which handles rendering and event polling.
 * 
 * This function continuously polls for events, updates the ImGui frame, renders the chat window,
 * and swaps the buffers until the window should close.
 * 
 * @param window A pointer to the GLFW window.
 */
void mainLoop(GLFWwindow* window)
{
    bool focusInputField = true;
    bool autoScroll = true;
    float inputHeight = 100.0f; // Set your desired input field height here

    while (!glfwWindowShouldClose(window))
    {
        // Poll for and process events
        glfwPollEvents();

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the chat window, passing the inputHeight
        renderChatWindow(autoScroll, focusInputField, inputHeight);

        // Render ImGui frame
        ImGui::Render();

        // Get the framebuffer size and set the viewport
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui draw data
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the front and back buffers
        glfwSwapBuffers(window);
    }
}

/**
 * @brief Cleans up ImGui and GLFW resources before exiting the application.
 * 
 * This function shuts down ImGui's OpenGL and GLFW implementations, destroys the ImGui context,
 * and then destroys the GLFW window and terminates the GLFW library.
 * 
 * @param window A pointer to the GLFW window to be destroyed.
 */
void cleanup(GLFWwindow* window)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

/**
 * @brief Entry point of the application.
 * 
 * Initializes GLFW, creates a window, initializes GLAD, sets up ImGui, 
 * runs the main loop, and performs cleanup.
 * 
 * @return int Returns 0 on successful execution, 1 on failure.
 */
int main()
{
    if (!initializeGLFW())
        return 1;

    GLFWwindow* window = createWindow();
    if (window == nullptr)
        return 1;

    if (!initializeGLAD())
        return 1;

    setupImGui(window);
    mainLoop(window);
    cleanup(window);

    return 0;
}
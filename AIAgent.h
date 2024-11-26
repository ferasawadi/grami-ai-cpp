#ifndef AI_AGENT_H
#define AI_AGENT_H

#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>

// Add Gemini API integration headers
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

class MemoryEntry {
public:
    MemoryEntry(const std::string& content, double importance = 1.0, 
                std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now())
        : m_content(content), 
          m_importance(importance), 
          m_timestamp(timestamp) {}

    std::string getContent() const { return m_content; }
    double getImportance() const { return m_importance; }
    std::chrono::system_clock::time_point getTimestamp() const { return m_timestamp; }

private:
    std::string m_content;
    double m_importance;
    std::chrono::system_clock::time_point m_timestamp;
};

class AIAgent {
public:
    // Constructor
    AIAgent(const std::string& name, size_t memoryCapacity = 100);

    // Define an action type
    using Action = std::function<void()>;

    // Register possible actions
    void registerAction(const std::string& actionName, Action action);

    // Make a decision and execute an action
    void executeAction(const std::string& actionName);

    // Simple learning mechanism
    void updateActionPriority(const std::string& actionName, double priority);

    // Get agent's name
    std::string getName() const;

    // Gemini API Integration Methods
    std::string queryGemini(const std::string& prompt);
    std::vector<std::string> generatePossibleActions(const std::string& context);
    std::string generateContentWithConfig(
        const std::string& prompt, 
        const std::map<std::string, std::string>& safetySettings = {},
        double temperature = 1.0, 
        int maxOutputTokens = 800, 
        double topP = 0.8, 
        int topK = 10,
        const std::vector<std::string>& stopSequences = {}
    );

    // Memory Management Methods
    void addMemory(const std::string& memory, double importance = 1.0);
    std::vector<std::string> recallMemories(const std::string& query, size_t maxResults = 5);
    void clearOldestMemories(size_t count);
    size_t getMemoryCount() const;
    void printMemories() const;

private:
    std::string m_name;
    std::map<std::string, Action> m_actions;
    std::map<std::string, double> m_actionPriorities;

    // Memory Management
    std::vector<MemoryEntry> m_memories;
    size_t m_memoryCapacity;

    // Gemini API configuration
    const std::string GEMINI_API_BASE = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent";
    const std::string API_KEY = ""; // API key removed, should be set via environment variable

    // Private helper method for memory relevance
    double calculateMemoryRelevance(const MemoryEntry& memory, const std::string& query);
};

#endif // AI_AGENT_H

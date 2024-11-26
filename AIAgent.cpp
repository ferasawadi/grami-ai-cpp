#include "AIAgent.h"

AIAgent::AIAgent(const std::string& name, size_t memoryCapacity) 
    : m_name(name), m_memoryCapacity(memoryCapacity) {}

void AIAgent::registerAction(const std::string& actionName, Action action) {
    m_actions[actionName] = action;
    // Default priority is 1.0 if not specified
    if (m_actionPriorities.find(actionName) == m_actionPriorities.end()) {
        m_actionPriorities[actionName] = 1.0;
    }
}

void AIAgent::executeAction(const std::string& actionName) {
    auto actionIt = m_actions.find(actionName);
    if (actionIt != m_actions.end()) {
        std::cout << "Agent " << m_name << " executing action: " << actionName << std::endl;
        actionIt->second(); // Call the action
    } else {
        std::cerr << "Action " << actionName << " not found!" << std::endl;
    }
}

void AIAgent::updateActionPriority(const std::string& actionName, double priority) {
    m_actionPriorities[actionName] = priority;
}

std::string AIAgent::getName() const {
    return m_name;
}

std::string AIAgent::queryGemini(const std::string& prompt) {
    try {
        // Prepare the JSON payload
        nlohmann::json payload = {
            {"contents", {
                {
                    {"parts", {
                        {{"text", prompt}}
                    }}
                }
            }}
        };

        // Construct the full URL with API key
        std::string fullUrl = GEMINI_API_BASE + "?key=" + API_KEY;

        // Make the POST request to Gemini
        cpr::Response r = cpr::Post(
            cpr::Url{fullUrl},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{payload.dump()}
        );

        // Check if the request was successful
        if (r.status_code == 200) {
            // Parse the JSON response
            auto response = nlohmann::json::parse(r.text);
            
            // Extract the text response
            std::string generatedText = response
                ["candidates"][0]
                ["content"]
                ["parts"][0]
                ["text"];
            
            return generatedText;
        } else {
            std::cerr << "Gemini API Error: " << r.status_code << std::endl;
            std::cerr << "Response: " << r.text << std::endl;
            return "Error querying Gemini API";
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in queryGemini: " << e.what() << std::endl;
        return "Exception occurred";
    }
}

std::vector<std::string> AIAgent::generatePossibleActions(const std::string& context) {
    std::string prompt = "Given the context: '" + context + 
        "', suggest 3-5 possible actions an AI agent could take. " 
        "Respond with a comma-separated list of actions.";
    
    std::string actionResponse = queryGemini(prompt);
    
    // Split the response into actions
    std::vector<std::string> actions;
    std::stringstream ss(actionResponse);
    std::string action;
    
    while (std::getline(ss, action, ',')) {
        // Trim whitespace
        action.erase(0, action.find_first_not_of(" \t"));
        action.erase(action.find_last_not_of(" \t") + 1);
        
        if (!action.empty()) {
            actions.push_back(action);
        }
    }
    
    return actions;
}

std::string AIAgent::generateContentWithConfig(
    const std::string& prompt, 
    const std::map<std::string, std::string>& safetySettings,
    double temperature, 
    int maxOutputTokens, 
    double topP, 
    int topK,
    const std::vector<std::string>& stopSequences
) {
    // Prepare the JSON payload
    nlohmann::json payload = {
        {"contents", {
            {{"parts", {
                {{"text", prompt}}
            }}}
        }},
        {"generationConfig", {
            {"temperature", temperature},
            {"maxOutputTokens", maxOutputTokens},
            {"topP", topP},
            {"topK", topK}
        }}
    };

    // Add safety settings if provided
    if (!safetySettings.empty()) {
        nlohmann::json safetyArray = nlohmann::json::array();
        for (const auto& [category, threshold] : safetySettings) {
            safetyArray.push_back({
                {"category", category},
                {"threshold", threshold}
            });
        }
        payload["safetySettings"] = safetyArray;
    }

    // Add stop sequences if provided
    if (!stopSequences.empty()) {
        payload["generationConfig"]["stopSequences"] = stopSequences;
    }

    // Prepare the request
    auto response = cpr::Post(
        cpr::Url{GEMINI_API_BASE + "?key=" + API_KEY},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    // Parse the response
    if (response.status_code == 200) {
        try {
            auto jsonResponse = nlohmann::json::parse(response.text);
            if (!jsonResponse["candidates"].empty() && 
                !jsonResponse["candidates"][0]["content"]["parts"].empty()) {
                return jsonResponse["candidates"][0]["content"]["parts"][0]["text"];
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing Gemini response: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "Gemini API request failed with status code: " 
                  << response.status_code << std::endl;
        std::cerr << "Response body: " << response.text << std::endl;
    }

    return "Error generating content";
}

// Memory Management Methods
void AIAgent::addMemory(const std::string& memory, double importance) {
    // If memory is at capacity, remove the least important memory
    if (m_memories.size() >= m_memoryCapacity) {
        // Find and remove the least important memory
        auto minIt = std::min_element(m_memories.begin(), m_memories.end(), 
            [](const MemoryEntry& a, const MemoryEntry& b) {
                return a.getImportance() < b.getImportance();
            });
        
        if (minIt != m_memories.end()) {
            m_memories.erase(minIt);
        }
    }

    // Add new memory
    m_memories.emplace_back(memory, importance);
}

double AIAgent::calculateMemoryRelevance(const MemoryEntry& memory, const std::string& query) {
    // Use Gemini to calculate semantic relevance
    std::string relevancePrompt = "How relevant is the following memory to this query? " 
        "Memory: '" + memory.getContent() + "' " 
        "Query: '" + query + "' " 
        "Respond with a relevance score from 0 to 1.";
    
    try {
        std::string relevanceStr = queryGemini(relevancePrompt);
        
        // Extract numeric relevance score
        std::istringstream iss(relevanceStr);
        double relevance = 0.0;
        iss >> relevance;

        // Fallback to 0.5 if parsing fails
        if (relevance == 0.0) relevance = 0.5;

        // Combine with memory's inherent importance
        return relevance * memory.getImportance();
    } catch (...) {
        // Default to moderate relevance if Gemini query fails
        return 0.5 * memory.getImportance();
    }
}

std::vector<std::string> AIAgent::recallMemories(const std::string& query, size_t maxResults) {
    // Calculate relevance for each memory
    std::vector<std::pair<double, std::string>> rankedMemories;
    for (const auto& memory : m_memories) {
        double relevance = calculateMemoryRelevance(memory, query);
        rankedMemories.emplace_back(relevance, memory.getContent());
    }

    // Sort memories by relevance in descending order
    std::sort(rankedMemories.begin(), rankedMemories.end(), 
        [](const auto& a, const auto& b) { return a.first > b.first; });

    // Extract top memories
    std::vector<std::string> topMemories;
    for (size_t i = 0; i < std::min(maxResults, rankedMemories.size()); ++i) {
        topMemories.push_back(rankedMemories[i].second);
    }

    return topMemories;
}

void AIAgent::clearOldestMemories(size_t count) {
    // Sort memories by timestamp
    std::sort(m_memories.begin(), m_memories.end(), 
        [](const MemoryEntry& a, const MemoryEntry& b) {
            return a.getTimestamp() < b.getTimestamp();
        });

    // Remove oldest memories
    count = std::min(count, m_memories.size());
    m_memories.erase(m_memories.begin(), m_memories.begin() + count);
}

size_t AIAgent::getMemoryCount() const {
    return m_memories.size();
}

void AIAgent::printMemories() const {
    std::cout << "Agent " << m_name << " Memories:" << std::endl;
    for (const auto& memory : m_memories) {
        std::cout << "- " << memory.getContent() 
                  << " (Importance: " << memory.getImportance() << ")" << std::endl;
    }
}

#include "AIAgent.h"
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

// Example actions
void exploreEnvironment() {
    std::cout << "Exploring the environment..." << std::endl;
}

void collectData() {
    std::cout << "Collecting data..." << std::endl;
}

void analyzeInformation() {
    std::cout << "Analyzing collected information..." << std::endl;
}

int main() {
    // Create an AI Agent with memory
    AIAgent intelligentAgent("GeminiExplorer", 10);  // 10 memory capacity

    // Register basic actions
    intelligentAgent.registerAction("explore", exploreEnvironment);
    intelligentAgent.registerAction("collect_data", collectData);
    intelligentAgent.registerAction("analyze", analyzeInformation);

    // Demonstrate Gemini API integration
    std::string context = "We are exploring a new digital environment with the goal of understanding its structure and potential resources.";
    
    // Use Gemini to generate possible actions
    std::vector<std::string> suggestedActions = intelligentAgent.generatePossibleActions(context);
    
    std::cout << "Gemini-suggested Actions:" << std::endl;
    for (const auto& action : suggestedActions) {
        std::cout << "- " << action << std::endl;
    }

    // Add memories with different importance
    intelligentAgent.addMemory("Discovered a network vulnerability", 0.9);
    intelligentAgent.addMemory("Mapped initial network topology", 0.7);
    intelligentAgent.addMemory("Identified potential security risks", 0.8);
    intelligentAgent.addMemory("Observed unusual network traffic pattern", 0.6);

    // Print current memories
    std::cout << "\nCurrent Memories:" << std::endl;
    intelligentAgent.printMemories();

    // Demonstrate memory recall
    std::string recallQuery = "What security-related information have we discovered?";
    std::vector<std::string> relevantMemories = intelligentAgent.recallMemories(recallQuery);
    
    std::cout << "\nRelevant Memories for Query '" << recallQuery << "':" << std::endl;
    for (const auto& memory : relevantMemories) {
        std::cout << "- " << memory << std::endl;
    }

    // Query Gemini for strategic insights
    std::string strategyQuery = "Provide a strategic approach for exploring an unknown digital environment, focusing on safety and efficiency.";
    std::string strategy = intelligentAgent.queryGemini(strategyQuery);
    
    std::cout << "\nStrategy from Gemini:" << std::endl;
    std::cout << strategy << std::endl;

    // Demonstrate advanced Gemini content generation with custom parameters
    std::map<std::string, std::string> safetySettings = {
        {"HARM_CATEGORY_DANGEROUS_CONTENT", "BLOCK_ONLY_HIGH"},
        {"HARM_CATEGORY_HATE_SPEECH", "BLOCK_MEDIUM_AND_ABOVE"}
    };

    std::string magicStory = intelligentAgent.generateContentWithConfig(
        "Write a story about a magic backpack that helps students learn.",
        safetySettings,  // Safety settings
        0.7,             // Temperature (creativity)
        500,             // Max output tokens
        0.9,             // Top P
        15,              // Top K
        {"The End"}      // Stop sequences
    );

    std::cout << "\n--- Magic Backpack Story ---\n" << magicStory << std::endl;

    // Execute some actions
    intelligentAgent.executeAction("explore");
    intelligentAgent.executeAction("collect_data");
    intelligentAgent.executeAction("analyze");

    return 0;
}

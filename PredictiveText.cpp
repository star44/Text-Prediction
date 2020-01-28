// PredictiveText.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <algorithm>
#include <unordered_map>
#include <random>

#define LENGTH 30

typedef struct node Node;

struct node {
	std::string sentence;
	std::vector<Node*> children;
};
	
std::default_random_engine generator;

std::string readDatasetAsString(std::ifstream* lexiconFile) {
	std::locale loc;
	std::string dataset = "";
	std::string line;
	while (getline(*lexiconFile, line)) {
		for (std::string::size_type i = 0; i < line.length(); ++i) {
			if (line[i] != ',' && line[i] != '.') {
				dataset += std::tolower(line[i], loc);
			}
		}
	}
	return dataset;
}


void buildTransitionMap(std::string dataset, std::unordered_map<std::string, int>* transitionMapPtr, int offset) {
	std::locale loc;
	std::string key = "";
	for (std::string::size_type i = offset; i < dataset.length(); ++i) {
		if (dataset[i] == ' ' && key.find(' ', 0) != std::string::npos) {
			// add to frequency if exists, otherwise overwrite
			(*transitionMapPtr)[key] = (*transitionMapPtr).count(key) == 1 ? (*transitionMapPtr)[key] + 1 : 1;
			key = "";
		}
		else {
			key += dataset[i];
		}
	}
}

void buildFrequencyMap(std::string dataset, std::unordered_map<std::string, int>* frequencyMapPtr) {
	std::locale loc;
	std::string key = "";
	for (std::string::size_type i = 0; i < dataset.length(); ++i) {
		if (dataset[i] == ' ' && key.length() > 1) {
			// add to frequency if exists, otherwise overwrite
			(*frequencyMapPtr)[key] = (*frequencyMapPtr).count(key) == 1 ? (*frequencyMapPtr)[key] + 1 : 1;
			key = "";
		}
		else {
			key += dataset[i];
		}
	}
}

std::string getRandomWord(std::unordered_map<std::string, int>* frequencyMapPtr, std::default_random_engine* generator) {
	std::string randomWord;
	int numKeys = (*frequencyMapPtr).size();

	std::uniform_int_distribution<int> distribution(0, numKeys);
	int idx = distribution(*generator);

	int i = 0;
	for (auto kv : *frequencyMapPtr) {
		if (i == idx) {
			randomWord = kv.first;
		}
		i++;
	}
	return randomWord;
}

std::string getLastWord(std::string text) {
	size_t idx = text.length() - 1;
	while (idx > 0 && text[idx - 1] != ' ') {
		idx--;
	}
	std::string lastWord = text.substr(idx, text.length());
	return lastWord;
}

std::string getFirstWord(std::string text) {
	size_t idx = 0;
	size_t length = text.length() - 1;
	bool found = text[idx] == ' ';
	while (idx < length && !found) {
		idx++;
		found |= text[idx] == ' ';
	}
	std::string firstWord = text.substr(0, idx);
	return firstWord;
}

std::vector<std::string> getSuggestions(std::unordered_map<std::string, int>* transitionMapPtr, std::unordered_map<std::string, int>* frequencyMapPtr, std::string word) {
	std::vector<std::string> suggestions;
	for (auto kv : *transitionMapPtr) {
		std::string firstWord = getFirstWord(kv.first);
		std::string lastWord = getLastWord(kv.first);
		if (word == firstWord) {
			suggestions.push_back(lastWord);
		}
	}
	if (suggestions.empty()) {
		suggestions.push_back(getRandomWord(frequencyMapPtr, &generator));
	}
	std::sort(suggestions.begin(), suggestions.end());
	while (suggestions.size() > 2) {
		suggestions.pop_back();
	}
	return suggestions;
}

void generatePhrases(std::unordered_map<std::string, int>* transitionMapPtr, std::unordered_map<std::string, int>* frequencyMapPtr, Node* graph, int depth) {
	depth++;
	std::vector<std::string> suggestions = getSuggestions(transitionMapPtr, frequencyMapPtr, getLastWord(graph->sentence));
	for (int i = 0; i < suggestions.size(); i++) {
		Node* childNode = new Node;
		childNode->sentence = graph->sentence + " " + suggestions[i];
		graph->children.push_back(childNode);
		if (depth < LENGTH) {
			generatePhrases(transitionMapPtr, frequencyMapPtr, childNode, depth);
		}
	}
}

void getAllNodes(Node* graph, std::vector<Node*> nodes) {
	std::cout << graph->sentence << std::endl;
	std::vector<Node*> children = graph->children;
	for (int i = 0; i < children.size(); i++) {
		nodes.push_back(children[i]);
		getAllNodes(children[i], nodes);
	}
}

std::string getTopSuggestion(std::string word, std::unordered_map<std::string, int>* transitionMapPtr) {
	std::string suggestion = "";
	int maxScore = 0;
	for (auto kv : *transitionMapPtr) {
		int score = kv.second;
		std::string firstWord = getFirstWord(kv.first);
		std::string lastWord = getLastWord(kv.first);
		if (word == firstWord && score > maxScore) {
			suggestion = lastWord;
			maxScore = score;
		}
	}
	return suggestion;
}


void displaySuggestions(std::vector<std::string> suggestions) {
	std::cout << "Suggestions: " << std::endl;
	for (int i = 0; i < suggestions.size(); i++) {
		std::cout << suggestions[i] << std::endl;
	}
}


int main() {
	std::unordered_map<std::string, int> transitionMap = {};
	std::unordered_map<std::string, int> frequencyMap = {};
 	std::ifstream lexiconFile("C:/Users/n9183167/Documents/ML/dictionary/dictionary.txt");
	std::string dataset = readDatasetAsString(&lexiconFile);
	lexiconFile.close();

	buildTransitionMap(dataset, &transitionMap, 0);
	buildFrequencyMap(dataset, &frequencyMap);
	size_t offset = dataset.find(' ', 0);
	buildTransitionMap(dataset, &transitionMap, offset);

	std::string input;
	std::vector<std::string> suggestions;
	std::cout << "Type something in (minimum 2 words): " << std::endl;
	getline(std::cin, input);
	std::string lastWord = getLastWord(input);

	Node* head = new Node;
	std::vector<Node*> nodes;
	head->sentence = lastWord;

	generatePhrases(&transitionMap, &frequencyMap, head, 0);
	getAllNodes(head, nodes);

	for (int i = 0; i < nodes.size(); i++) {
		std::cout << nodes[i]->sentence << std::endl;
		delete nodes[i];
	}
	
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

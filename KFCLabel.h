#pragma once
#include "imgui.h"
#include <iostream>
class KFCLabel
{
public:
	std::string name;
	std::string className;
	std::string filePath;
	std::string shape;
	ImVec2 mousePosInCanvas1;
	ImVec2 mousePosInCanvas2;
	ImVec2 mousePosInCanvas3;
	ImVec2 mousePosInCanvas4;
	ImVec2 mousePosInCanvas5;
	ImVec2 mousePosInCanvas6;
	ImVec2 mousePosInCanvas7;
	ImVec2 mousePosInCanvas8;

	float polySize = 0;
	int edges = 0;

	KFCLabel(std::string name) {
		this->name = name;
	}
};


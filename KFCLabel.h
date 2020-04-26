#pragma once
#include "imgui.h"
#include <iostream>
class KFCLabel
{
private:
	std::string name;
	std::string className;
	std::string filePath;
	std::string shape;
	ImVec2 mousePosInCanvas1;
	ImVec2 mousePosInCanvas2;
	ImVec2 mousePosInCanvas3;
	ImVec2 mousePosInCanvas4;
	float polySize = 0;
	int edges = 0;
public:
	KFCLabel(std::string name) {
		this->name = name;
	}

	std::string getName() { return name; }
	void setName(std::string newName) { name = newName; }

	std::string getClassName() { return className; }
	void setClassName(std::string newClassName) { className = newClassName; }

	std::string getFilePath() { return filePath; }
	void setFilePath(std::string newFilePath) { filePath = newFilePath; }

	std::string getShape() { return shape; }
	void setShape(std::string newShape) { shape = newShape; }

	float getPolySize() { return polySize; }
	void setPolySize(float newPolySize) { polySize = newPolySize; }

	int getEdges() { return edges; }
	void setEdges(int newEdges) { edges = newEdges; }

	ImVec2 getMousePosition1() { return mousePosInCanvas1; }
	void setMousePosition1(ImVec2 newPosition) { mousePosInCanvas1 = newPosition; }

	ImVec2 getMousePosition2() { return mousePosInCanvas2; }
	void setMousePosition2(ImVec2 newPosition) { mousePosInCanvas2 = newPosition; }

	ImVec2 getMousePosition3() { return mousePosInCanvas3; }
	void setMousePosition3(ImVec2 newPosition) { mousePosInCanvas3 = newPosition; }

	ImVec2 getMousePosition4() { return mousePosInCanvas4; }
	void setMousePosition4(ImVec2 newPosition) { mousePosInCanvas4 = newPosition; }

};


#pragma once

#include <cstdio>

class OutputFileHandle {
public:
	OutputFileHandle(std::string file_name)
	{
		fp = std::fopen(file_name.c_str(), "w");
		if (!fp) { fmt::print("File opening failed"); }
	}

	~OutputFileHandle() { std::fclose(fp); }

	FILE *FP() const { return fp; };

private:
	FILE *fp = nullptr;
};

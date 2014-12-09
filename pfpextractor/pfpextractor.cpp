#include <Windows.h>
#include <direct.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

typedef int8_t n8;
typedef int16_t n16;
typedef int32_t n32;
typedef int64_t n64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

template<typename T>
std::vector<T> FSSplitOf(const T& a_sString, const T& a_sSeparatorSet)
{
	std::vector<T> vString;
	for (auto it = a_sString.begin(); it != a_sString.end(); ++it)
	{
		auto itPos = find_first_of(it, a_sString.end(), a_sSeparatorSet.begin(), a_sSeparatorSet.end());
		if (itPos != a_sString.end())
		{
			vString.push_back(a_sString.substr(it - a_sString.begin(), itPos - it));
			it = itPos;
		}
		else
		{
			vString.push_back(a_sString.substr(it - a_sString.begin()));
			break;
		}
	}
	return vString;
}

void FCopyFile(FILE* a_fpDest, FILE* a_fpSrc, n64 a_nSrcOffset, n64 a_nSize)
{
	const n64 nBufferSize = 0x100000;
	u8* pBuffer = new u8[nBufferSize];
	_fseeki64(a_fpSrc, a_nSrcOffset, SEEK_SET);
	while (a_nSize > 0)
	{
		n64 nSize = a_nSize > nBufferSize ? nBufferSize : a_nSize;
		fread(pBuffer, 1, static_cast<size_t>(nSize), a_fpSrc);
		fwrite(pBuffer, 1, static_cast<size_t>(nSize), a_fpDest);
		a_nSize -= nSize;
	}
	delete[] pBuffer;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		return 1;
	}
	FILE* fp = fopen(argv[1], "rb");
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 4, SEEK_SET);
	u32 count = 0;
	fread(&count, 4, 1, fp);
	u32 offset = 8;
	char name[257] = {};
	for (u32 i = 0; i < count; i++)
	{
		fseek(fp, offset, SEEK_SET);
		u8 length = fgetc(fp);
		fread(name, 1, length, fp);
		name[length] = '\0';
		string path = argv[2];
		path += "/";
		path += name;
		vector<string> dirs = FSSplitOf<string>(path, "/\\");
		string dirPath = dirs[0];
		_mkdir(dirPath.c_str());
		for (int j = 1; j < static_cast<int>(dirs.size()) - 1; j++)
		{
			dirPath += "/";
			dirPath += dirs[j];
			_mkdir(dirPath.c_str());
		}
		u32 fileOffset = 0;
		fread(&fileOffset, 4, 1, fp);
		u32 fileSize = 0;
		fread(&fileSize, 4, 1, fp);
		FILE* fp2 = fopen(path.c_str(), "wb");
		if (fp2 == nullptr)
		{
			fclose(fp);
			return 1;
		}
		FCopyFile(fp2, fp, fileOffset, fileSize);
		fclose(fp2);
		offset += 1 + length + 8;
	}
	fclose(fp);
	return 0;
}

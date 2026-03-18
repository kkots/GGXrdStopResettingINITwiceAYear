#include "pch.h"
#include "GGXrdStopResettingINITwiceAYearCommon.h"
// The purpose of this program is to patch GuiltyGearXrd.exe to add instructions to it so that
// it loads the ggxrd_hitbox_overlay.dll on startup automatically
#include <iostream>
#include <string>
#ifndef FOR_LINUX
#include "InjectorCommonOut.h"
#include <commdlg.h>
#include <io.h>     // for _get_osfhandle
#else
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <climits>
#endif
#include <vector>

#ifndef FOR_LINUX
InjectorCommonOut outputObject;
#define CrossPlatformString std::wstring
#define CrossPlatformChar wchar_t
#define CrossPlatformPerror _wperror
#define CrossPlatformText(txt) L##txt
#define CrossPlatformCout outputObject
#define CrossPlatformNumberToString std::to_wstring
#define CrossPlatformStringCopy wcscpy_s
#define SEPARATOR_CHAR L'\\'
#define SEPARATOR_CHAR_AS_STRING L"\\"
#define SPRINTF swprintf_s
#define SPRINTF_NARROW sprintf_s
#define CrossPlatformStringLen wcslen
#define CrossPlatformStringFmt L"%ls"
#define CrossPlatformStringCompareCaseInsensitive _stricmp
#define CrossPlatformPathCompare wcscmp
#else
#define CrossPlatformString std::string
#define CrossPlatformChar char
#define CrossPlatformPerror perror
#define CrossPlatformText(txt) txt
#define CrossPlatformCout std::cout
#define CrossPlatformNumberToString std::to_string
#define strcpy_s(dst, dst_size, src) strcpy(dst, src)
#define CrossPlatformStringCopy strcpy_s
#ifndef MAX_PATH
#define MAX_PATH 10000
#endif
#define SEPARATOR_CHAR '/'
#define SEPARATOR_CHAR_AS_STRING "/"
typedef unsigned int DWORD;
#define SPRINTF sprintf
#define SPRINTF_NARROW sprintf
#define CrossPlatformStringLen strlen
#define CrossPlatformStringFmt "%s"
#define CrossPlatformStringCompareCaseInsensitive stricmp
#define CrossPlatformPathCompare strcmp

int stricmp(const char* left, const char* right) {
	while (true) {
		char leftLower = tolower(*left);
		char rightLower = tolower(*right);
		if (leftLower == rightLower) {
			if (leftLower == 0) return 0;
			++left;
			++right;
			continue;
		}
		return leftLower - rightLower;
	}
}

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    WORD   e_magic;                     // Magic number
    WORD   e_cblp;                      // Bytes on last page of file
    WORD   e_cp;                        // Pages in file
    WORD   e_crlc;                      // Relocations
    WORD   e_cparhdr;                   // Size of header in paragraphs
    WORD   e_minalloc;                  // Minimum extra paragraphs needed
    WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    WORD   e_ss;                        // Initial (relative) SS value
    WORD   e_sp;                        // Initial SP value
    WORD   e_csum;                      // Checksum
    WORD   e_ip;                        // Initial IP value
    WORD   e_cs;                        // Initial (relative) CS value
    WORD   e_lfarlc;                    // File address of relocation table
    WORD   e_ovno;                      // Overlay number
    WORD   e_res[4];                    // Reserved words
    WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    WORD   e_oeminfo;                   // OEM information; e_oemid specific
    WORD   e_res2[10];                  // Reserved words
    LONG   e_lfanew;                    // File address of new exe header
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
  
typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;

    //
    // NT additional fields.
    //

    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef PIMAGE_NT_HEADERS32                 PIMAGE_NT_HEADERS;

typedef IMAGE_NT_HEADERS32     IMAGE_NT_HEADERS;

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((ULONG_PTR)(ntheader) +                                            \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))
    
#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_HIGH 1
#define IMAGE_REL_BASED_LOW 2
#define IMAGE_REL_BASED_HIGHLOW 3
#define IMAGE_REL_BASED_HIGHADJ 4
#define IMAGE_REL_BASED_DIR64 10

#define FIELD_OFFSET offsetof
    
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5

#define FILE_ATTRIBUTE_NORMAL 0x80
#define OPEN_EXISTING 3
#define FILE_SHARE_READ 1
#define GENERIC_READ 0x80000000

typedef unsigned long long ULONGLONG;
typedef unsigned long ULONG;
#endif

#if defined( _WIN64 )
typedef PIMAGE_NT_HEADERS32 nthdr;
#undef IMAGE_FIRST_SECTION
#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((ULONG_PTR)(ntheader) +                                            \
     FIELD_OFFSET( IMAGE_NT_HEADERS32, OptionalHeader ) +                 \
     ((ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))
#else
typedef PIMAGE_NT_HEADERS nthdr;
#endif
nthdr pNtHeader = nullptr;
BYTE* fileBase = nullptr;

#ifndef FOR_LINUX
extern void GetLine(CrossPlatformString& line);
#else
void GetLine(std::string& line) {
	std::getline(std::cin, line);
}
void OutputStringA(const char* str) {
	std::cout << str;
}
void OutputStringW(const wchar_t* str) {
	std::wcout << str;
}
#endif

#ifdef FOR_LINUX
void copyFileLinux(const std::string& pathSource, const std::string& pathDestination);
#endif

bool crossPlatformFileCopy(const CrossPlatformString& dst, const CrossPlatformString& src,
		const CrossPlatformChar* successMsg,
		const CrossPlatformChar* errorMsg) {
    bool success = true;
    #ifndef FOR_LINUX
    if (!CopyFileW(src.c_str(), dst.c_str(), true)) {
        CrossPlatformCout << errorMsg;
    	CrossPlatformString ignoreLine;
    	GetLine(ignoreLine);
    	success = false;
    }
    #else
    copyFileLinux(src, dst);
    std::cout << "Backup copy created successfully.\n";
    #endif
    CrossPlatformCout << successMsg;
    return success;
}

char exeName[] = "\x3d\x6b\x5f\x62\x6a\x6f\x3d\x5b\x57\x68\x4e\x68\x5a\x24\x5b\x6e\x5b\xf6";

int findLast(const CrossPlatformString& str, CrossPlatformChar character) {
    if (str.empty()) return -1;
    auto it = str.cend();
    --it;
    while (true) {
        if (*it == character) return (int)(it - str.cbegin());
        if (it == str.cbegin()) return -1;
        --it;
    }
    return -1;
}

// Does not include trailing slash
CrossPlatformString getParentDir(const CrossPlatformString& path) {
    CrossPlatformString result;
    int lastSlashPos = findLast(path, SEPARATOR_CHAR);
    if (lastSlashPos == -1) return result;
    result.insert(result.begin(), path.cbegin(), path.cbegin() + lastSlashPos);
    return result;
}

CrossPlatformString getFileName(const CrossPlatformString& path) {
    CrossPlatformString result;
    int lastSlashPos = findLast(path, SEPARATOR_CHAR);
    if (lastSlashPos == -1) return path;
    result.insert(result.begin(), path.cbegin() + lastSlashPos + 1, path.cend());
    return result;
}

bool fileExists(const CrossPlatformChar* path) {
    #ifndef FOR_LINUX
    DWORD fileAtrib = GetFileAttributesW(path);
    if (fileAtrib == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return true;
    #else
    FILE* file = fopen(path, "rb");
    if (!file) return false;
    fclose(file);
    return true;
    #endif
}

bool fileExists(const CrossPlatformString& path) {
	return fileExists(path.c_str());
}

#if defined(FOR_LINUX) || !defined(_DEBUG)
#define byteSpecificationError
#else
extern HWND mainWindow;
#define byteSpecificationError { \
    OutputDebugStringA("Wrong byte specification: "); \
    OutputDebugStringA(byteSpecification); \
    OutputDebugStringA("\n"); \
	return numOfTriangularChars; \
}
#endif

// byteSpecification is of the format "00 8f 1e ??". ?? means unknown byte.
// Converts a "00 8f 1e ??" string into two vectors:
// sig vector will contain bytes '00 8f 1e' for the first 3 bytes and 00 for every ?? byte.
// sig vector will be terminated with an extra 0 byte.
// mask vector will contain an 'x' character for every non-?? byte and a '?' character for every ?? byte.
// mask vector will be terminated with an extra 0 byte.
// Can additionally provide an size_t* position argument. If the byteSpecification contains a ">" character, position will store the offset of that byte.
// If multiple ">" characters are present, position must be an array able to hold all positions, and positionLength specifies the length of the array.
// If positionLength is 0, it is assumed the array is large enough to hold all > positions.
// Returns the number of > characters.
size_t byteSpecificationToSigMask(const char* byteSpecification, std::vector<char>& sig, std::vector<char>& mask, size_t* position = nullptr, size_t positionLength = 0) {
	if (position && positionLength == 0) positionLength = UINT_MAX;
	size_t numOfTriangularChars = 0;
	sig.clear();
	mask.clear();
	unsigned long long accumulatedNibbles = 0;
	int nibbleCount = 0;
	bool nibblesUnknown = false;
	const char* byteSpecificationPtr = byteSpecification;
	bool nibbleError = false;
	const char* nibbleSequenceStart = byteSpecification;
	while (true) {
		char currentChar = *byteSpecificationPtr;
		if (currentChar == '>') {
			if (position && numOfTriangularChars < positionLength) {
				*position = sig.size();
				++position;
			}
			++numOfTriangularChars;
			nibbleSequenceStart = byteSpecificationPtr + 1;
		} else if (currentChar == '(') {
			nibbleCount = 0;
			nibbleError = false;
			nibblesUnknown = false;
			accumulatedNibbles = 0;
			if (byteSpecificationPtr <= nibbleSequenceStart) {
				byteSpecificationError
			}
			const char* moduleNameEnd = byteSpecificationPtr;
			++byteSpecificationPtr;
			bool parseOk = true;
			#define skipWhitespace \
				while (*byteSpecificationPtr != '\0' && *byteSpecificationPtr <= 32) { \
					++byteSpecificationPtr; \
				}
			#define checkQuestionMarks \
				if (parseOk) { \
					if (strncmp(byteSpecificationPtr, "??", 2) != 0) { \
						parseOk = false; \
					} else { \
						byteSpecificationPtr += 2; \
					} \
				}
			#define checkWhitespace \
				if (parseOk) { \
					if (*byteSpecificationPtr == '\0' || *byteSpecificationPtr > 32) { \
						parseOk = false; \
					} else { \
						while (*byteSpecificationPtr != '\0' && *byteSpecificationPtr <= 32) { \
							++byteSpecificationPtr; \
						} \
					} \
				}
			skipWhitespace
			checkQuestionMarks
			checkWhitespace
			checkQuestionMarks
			checkWhitespace
			checkQuestionMarks
			checkWhitespace
			checkQuestionMarks
			skipWhitespace
			#undef skipWhitespace
			#undef checkQuestionMarks
			#undef checkWhitespace
			if (*byteSpecificationPtr != ')') {
				parseOk = false;
			}
			if (!parseOk) {
				byteSpecificationError
			}
		} else if (currentChar != ' ' && currentChar != '\0') {
			char currentNibble = 0;
			if (currentChar >= '0' && currentChar <= '9' && !nibblesUnknown) {
				currentNibble = currentChar - '0';
			} else if (currentChar >= 'a' && currentChar <= 'f' && !nibblesUnknown) {
				currentNibble = currentChar - 'a' + 10;
			} else if (currentChar >= 'A' && currentChar <= 'F' && !nibblesUnknown) {
				currentNibble = currentChar - 'A' + 10;
			} else if (currentChar == '?' && (nibbleCount == 0 || nibblesUnknown)) {
				nibblesUnknown = true;
			} else {
				nibbleError = true;
			}
			accumulatedNibbles = (accumulatedNibbles << 4) | currentNibble;
			++nibbleCount;
			if (nibbleCount > 16) {
				nibbleError = true;
			}
		} else {
			if (nibbleCount) {
				if (nibbleError) {
					byteSpecificationError
				}
				do {
					if (!nibblesUnknown) {
						sig.push_back(accumulatedNibbles & 0xff);
						mask.push_back('x');
						accumulatedNibbles >>= 8;
					} else {
						sig.push_back(0);
						mask.push_back('?');
					}
					nibbleCount -= 2;
				} while (nibbleCount > 0);
				nibbleCount = 0;
				nibblesUnknown = false;
			}
			if (currentChar == '\0') {
				break;
			}
			nibbleSequenceStart = byteSpecificationPtr + 1;
		}
		++byteSpecificationPtr;
	}
	sig.push_back('\0');
	mask.push_back('\0');
	#undef byteSpecificationError
	return numOfTriangularChars;
}

struct Sig {
    std::vector<char> sig;
    std::vector<char> mask;
    std::vector<size_t> positions;
    Sig(const char* byteSpecification) {
        size_t numChar = 0;
        for (const char* ptr = byteSpecification; *ptr != '\0'; ++ptr) {
            if (*ptr == '>') ++numChar;
        }
        if (numChar) {
            positions.resize(numChar);
        }
        byteSpecificationToSigMask(byteSpecification, sig, mask, positions.data(), numChar);
    }
};

struct SectionStartEnd {
    DWORD va = 0;
    DWORD vaEnd = 0;
    BYTE* ptr = nullptr;
    BYTE* ptrEnd = nullptr;
    PIMAGE_SECTION_HEADER section = nullptr;
    
    SectionStartEnd(const char* sectionName) {
        PIMAGE_SECTION_HEADER sectionSeek = IMAGE_FIRST_SECTION(pNtHeader);
		for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; ++i) {
			if (strncmp((const char*)sectionSeek->Name, sectionName, 8) == 0) {
				ptr = fileBase + sectionSeek->PointerToRawData;
				ptrEnd = ptr + sectionSeek->SizeOfRawData;
				va = (ULONG_PTR)pNtHeader->OptionalHeader.ImageBase + sectionSeek->VirtualAddress;
				vaEnd = va + sectionSeek->Misc.VirtualSize;
				section = sectionSeek;
				return;
			}
			++sectionSeek;
		}
    }
};

int sigscan(const char* start, const char* end, const char* sig, const char* mask) {
    const char* startPtr = start;
    const size_t maskLen = strlen(mask);
    const size_t seekLength = end - start - maskLen + 1;
    for (size_t seekCounter = seekLength; seekCounter != 0; --seekCounter) {
        const char* stringPtr = startPtr;

        const char* sigPtr = sig;
        for (const char* maskPtr = mask; true; ++maskPtr) {
            const char maskPtrChar = *maskPtr;
            if (maskPtrChar != '?') {
                if (maskPtrChar == '\0') return (int)(startPtr - start);
                if (*sigPtr != *stringPtr) break;
            }
            ++sigPtr;
            ++stringPtr;
        }
        ++startPtr;
    }
    return -1;
}
inline int sigscan(const char* start, const char* end, const Sig& sig) {
    return sigscan(start, end, sig.sig.data(), sig.mask.data());
}

inline int sigscan(const BYTE* start, const BYTE* end, const char* sig, const char* mask) {
	return sigscan((const char*)start, (const char*)end, sig, mask);
}

inline int sigscan(const BYTE* start, const BYTE* end, const Sig& sig) {
	return sigscan((const char*)start, (const char*)end, sig.sig.data(), sig.mask.data());
}

inline int sigscan(const SectionStartEnd& section, const char* sig, const char* mask) {
    return sigscan(section.ptr, section.ptrEnd, sig, mask);
}

inline int sigscan(const SectionStartEnd& section, const Sig& sig) {
    return sigscan(section.ptr, section.ptrEnd, sig.sig.data(), sig.mask.data());
}

DWORD sigscan(FILE* file, const char* sig, const char* mask) {
    class DoThisWhenExiting {
    public:
        DoThisWhenExiting(FILE* file, size_t originalPos) : file(file), originalPos(originalPos) { }
        ~DoThisWhenExiting() {
            fseek(file, (long)originalPos, SEEK_SET);
        }
        FILE* file = nullptr;
        size_t originalPos = 0;
    } doThisWhenExiting(file, ftell(file));

    fseek(file, 0, SEEK_SET);
    char buffer[2048] = { '\0' };
    bool isFirst = true;
    size_t maskLen = strlen(mask);
    DWORD currentFilePosition = 0;

    while (true) {
        size_t readBytes;
        if (isFirst) {
            readBytes = fread(buffer, 1, 1024, file);
        } else {
            readBytes = fread(buffer + 1024, 1, 1024, file);
        }
        if (readBytes == 0) {
            if (ferror(file)) {
                CrossPlatformPerror(CrossPlatformText("Error reading from file"));
            }
            // assume it's feof
            break;
        }

        if (isFirst) {
            int pos = sigscan(buffer, buffer + readBytes, sig, mask);
            if (pos != -1) {
                return (DWORD)pos;
            }
        } else {
            int pos = sigscan(buffer + 1024 - maskLen + 1, buffer + 1024 + readBytes, sig, mask);
            if (pos != -1) {
                return currentFilePosition + (DWORD)pos - (DWORD)maskLen + 1;
            }
        }

        if (readBytes < 1024) {
            if (ferror(file)) {
                CrossPlatformPerror(CrossPlatformText("Error reading from file"));
            }
            // assume it's feof
            break;
        }

        if (!isFirst) {
            memcpy(buffer, buffer + 1024, 1024);
        }
        isFirst = false;

        currentFilePosition += 1024;
    }
    // we didn't open the file, so we're not the ones who should close it
    return 0;
}
inline DWORD sigscan(FILE* file, const Sig& sig) {
    return sigscan(file, sig.sig.data(), sig.mask.data());
}

bool readWholeFile(FILE* file, std::vector<char>& wholeFile) {
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    wholeFile.resize(fileSize);
    char* wholeFilePtr = wholeFile.data();
    size_t readBytesTotal = 0;
    while (true) {
        size_t sizeToRead = 1024;
        if (fileSize - readBytesTotal < 1024) sizeToRead = fileSize - readBytesTotal;
        if (sizeToRead == 0) break;
        size_t readBytes = fread(wholeFilePtr, 1, sizeToRead, file);
        if (readBytes != sizeToRead) {
            if (ferror(file)) {
                CrossPlatformPerror(CrossPlatformText("Error reading file"));
                return false;
            }
            // assume feof
            break;
        }
        wholeFilePtr += 1024;
        readBytesTotal += 1024;
    }
    return true;
}

std::string repeatCharNTimes(char charToRepeat, int times) {
    std::string result;
    result.resize(times, charToRepeat);
    return result;
}

int calculateRelativeCall(DWORD callInstructionAddress, DWORD calledAddress) {
    return (int)calledAddress - (int)(callInstructionAddress + 5);
}

DWORD followRelativeCall(DWORD callInstructionAddress, const char* callInstructionAddressInRam) {
    int offset = *(int*)(callInstructionAddressInRam + 1);
    return callInstructionAddress + 5 + offset;
}

DWORD rvaToRaw(DWORD rva) {
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; ++i) {
		if (rva >= section->VirtualAddress && (
				i == pNtHeader->FileHeader.NumberOfSections - 1
					? true
					: rva < (section + 1)->VirtualAddress
			)) {
			return rva - section->VirtualAddress + section->PointerToRawData;
		}
		++section;
	}
	return 0;
}

DWORD rawToRva(DWORD raw) {
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	PIMAGE_SECTION_HEADER maxSection = section;
	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; ++i) {
		if (raw >= section->PointerToRawData && raw < section->PointerToRawData + section->SizeOfRawData) {
			return raw - section->PointerToRawData + section->VirtualAddress;
		}
		if (section->VirtualAddress > maxSection->VirtualAddress) maxSection = section;
		section++;
	}
	return raw - maxSection->PointerToRawData + maxSection->VirtualAddress;
}

inline DWORD vaToRva(DWORD va) { return va - (DWORD)pNtHeader->OptionalHeader.ImageBase; }
inline DWORD rvaToVa(DWORD rva) { return rva + (DWORD)pNtHeader->OptionalHeader.ImageBase; }
DWORD vaToRaw(DWORD va) { return rvaToRaw(vaToRva(va)); }
DWORD rawToVa(DWORD raw) { return rvaToVa(rawToRva(raw)); }
inline DWORD ptrToRaw(BYTE* ptr) { return (DWORD)(ptr - fileBase); }
DWORD ptrToRva(BYTE* ptr) { return rawToRva(ptrToRaw(ptr)); }
DWORD ptrToVa(BYTE* ptr) { return rawToVa(ptrToRaw(ptr)); }
inline BYTE* rawToPtr(DWORD raw) { return raw + fileBase; }
BYTE* rvaToPtr(DWORD rva) { return rawToPtr(rvaToRaw(rva)); }
BYTE* vaToPtr(DWORD va) { return rawToPtr(vaToRaw(va)); }

struct FoundReloc {
	char type;  // see macros starting with IMAGE_REL_BASED_
	DWORD regionVa;  // position of the place that the reloc is patching
	DWORD relocVa;  // position of the reloc entry itself
	BYTE* ptr;  // points to the page base member of the block
	bool ptrIsLast;  // block is the last in the table
};

struct FoundRelocBlock {
	BYTE* ptr;  // points to the page base member of the block
	DWORD pageBaseVa;  // page base of all patches that the reloc is responsible for
	DWORD relocVa;  // position of the reloc block itself. Points to the page base member of the block
	DWORD size;  // size of the entire block, including the page base and block size and all entries
	bool isLast;  // this block is the last in the table
};

struct RelocBlockIterator {
	
	BYTE* const relocTableOrig;
	const DWORD relocTableVa;
	const DWORD imageBase;
	DWORD relocTableSize;  // remaining size
	BYTE* relocTableNext;
	
	RelocBlockIterator(BYTE* relocTable, DWORD relocTableVa, DWORD relocTableSize, DWORD imageBase)
		:
		relocTableOrig(relocTable),
		relocTableVa(relocTableVa),
		imageBase(imageBase),
		relocTableSize(relocTableSize),
		relocTableNext(relocTable) { }
		
	bool getNext(FoundRelocBlock& block) {
		
		if (relocTableSize == 0) return false;
		
		BYTE* relocTable = relocTableNext;
		
		DWORD pageBaseRva = *(DWORD*)relocTable;
		DWORD pageBaseVa = rvaToVa(pageBaseRva);
		DWORD blockSize = *(DWORD*)(relocTable + 4);
		
		relocTableNext += blockSize;
		
		if (relocTableSize <= blockSize) relocTableSize = 0;
		else relocTableSize -= blockSize;
		
		block.ptr = relocTable;
		block.pageBaseVa = pageBaseVa;
		block.relocVa = relocTableVa + (DWORD)(uintptr_t)(relocTable - relocTableOrig);
		block.size = blockSize;
		block.isLast = relocTableSize == 0;
		return true;
	}
};

struct RelocEntryIterator {
	const DWORD blockVa;
	BYTE* const blockStart;
	const BYTE* ptr;  // the pointer to the next entry
	DWORD blockSize;  // the remaining, unconsumed size of the block (we don't actually modify any data, so maybe consume is not the right word)
	const DWORD pageBaseVa;
	const bool blockIsLast;
	
	/// <param name="ptr">The address of the start of the whole reloc block, NOT the first member.</param>
	/// <param name="blockSize">The size of the entire reloc block, in bytes, including the page base member and the block size member.</param>
	/// <param name="pageBaseVa">The page base of the reloc block.</param>
	/// <param name="blockVa">The Virtual Address where the reloc block itself is located. Must point to the page base member of the block.</param>
	RelocEntryIterator(BYTE* ptr, DWORD blockSize, DWORD pageBaseVa, DWORD blockVa, bool blockIsLast)
		:
		ptr(ptr + 8),
		blockSize(blockSize <= 8 ? 0 : blockSize - 8),
		pageBaseVa(pageBaseVa),
		blockStart(ptr),
		blockVa(blockVa),
		blockIsLast(blockIsLast) { }
		
	RelocEntryIterator(const FoundRelocBlock& block) : RelocEntryIterator(
		block.ptr, block.size, block.pageBaseVa, block.relocVa, block.isLast) { }
	
	bool getNext(FoundReloc& reloc) {
		if (blockSize == 0) return false;
		
		unsigned short entry = *(unsigned short*)ptr;
		char blockType = (entry & 0xF000) >> 12;
		
		DWORD entrySize = blockType == IMAGE_REL_BASED_HIGHADJ ? 4 : 2;
		
		reloc.type = blockType;
		reloc.regionVa = pageBaseVa | (entry & 0xFFF);
		reloc.relocVa = blockVa + (DWORD)(uintptr_t)(ptr - blockStart);
		reloc.ptr = blockStart;
		reloc.ptrIsLast = blockIsLast;
		
		if (blockSize <= entrySize) blockSize = 0;
		else blockSize -= entrySize;
		
		ptr += entrySize;
		
		return true;
	}
	
};

struct RelocTable {
	
	BYTE* relocTable;  // the pointer pointing to the reloc table's start. Typically that'd be the page base member of the first block
	DWORD va;  // Virtual Address of the reloc table's start
	DWORD raw;  // the raw position of the reloc table's start
	DWORD size;  // the size of the whole reloc table
	int sizeWhereRaw;  // the raw location of the size of the whole reloc table
	DWORD imageBase;  // the Virtual Address of the base of the image
	
	// Finds the file position of the start of the reloc table and its size
	void findRelocTable() {
		
	    IMAGE_DATA_DIRECTORY& relocDir = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	    DWORD relocRva = relocDir.VirtualAddress;
	    DWORD* relocSizePtr = &relocDir.Size;
	    sizeWhereRaw = (int)(uintptr_t)((BYTE*)relocSizePtr - fileBase);
	    
	    va = rvaToVa(relocRva);
	    raw = rvaToRaw(relocRva);
	    relocTable = fileBase + raw;
	    size = *relocSizePtr;
	    imageBase = (DWORD)pNtHeader->OptionalHeader.ImageBase;
	}
	
	// region specified in Virtual Address space
	std::vector<FoundReloc> findRelocsInRegion(DWORD regionStart, DWORD regionEnd) const {
		std::vector<FoundReloc> result;
		
		RelocBlockIterator blockIterator(relocTable, va, size, imageBase);
		
		FoundRelocBlock block;
		while (blockIterator.getNext(block)) {
			if (block.pageBaseVa >= regionEnd || (block.pageBaseVa | 0xFFF) + 8 < regionStart) {
				continue;
			}
			RelocEntryIterator entryIterator(block);
			FoundReloc reloc;
			while (entryIterator.getNext(reloc)) {
				if (reloc.type == IMAGE_REL_BASED_ABSOLUTE) continue;
				DWORD patchVa = reloc.regionVa;
				DWORD patchSize = 4;
				
				switch (reloc.type) {
					case IMAGE_REL_BASED_HIGH: patchVa += 2; patchSize = 2; break;
					case IMAGE_REL_BASED_LOW: patchSize = 2; break;
					case IMAGE_REL_BASED_HIGHLOW: break;
					case IMAGE_REL_BASED_HIGHADJ:
						patchSize = 2;
						break;
					case IMAGE_REL_BASED_DIR64: patchSize = 8; break;
				}
				
				if (patchVa >= regionEnd || patchVa + patchSize < regionStart) continue;
				
				result.push_back(reloc);
			}
		}
		return result;
	}
	inline std::vector<FoundReloc> findRelocsInRegion(BYTE* regionStart, BYTE* regionEnd) const {
		return findRelocsInRegion(ptrToVa(regionStart), ptrToVa(regionEnd));
	}
	
	FoundRelocBlock findLastRelocBlock() const {
		
		RelocBlockIterator blockIterator(relocTable, va, size, imageBase);
		
		FoundRelocBlock block;
		while (blockIterator.getNext(block));
		return block;
	}
	
	// returns empty, unused entries that could potentially be reused for the desired vaToPatch
	std::vector<FoundReloc> findReusableRelocEntries(DWORD vaToPatch) const {
		std::vector<FoundReloc> result;
		
		RelocBlockIterator blockIterator(relocTable, va, size, imageBase);
		
		FoundRelocBlock block;
		while (blockIterator.getNext(block)) {
			
			if (!(
				block.pageBaseVa <= vaToPatch && vaToPatch <= (block.pageBaseVa | 0xFFF)
			)) {
				continue;
			}
			
			RelocEntryIterator entryIterator(block);
			FoundReloc reloc;
			while (entryIterator.getNext(reloc)) {
				if (reloc.type == IMAGE_REL_BASED_ABSOLUTE) {
					result.push_back(reloc);
				}
			}
		}
		
		return result;
	}
	
	template<typename T>
	void write(FILE* file, int filePosition, T value) {
		fseek(file, filePosition, SEEK_SET);
		fwrite(&value, sizeof (T), 1, file);
	}
	
	template<typename T, size_t size>
	void write(FILE* file, int filePosition, T (&value)[size]) {
		fseek(file, filePosition, SEEK_SET);
		fwrite(value, sizeof (T), size, file);
	}
	
	// Resize the whole reloc table.
	// Writes both into the file and into the size member.
	void increaseSizeBy(FILE* file, DWORD n) {
		
	    DWORD relocSizeRoundUp = (size + 3) & ~3;
		size = relocSizeRoundUp + n;  // reloc size includes the page base VA and the size of the reloc table entry and of all the entries
		// but I still have to round it up to 32 bits
		
		write(file, sizeWhereRaw, size);
		
	}
	void decreaseSizeBy(FILE* file, DWORD n) {
		
		size -= n;  // reloc size includes the page base VA and the size of the reloc table entry and of all the entries
		// but I still have to round it up to 32 bits
		
		write(file, sizeWhereRaw, size);
		
	}
	
	// Try to:
	// 1) Reuse an existing 0000 entry that has a page base from which we can reach the target;
	// 2) Try to expand the last block if the target is reachable from its page base;
	// 3) Add a new block to the end of the table with that one entry.
	void addEntry(FILE* file, DWORD vaToRelocate, char type) {
		unsigned short relocEntry = ((unsigned short)type << 12) | (vaToRva(vaToRelocate) & 0xFFF);
		
		std::vector<FoundReloc> reusableEntries = findReusableRelocEntries(vaToRelocate);
		if (!reusableEntries.empty()) {
			const FoundReloc& firstReloc = reusableEntries.front();
			write(file, firstReloc.relocVa - va + raw, relocEntry);
			*(unsigned short*)(relocTable + firstReloc.relocVa - va) = relocEntry;
			return;
		}
		
		const FoundRelocBlock lastBlock = findLastRelocBlock();
		if (lastBlock.pageBaseVa <= vaToRelocate && vaToRelocate <= (lastBlock.pageBaseVa | 0xFFF)) {
			DWORD newSize = lastBlock.size + 2;
			newSize  = (newSize + 3) & ~3;  // round the size up to 32 bits
			
			DWORD oldTableSize = size;
			DWORD sizeIncrease = newSize - lastBlock.size;
			increaseSizeBy(file, sizeIncrease);
			
			int pos = lastBlock.relocVa - va + raw;
			write(file, pos + 4, newSize);
			*(DWORD*)(relocTable + lastBlock.relocVa - va + 4) = newSize;
			
			write(file, pos + lastBlock.size, relocEntry);
			unsigned short* newEntryPtr = (unsigned short*)(relocTable + lastBlock.relocVa - va + lastBlock.size);
			*newEntryPtr = relocEntry;
			
			if (sizeIncrease > 2) {
				unsigned short zeros = 0;
				write(file, pos + lastBlock.size + 2, zeros);
				*(newEntryPtr + 1) = zeros;
			}
			
			return;
		}
		
		DWORD oldSize = size;
	    // "Each block must start on a 32-bit boundary." - Microsoft
		DWORD oldSizeRoundedUp = (oldSize + 3) & ~3;
		
		// add a new reloc table with one entry
		increaseSizeBy(file, 12);  // changes 'size'
		
		
		DWORD rvaToRelocate = vaToRva(vaToRelocate);
		DWORD newRelocPageBase = rvaToRelocate & 0xFFFFF000;
		
		DWORD tableData[3];
		tableData[0] = newRelocPageBase;
		tableData[1] = 12;  // page base (4) + size of whole block (4) + one entry (2) + padding to make it 32-bit complete (2)
		tableData[2] = relocEntry;
		
		write(file, raw + oldSizeRoundedUp, tableData);
		memcpy(relocTable + oldSizeRoundedUp, tableData, sizeof tableData);
		
	}
	inline void addEntry(FILE* file, BYTE* ptrToRelocate, char type) {
		addEntry(file, ptrToVa(ptrToRelocate), type);
	}
	
	// fills entry with zeros, diabling it. Does not change the size of either the reloc table or the reloc block
	void removeEntry(FILE* file, const FoundReloc& reloc) {
		unsigned short zeros = 0;
		int relocRaw = reloc.relocVa - va + raw;
		int relocOffset = reloc.relocVa - va;
		write(file, relocRaw, zeros);
		*(unsigned short*)(relocTable + reloc.relocVa - va) = zeros;
		if (reloc.ptrIsLast) {
			DWORD& size = *(
				(DWORD*)reloc.ptr + 1
			);
			int count = 0;
			int lastOffset = size - 2;
			while (lastOffset >= 8) {
				if (*(unsigned short*)(reloc.ptr + lastOffset) != 0) {
					break;
				}
				lastOffset -= 2;
				++count;
			}
			if (lastOffset < 8) {
				// delete whole block
				BYTE zeros[8] { 0 };
				int target = ptrToRaw(reloc.ptr);
				int sizeLeft = size;
				fseek(file, target, SEEK_SET);
				while (sizeLeft > 0) {
					int toWrite = sizeLeft > 8 ? 8 : sizeLeft;
					fwrite(zeros, 1, toWrite, file);
					sizeLeft -= toWrite;
				}
				decreaseSizeBy(file, size);
			} else {
				int shrinkage = 0;
				while (count >= 2) {
					shrinkage += 2;
					count -= 2;
				}
				// shrink block by 4 bytes
				size -= shrinkage;
				decreaseSizeBy(file, shrinkage << 1);
			}
		}
	}
	
};

void trimLeft(std::string& str) {
    if (str.empty()) return;
	auto it = str.begin();
	while (it != str.end() && *it <= 32) ++it;
	str.erase(str.begin(), it);
}

void trimRight(std::string& str) {
    if (str.empty()) return;
    auto it = str.end();
    --it;
    while (true) {
        if (*it > 32) break;
        if (it == str.begin()) {
            str.clear();
            return;
        }
        --it;
    }
    str.resize(it - str.begin() + 1);
}

bool crossPlatformOpenFile(FILE** file, const CrossPlatformString& path) {
	#ifndef FOR_LINUX
	errno_t errorCode = _wfopen_s(file, path.c_str(), CrossPlatformText("r+b"));
	if (errorCode != 0 || !*file) {
		if (errorCode != 0) {
			wchar_t buf[1024];
			_wcserror_s(buf, errorCode);
			CrossPlatformCout << L"Failed to open file: " << buf << L'\n';
		} else {
			CrossPlatformCout << L"Failed to open file.\n";
		}
		if (*file) {
			fclose(*file);
		}
		return false;
	}
	return true;
	#else
	*file = fopen(path.c_str(), "r+b");
	if (!*file) {
		perror("Failed to open file");
		return false;
	}
	return true;
	#endif
}

#ifdef FOR_LINUX
void copyFileLinux(const std::string& pathSource, const std::string& pathDestination) {
    std::ifstream src(pathSource, std::ios::binary);
    std::ofstream dst(pathDestination, std::ios::binary);

    dst << src.rdbuf();
}
#endif

// does not close the file, but truncates it
void overwriteWholeFile(FILE* file, const std::vector<char>& data) {
    fseek(file, 0, SEEK_SET);
    fwrite(data.data(), 1, data.size(), file);
    fflush(file);
    #ifndef FOR_LINUX
    SetEndOfFile((HANDLE)_get_osfhandle(_fileno(file)));
    #else
    ftruncate(fileno(file), data.size());
    #endif
}

/// <summary>
/// Finds the address which holds a pointer to a function with the given name imported from the given DLL,
/// in a given 32-bit portable executable file.
/// For example, searching USER32.DLL, GetKeyState would return a positive value on successful find, and
/// in a running process you'd cast that value to a short (__stdcall**)(int).
/// </summary>
/// <param name="dll">Include ".DLL" in the DLL's name here. Case-insensitive.</param>
/// <param name="function">The name of the function. Case-sensitive.</param>
/// <returns>The file offset which holds a pointer to a function. -1 if not found.</returns>
int findImportedFunction(const char* dll, const char* function) {
	
	// see IMAGE_IMPORT_DESCRIPTOR
	struct ImageImportDescriptor {
		DWORD ImportLookupTableRVA;  // The RVA of the import lookup table. This table contains a name or ordinal for each import. (The name "Characteristics" is used in Winnt.h, but no longer describes this field.)
		DWORD TimeDateStamp;  // The stamp that is set to zero until the image is bound. After the image is bound, this field is set to the time/data stamp of the DLL. LIES, this field is 0 for me at runtime.
		DWORD ForwarderChain;  // The index of the first forwarder reference. 0 for me.
		DWORD NameRVA;  // The address of an ASCII string that contains the name of the DLL. This address is relative to the image base.
		DWORD ImportAddressTableRVA;  // The RVA of the import address table. The contents of this table are identical to the contents of the import lookup table until the image is bound.
	};
	DWORD importsSize = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	const ImageImportDescriptor* importPtrNext = (const ImageImportDescriptor*)(rvaToPtr(
		pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
	));
	for (; importsSize > 0; importsSize -= sizeof (ImageImportDescriptor)) {
		const ImageImportDescriptor* importPtr = importPtrNext++;
		if (!importPtr->ImportLookupTableRVA) break;
		const char* dllName = (const char*)(rvaToPtr(importPtr->NameRVA));
		if (CrossPlatformStringCompareCaseInsensitive(dllName, dll) != 0) continue;
		DWORD* funcPtr = (DWORD*)(rvaToPtr(importPtr->ImportAddressTableRVA));
		DWORD* imageImportByNameRvaPtr = (DWORD*)(rvaToPtr(importPtr->ImportLookupTableRVA));
		struct ImageImportByName {
			short importIndex;  // if you know this index you can use it for lookup. Name is just convenience for programmers.
			char name[1];  // arbitrary length, zero-terminated ASCII string
		};
		do {
			DWORD rva = *imageImportByNameRvaPtr;
			if (!rva) break;
			const ImageImportByName* importByName = (const ImageImportByName*)(rvaToPtr(rva));
			if (strcmp(importByName->name, function) == 0) {
				return (int)((BYTE*)funcPtr - fileBase);
			}
			++funcPtr;
			++imageImportByNameRvaPtr;
		} while (true);
		return -1;
	}
	return -1;
}

int findImportedFunctionAndReport(const char* dllName, const char* functionName) {
    int found = findImportedFunction(dllName, functionName);
	if (found == -1) {
	    CrossPlatformCout << "Failed to find " << functionName << " function.\n";
	    return -1;
	}
	CrossPlatformCout << "Found " << functionName << " function at va:0x"
	    << std::hex << rawToVa(found) << std::dec << '\n';
	return found;
}

void meatOfTheProgram() {
    CrossPlatformString ignoreLine;
	#ifndef FOR_LINUX
    CrossPlatformCout << CrossPlatformText("Please select a path to your ") << exeName << CrossPlatformText(" file that will be patched...\n");
	#else
	CrossPlatformCout << CrossPlatformText("Please type in/paste/drag-n-drop a path to your ") << exeName << CrossPlatformText(" file"
		" (including the file name and extension) that will be patched...\n");
	#endif

    #ifndef FOR_LINUX
    std::vector<WCHAR> szFileBuf(MAX_PATH, L'\0');

    OPENFILENAMEW selectedFiles{ 0 };
    selectedFiles.lStructSize = sizeof(OPENFILENAMEW);
    selectedFiles.hwndOwner = NULL;
    selectedFiles.lpstrFile = szFileBuf.data();
    selectedFiles.nMaxFile = (DWORD)szFileBuf.size();
    // it says "Windows Executable\0*.EXE\0"
	char scramble[] =
		"\x4d\xf6\x5f\xf6\x64\xf6\x5a\xf6\x65\xf6\x6d\xf6\x69\xf6\x16\xf6\x3b\xf6"
		"\x6e\xf6\x5b\xf6\x59\xf6\x6b\xf6\x6a\xf6\x57\xf6\x58\xf6\x62\xf6\x5b\xf6"
		"\xf6\xf6\x20\xf6\x24\xf6\x3b\xf6\x4e\xf6\x3b\xf6\xf6\xf6\xf6\xf6";
	wchar_t filter[(sizeof scramble - 1) / sizeof (wchar_t)];
	int offset = (int)(
		(GetTickCount64() & 0xF000000000000000ULL) >> (63 - 4)
	);
	for (int i = 0; i < sizeof scramble - 1; ++i) {
		char c = scramble[i] + offset + 10;
		((char*)filter)[i] = c;
	}
    selectedFiles.lpstrFilter = filter;
    selectedFiles.nFilterIndex = 1;
    selectedFiles.lpstrFileTitle = NULL;
    selectedFiles.nMaxFileTitle = 0;
    selectedFiles.lpstrInitialDir = NULL;
    selectedFiles.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameW(&selectedFiles)) {
        DWORD errCode = CommDlgExtendedError();
        if (!errCode) {
            std::wcout << "The file selection dialog was closed by the user.\n";
        } else {
            std::wcout << "Error selecting file. Error code: 0x" << std::hex << errCode << std::dec << std::endl;
        }
        return;
    }
    
    std::wstring szFile = szFileBuf.data();
    #else
    int offset;
    std::string szFile;
    GetLine(szFile);
    trimLeft(szFile);
    trimRight(szFile);
    if (!szFile.empty() && (szFile[0] == '\'' || szFile[0] == '"')) {
    	szFile.erase(szFile.begin());
    }
    if (!szFile.empty() && (szFile[szFile.size() - 1] == '\'' || szFile[szFile.size() - 1] == '"')) {
    	szFile.erase(szFile.begin() + (szFile.size() - 1));
    }
    if (szFile.empty()) {
        std::cout << "Empty path provided. Aborting.\n";
        return;
    }
    #endif
    CrossPlatformCout << "Selected file: " << szFile.c_str() << std::endl;

    CrossPlatformString fileName = getFileName(szFile);
    CrossPlatformString parentDir = getParentDir(szFile);
	
	int dotPos = findLast(fileName, L'.');
	if (dotPos == -1) {
		CrossPlatformCout << "Chosen file name does not end with '.EXE': " << fileName.c_str() << std::endl;
		return;
	}
	
	CrossPlatformString fileNameNoDot(fileName.begin(), fileName.begin() + dotPos);
	
	CrossPlatformChar strbuf[1024];
	int len = SPRINTF(strbuf,
			CrossPlatformStringFmt
			SEPARATOR_CHAR_AS_STRING
			CrossPlatformStringFmt
			CrossPlatformText("_backup")
			CrossPlatformStringFmt,
		parentDir.data(), fileNameNoDot.data(), fileName.data() + dotPos);
	
    int backupNameCounter = 1;
    while (fileExists(strbuf)) {
    	len = SPRINTF(strbuf,
    			CrossPlatformStringFmt
    			SEPARATOR_CHAR_AS_STRING
    			CrossPlatformStringFmt
    			CrossPlatformText("_backup%d")
    			CrossPlatformStringFmt,
    		parentDir.data(), fileNameNoDot.data(), backupNameCounter, fileName.data() + dotPos);
        ++backupNameCounter;
    }
    
    CrossPlatformString backupFilePath = strbuf;
    CrossPlatformCout << "Will use backup file path: " << backupFilePath.c_str() << std::endl;
	
    FILE* file = nullptr;
    if (!crossPlatformOpenFile(&file, szFile.c_str())) return;

    std::vector<char> wholeFile;
    if (!readWholeFile(file, wholeFile)) return;
    fileBase = (BYTE*)wholeFile.data();
    
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)fileBase;
	pNtHeader = (nthdr)((PBYTE)fileBase + pDosHeader->e_lfanew);

	SectionStartEnd text{".text"};
	SectionStartEnd rdata{".rdata"};
	
	if (!text.ptr) {
        CrossPlatformCout << "Failed to find .text section\n";
        return;
	}
	
	if (!rdata.ptr) {
        CrossPlatformCout << "Failed to find .rdata section\n";
        return;
	}
	
	Sig FFileManagerWindows_GetFileTimestamp_Sig{
	    "6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 83 ec 60 a1 ?? ?? ?? ?? 33 c4 89 44 24 5c 53 55 56 57"
	    " a1 ?? ?? ?? ?? 33 c4 50 8d 44 24 74 64 a3 00 00 00 00 8b bc 24 84 00 00 00 8b f1 8b 06 8b 50 54 57 8d 4c 24 2c 51 8b ce"
	    " ff d2 33 db 89 5c 24 7c 39 58 04 74 04 8b 00 eb 05"
	    #define MOV_EAX_EMPTYSTRING 0  // MOV EAX,""
	    " >b8 ?? ?? ?? ??"
	    " 8b 16 8b 52 58 50 8d 44 24 20 50 8b ce ff d2 39 58 04 74 04 8b 00 eb 05"
	    " b8 ?? ?? ?? ?? 8b 2d 44 d5 48 01 8d 4c 24 40 51 50 ff d5 83 c4 08 85 c0 75 0a df 6c 24 60 dd 5c 24 14 eb 0e"
	    #define NEGATIVE_1 1  // MOVSD XMM0,qword ptr [DOUBLE_014a0db0], DOUBLE_014a0db0 says 00 00 00 00 00 00 f0 bf, which means -1.0
	    " >f2 0f 10 05 ?? ?? ?? ??"
	    " f2 0f 11 44 24 14 8b 44 24 1c 89 5c 24 24 89 5c 24 20 3b c3 74 0d 50"
	    #define APP_FREE 2  // CALL appFree
	    " >e8 ?? ?? ?? ??"
	    " 83 c4 04 89 5c 24 1c 8b 44 24 28 c7 44 24 7c ff ff ff ff 89 5c 24 30 89 5c 24 2c 3b c3 74 09 50 e8 ?? ?? ?? ??"
	    " 83 c4 04 f2 0f 10 44 24 14 66 0f 2e 05 ?? ?? ?? ?? 9f f6 c4 44 7a 5d 8b 16 8b 52 54 57 8d 44 24 38 50 8b ce"
	    " ff d2 39 58 04 74 04 8b 00 eb 05 b8 ?? ?? ?? ?? 8d 4c 24 40 51 50 ff d5 83 c4 08 85 c0 75 0a df 6c 24 60 dd 5c 24 14 eb 0e"
	    " f2 0f 10 05 ?? ?? ?? ?? f2 0f 11 44 24 14 8b 44 24 34 89 5c 24 3c 89 5c 24 38 3b c3 74 09 50 e8 ?? ?? ?? ??"
	    " 83 c4 04 dd 44 24 14 8b 4c 24 74 64 89 0d 00 00 00 00 59 5f 5e 5d 5b 8b 4c 24 5c 33 cc e8 ?? ?? ?? ?? 83 c4 6c c2 04 00"
    };
	// ConvertToAbsolutePath is 0x54. Takes FString* out and wchar_t* Filename. Returns out
	// ConvertAbsolutePathToUserPath is 0x58. Takes FString* out and wchar_t* AbsolutePath. Returns out
	
	int FFileManagerWindows_GetFileTimestamp = sigscan(text, FFileManagerWindows_GetFileTimestamp_Sig);
	if (FFileManagerWindows_GetFileTimestamp == -1) {
	    if (sigscan(text, Sig{
	        "53 55 56 57"
            " 8b f1"
            " 83 ec ?? 8b 06"
            " 8b 50 54"
            " 8B 7C 24 ??"}) != -1) {
	        CrossPlatformCout << "FFileManagerWindows::GetFileTimestamp function is already patched by someone.\n";
	    } else {
	        CrossPlatformCout << "Failed to find FFileManagerWindows::GetFileTimestamp function.\n";
	    }
	    return;
	}
	FFileManagerWindows_GetFileTimestamp += (int)(text.ptr - fileBase);
	CrossPlatformCout << "Found FFileManagerWindows::GetFileTimestamp function at va:0x"
	    << std::hex << rawToVa(FFileManagerWindows_GetFileTimestamp) << std::dec << '\n';
	
	int CreateFileWOff = findImportedFunctionAndReport("kernel32.dll", "CreateFileW");
	if (CreateFileWOff == -1) return;
	int GetFileTimeOff = findImportedFunctionAndReport("kernel32.dll", "GetFileTime");
	if (GetFileTimeOff == -1) return;
	int CloseHandleOff = findImportedFunctionAndReport("kernel32.dll", "CloseHandle");
	if (CloseHandleOff == -1) return;
	int GetFileAttributesWOff = findImportedFunctionAndReport("kernel32.dll", "GetFileAttributesW");
	if (GetFileAttributesWOff == -1) return;
	
	RelocTable relocTable;
	relocTable.findRelocTable();
	
	// we have everything we need
	
    fclose(file);
    
    if (!crossPlatformFileCopy(backupFilePath, szFile,
	    CrossPlatformText("Backup copy created successfully.\n"),
	    CrossPlatformText("Failed to create a backup copy. Do you want to continue anyway?"
        	" You won't be able to revert the file to the original. Press Enter to agree...\n"))) return;
    
    if (!crossPlatformOpenFile(&file, szFile.c_str())) return;
	
	struct NewCode {
	    std::vector<BYTE> data;
	    void add(const Sig& sig) {
	        if (sig.mask.empty()) return;
	        if (sig.mask.front() == '\0') return;
	        data.insert(data.end(), sig.sig.begin(), sig.sig.end() - 1);
	    }
	    void add(const Sig& sig, DWORD substitute) {
	        if (sig.mask.empty()) return;
	        if (sig.mask.front() == '\0') return;
	        
	        #ifdef _DEBUG
            static bool reportedOnce = false;
	        bool broke = false;
	        #endif
	        int count = 0;
	        std::vector<char>::const_iterator maskIt;
	        std::vector<char>::const_iterator nextIt = sig.mask.begin();
	        for (int charCount = (int)sig.sig.size() - 1; charCount > 0; --charCount) {
	            maskIt = nextIt++;
	            if (*maskIt == '?') {
                    #ifdef _DEBUG
                    if (broke) {
                        if (!reportedOnce) {
                            reportedOnce = true;
                            MessageBoxW(mainWindow, L"Can't have multiple groups of '?' for substitution.", L"Error", MB_OK);
                        }
                        return;
                    }
                    #endif
	                ++count;
	            }
                #ifdef _DEBUG
	            else if (count) {
                    broke = true;
                    if (count != 4 && count != 1) {
                        if (!reportedOnce) {
                            reportedOnce = true;
                            MessageBoxW(mainWindow, L"Wrong number of '?' for substitution.", L"Error", MB_OK);
                        }
                        return;
                    }
	            }
                #endif
	        }
	        #ifdef _DEBUG
	        if (count != 4 && count != 1) {
                if (!reportedOnce) {
                    reportedOnce = true;
                    MessageBoxW(mainWindow, L"Wrong number of '?' for substitution.", L"Error", MB_OK);
                }
                return;
	        }
	        #endif
	        size_t oldSize = data.size();
	        data.resize(oldSize + sig.sig.size() - 1);
	        auto dest = data.begin() + oldSize;
	        auto src = sig.sig.begin();
	        maskIt = sig.mask.begin();
	        
	        for (int charCount = (int)sig.sig.size() - 1; charCount > 0; ) {
	            if (*maskIt == '?') {
	                if (count == 4) {
	                    *(DWORD*)&*dest = substitute;
	                    dest += 4;
                        src += 4;
                        maskIt += 4;
                        charCount -= 4;
	                } else if (count == 1) {
	                    *(BYTE*)&*dest = (BYTE)substitute;
	                    ++dest;
                        ++src;
                        ++maskIt;
                        --charCount;
	                }
	            } else {
	                *dest++ = *src++;
	                ++maskIt;
	                --charCount;
	            }
	        }
	    }
	} newCode;
	
	#define FIRST_FSTRING 0x0  // FString (12 bytes). MUST BE 0 due to using a 8B 04 24 instruction
	#define SECOND_FSTRING 0xc  // FString (12 bytes)
	#define FIRST_TEXT 0x18  // const wchar_t* (4 bytes)
	#define RESULT 0x1c  // double (8 bytes)
	#define FILETIMEVAR 0x24  // FILETIME (8 bytes)
	#define SUCCESSFUL_PATH 0x2c  // const wchar_t* (4 bytes)
	#define STACK_SPACE 0x60
	
	newCode.add(
	    "53 55 56 57"  // PUSH EBX,EBP,ESI,EDI
	    " 8b f1"  // MOV ESI,ECX
	    " 83 ec ??", STACK_SPACE);  // SUB ESP,STACK_SPACE
	    
    newCode.add("8b 06"  // MOV EAX,dword ptr [ESI]
	    " 8b 50 54"  // MOV EDX,dword ptr [EAX + 0x54]  ; ConvertToAbsolutePath. Takes FString* out and wchar_t* Filename. Returns out
	    " 8B 7C 24 ??", STACK_SPACE + 0x10 + 0x4);  // MOV EDI,dword ptr [ESP + STACK_SPACE + 0x10 (4 pushes) + 4 (Filename, first stack argument)]
    newCode.add("57");  // PUSH EDI
	newCode.add("8d 4c 24 ??", FIRST_FSTRING + 4  /* see PUSH EDI above */);  // LEA ECX,[ESP + FIRST_FSTRING + 4]
	newCode.add("51"  // PUSH ECX
	    " 8b ce" // MOV ECX,ESI
	    " ff d2"  // CALL EDX
    );
	
	DWORD baseVa = rawToVa(FFileManagerWindows_GetFileTimestamp);
	DWORD emptyStringVa = *(DWORD*)(fileBase + FFileManagerWindows_GetFileTimestamp + FFileManagerWindows_GetFileTimestamp_Sig.positions[MOV_EAX_EMPTYSTRING] + 1);
	CrossPlatformCout << "Found an empty string at va:0x" << std::hex << emptyStringVa << std::dec << '\n';
	DWORD negative1Va = *(DWORD*)(fileBase + FFileManagerWindows_GetFileTimestamp + FFileManagerWindows_GetFileTimestamp_Sig.positions[NEGATIVE_1] + 4);
	CrossPlatformCout << "Found an -1.0 at va:0x" << std::hex << negative1Va << std::dec << '\n';
	DWORD appFreeVa = baseVa
	    + (DWORD)FFileManagerWindows_GetFileTimestamp_Sig.positions[APP_FREE]
	    + 5
	    + *(int*)(fileBase + FFileManagerWindows_GetFileTimestamp + FFileManagerWindows_GetFileTimestamp_Sig.positions[APP_FREE] + 1);
	CrossPlatformCout << "Found appFree function at va:0x" << std::hex << appFreeVa << std::dec << '\n';
	
	std::vector<FoundReloc> relocs = relocTable.findRelocsInRegion(
	    baseVa,
	    baseVa + (DWORD)FFileManagerWindows_GetFileTimestamp_Sig.sig.size() - 1);
	
	for (const FoundReloc& foundReloc : relocs) {
		relocTable.removeEntry(file, foundReloc);
	}
	
	newCode.add(
	    "33 db"  // XOR EBX,EBX
	    " 39 58 04"  // CMP dword ptr [EAX + 0x4],EBX
	    " 74 04"  // JZ 0x4
	    " 8b 00"  // MOV EAX,dword ptr [EAX]  ; read Data member of FString
	    " eb 05"  // JMP 0x5
    );
	size_t newCodeSize = newCode.data.size();
	newCode.add("b8 ?? ?? ?? ??", emptyStringVa);  // MOV EAX,emptyString
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 1, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("89 44 24 ??", FIRST_TEXT);  // MOV dword ptr[ESP+FIRST_TEXT],EAX
	
	newCode.add(
	    "8b 16"  // MOV EDX,dword ptr [ESI]
	    " 8b 52 58"  // MOV EDX,dword ptr[EDX + 0x58]  ; ConvertAbsolutePathToUserPath. Takes FString* out and wchar_t* AbsolutePath. Returns out
	    " 50"  // PUSH EAX
	    " 8d 44 24 ??", SECOND_FSTRING + 4 /* see PUSH EAX above */);  // LEA EAX,[ESP + SECOND_FSTRING + 4]
	newCode.add("50"  // PUSH EAX
	    " 8b ce" // MOV ECX,ESI
	    " ff d2"  // CALL EDX
	    
	    " 39 58 04"  // CMP dword ptr [EAX + 0x4],EBX
	    " 74 04"  // JZ 0x4
	    " 8b 00"  // MOV EAX,dword ptr [EAX]  ; read Data member of FString
	    " eb 05"  // JMP 0x5
    );
	newCodeSize = newCode.data.size();
	newCode.add("b8 ?? ?? ?? ??", emptyStringVa);  // MOV EAX,emptyString
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 1, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("89 44 24 ??", SUCCESSFUL_PATH);  // MOV dword ptr[ESP + SUCCESSFUL_PATH],EAX
	newCode.add("50");  // PUSH EAX
	
	newCodeSize = newCode.data.size();
	newCode.add("8b 2d ?? ?? ?? ??", rawToVa(GetFileAttributesWOff));  // MOV EBP,dword ptr[->KERNEL32.DLL::GetFileAttributesW]
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 2, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("FF D5"  // CALL EBP
	    " 4B"  // DEC EBX
	    " 39 D8");  // CMP EAX,EBX
	size_t jmpToAfterRet1 = newCode.data.size();
	newCode.add("75 00"  // JNZ afterRet, will fill in later
	    " 8B 44 24 ??", FIRST_TEXT);  // MOV EAX,dword ptr[ESP + FIRST_TEXT]
    newCode.add("89 44 24 ??", SUCCESSFUL_PATH);  // MOV dword ptr[ESP + SUCCESSFUL_PATH],EAX
	newCode.add("50");  // PUSH EAX
	newCode.add("ff d5"  // CALL EBP
	    " 39 D8");  // CMP EAX,EBX
	size_t jmpToAfterRet2 = newCode.data.size();
	newCode.add("75 00");  // JNZ afterRet, will fill in later
	
	newCodeSize = newCode.data.size();
	size_t errorReturn = newCodeSize;
	newCode.add("f2 0f 10 05 ?? ?? ?? ??", negative1Va);  // MOVSD XMM0,qword ptr[-1.0]
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 4, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("f2 0f 11 44 24 ??", RESULT);  // MOVSD qword ptr[ESP + RESULT],XMM0
	
	size_t returnLabel = newCode.data.size();
	newCode.add("43"  // INC EBX
	    " 8B 04 24"  // MOV EAX,dword ptr[ESP]  ; read Data member of first FString
	    " 39 D8"  // CMP EAX,EBX
	    " 74 09"  // JZ 0x9
	    " 50"  // PUSH EAX
    );
	newCodeSize = newCode.data.size();
	offset = (int)appFreeVa - (int)(baseVa + newCodeSize + 5);
	newCode.add("e8 ?? ?? ?? ??", offset);
	
	newCode.add("83 c4 04"  // ADD ESP,0x4
	    " 8b 44 24 ??", SECOND_FSTRING);  // MOV EAX,dword ptr[ESP + SECOND_FSTRING]  ; read Data member of second FString
	newCode.add("39 D8"  // CMP EAX,EBX
	    " 74 09"  // JZ 0x9
	    " 50"  // PUSH EAX
    );
	newCodeSize = newCode.data.size();
	offset = (int)appFreeVa - (int)(baseVa + newCodeSize + 5);
	newCode.add("e8 ?? ?? ?? ??", offset);
	
	newCode.add("83 c4 04"  // ADD ESP,0x4
	    " dd 44 24 ??", RESULT);  // FLD qword ptr[ESP + RESULT]
	newCode.add("83 c4 ??", STACK_SPACE);  // ADD ESP,STACK_SPACE
	newCode.add("5f 5e 5d 5b"  // POP EDI,ESI,EBP,EBX
	    " c2 04 00");  // RET 0x4
	
	int jumpDistance = (int)(newCode.data.size() - (jmpToAfterRet1 + 2));
	#ifdef _DEBUG
	if (jumpDistance > 0x7f) {
	    MessageBoxW(mainWindow, L"Jump offset 1 too long.", L"Error", MB_OK);
	    return;
	}
	#endif
	newCode.data[jmpToAfterRet1 + 1] = (BYTE)jumpDistance;
	
	jumpDistance = (int)(newCode.data.size() - (jmpToAfterRet2 + 2));
	#ifdef _DEBUG
	if (jumpDistance > 0x7f) {
	    MessageBoxW(mainWindow, L"Jump offset 2 too long.", L"Error", MB_OK);
	    return;
	}
	#endif
	newCode.data[jmpToAfterRet2 + 1] = (BYTE)jumpDistance;
	
	// CreateFileW(L"path",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	newCode.add("6A 00");  // PUSH 0
	newCode.add("68 ?? ?? ?? ??", FILE_ATTRIBUTE_NORMAL);  // PUSH FILE_ATTRIBUTE_NORMAL
	newCode.add("6A ??", OPEN_EXISTING);  // PUSH OPEN_EXISTING
	newCode.add("6A 00");  // PUSH 0
	newCode.add("6A ??", FILE_SHARE_READ);  // PUSH FILE_SHARE_READ
	newCode.add("68 ?? ?? ?? ??", GENERIC_READ);  // PUSH GENERIC_READ
	newCode.add("FF 74 24 ??", SUCCESSFUL_PATH + 6*4);  // PUSH dword ptr[ESP + SUCCESSFUL_PATH + 6pushes]
	
	newCodeSize = newCode.data.size();
	newCode.add("ff 15 ?? ?? ?? ??", rawToVa(CreateFileWOff));  // CALL dword ptr[->KERNEL32.DLL::CreateFileW]
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 2, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("8b f0"  // MOV ESI,EAX
	    " 39 DE"  // CMP ESI,EBX
    );
	#ifdef _DEBUG
	if (newCode.data.size() + 2 - errorReturn > 0x80) {
	    MessageBoxW(mainWindow, L"Can't jump from CreateFileW to error return.", L"Error", MB_OK);
	    return;
	}
	#endif
	newCode.add("74 ??", (int)errorReturn - (int)(newCode.data.size() + 2));  // JZ errorReturn
	
	// GetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime)
	newCode.add("8D 44 24 ??", FILETIMEVAR);  // LEA EAX,[ESP+FILETIMEVAR]
	newCode.add("50"  // PUSH EAX
	    " 6A 00"  // PUSH 0
	    " 6A 00"  // PUSH 0
	    " 56");  // PUSH ESI
	
	newCodeSize = newCode.data.size();
	newCode.add("ff 15 ?? ?? ?? ??", rawToVa(GetFileTimeOff));  // CALL dword ptr[->KERNEL32.DLL::GetFileTime]
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 2, IMAGE_REL_BASED_HIGHLOW);
	
	#ifdef _DEBUG
	if (newCode.data.size() + 4 - errorReturn > 0x80) {
	    MessageBoxW(mainWindow, L"Can't jump from GetFileTime to error return.", L"Error", MB_OK);
	    return;
	}
	#endif
	
	newCode.add("89 c5"  // MOV EBP,EAX
	    " 56");  // PUSH ESI
	
	newCodeSize = newCode.data.size();
	newCode.add("ff 15 ?? ?? ?? ??", rawToVa(CloseHandleOff));  // CALL dword ptr[->KERNEL32.DLL::CloseHandle]
	relocTable.addEntry(file, baseVa + (DWORD)newCodeSize + 2, IMAGE_REL_BASED_HIGHLOW);
	
	newCode.add("85 ed"  // TEST EBP,EBP
	    " 74 ??",  // JZ errorReturn
	    (int)errorReturn - (int)(newCode.data.size() + 4)
    );
	
	// FILETIME:
    // Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	
	long long timeDiff = (
            (1970LL - 1601LL) * 365LL
            + 24LL * 3LL  // 3 centuries worth of leap years
            + (70 - 1) / 4  // leap years between 1900 and 1970, not inclusive
        ) * 24LL  // hours
        * 60LL  // minutes
        * 60LL  // seconds
        * 1000LL  // milliseconds
        * 1000LL  // microseconds
        * 10LL;  // 100 nanosecond intervals
	
	newCode.add("81 6C 24 ??", FILETIMEVAR); // SUB dword ptr[ESP+FILETIMEVAR],...
	newCode.add("?? ?? ?? ??", (DWORD)timeDiff); // first 4 bytes of timeDiff
	newCode.add("81 5C 24 ??", FILETIMEVAR + 4); // SBB dword ptr[ESP+FILETIMEVAR + 4],...
	newCode.add("?? ?? ?? ??", (DWORD)(timeDiff >> 32)); // last 4 bytes of timeDiff
	newCode.add("b9 ?? ?? ?? ??", 10000000);  // MOV ECX,10000000
	newCode.add("8b 44 24 ??", FILETIMEVAR + 4);  // MOV EAX,dword ptr[ESP+FILETIMEVAR + 4]
	newCode.add("31 d2"  // XOR EDX,EDX
	    " F7 F1"  // DIV ECX
	    " 89 c5");  // MOV EBP,EAX
	newCode.add("8b 44 24 ??", FILETIMEVAR);  // MOV EAX,dword ptr[ESP+FILETIMEVAR]
    newCode.add("F7 F1"  // DIV ECX
        " 89 44 24 ??", RESULT);  // MOV dword ptr[ESP+RESULT],EAX
    newCode.add("89 6C 24 ??", RESULT + 4);  // MOV dword ptr[ESP+RESULT + 4],EBP
	
	newCode.add("DF 6C 24 ??", RESULT);  // FILD qword ptr[ESP+RESULT]
	newCode.add("dd 5c 24 ??", RESULT);  // FSTP qword ptr[ESP+RESULT]
	
	jumpDistance = (int)returnLabel - (int)(newCode.data.size() + 2);
	if (jumpDistance >= -0x80) {
	    newCode.add("eb ??", jumpDistance);
	} else {
	    jumpDistance = (int)returnLabel - (int)(newCode.data.size() + 5);
	    newCode.add("e9 ?? ?? ?? ??", jumpDistance);
	}
	
	#ifdef _DEBUG
	if (newCode.data.size() > FFileManagerWindows_GetFileTimestamp_Sig.sig.size() - 1) {
	    MessageBoxW(mainWindow, L"The new code is larger than the old code.", L"Error", MB_OK);
	    return;
	}
	#endif
	
	fseek(file, FFileManagerWindows_GetFileTimestamp, SEEK_SET);
	fwrite(newCode.data.data(), 1, newCode.data.size(), file);
	
    CrossPlatformCout << "Patched successfully\n";
	
    fclose(file);
	
	CrossPlatformString bin_win32_Folder = getParentDir(szFile);
	CrossPlatformString bin_Folder = getParentDir(bin_win32_Folder);
	CrossPlatformString root_Folder = getParentDir(bin_Folder);
	CrossPlatformString engine_config_Folder = root_Folder
	    + SEPARATOR_CHAR + CrossPlatformText("Engine")
	    + SEPARATOR_CHAR + CrossPlatformText("Config");
	CrossPlatformString redgame_Folder = root_Folder
	    + SEPARATOR_CHAR + CrossPlatformText("REDGame");
	CrossPlatformString redgame_config_Folder = root_Folder
	    + SEPARATOR_CHAR + CrossPlatformText("REDGame")
	    + SEPARATOR_CHAR + CrossPlatformText("Config");
	
	struct ThreeNames {
	    const CrossPlatformChar* red;
	    const CrossPlatformChar* default_;
	    const CrossPlatformChar* base;
	    ThreeNames(const CrossPlatformChar* red, const CrossPlatformChar* default_, const CrossPlatformChar* base)
	        : red(red), default_(default_), base(base) { }
	};
	
	static const ThreeNames iniNames[] = {
	    { CrossPlatformText("REDDebug.ini"), CrossPlatformText("DefaultDebug.ini"), CrossPlatformText("BaseDebug.ini") },
	    { CrossPlatformText("REDEngine.ini"), CrossPlatformText("DefaultEngine.ini"), CrossPlatformText("BaseEngine.ini") },
	    { CrossPlatformText("REDGame.ini"), CrossPlatformText("DefaultGame.ini"), CrossPlatformText("BaseGame.ini") },
	    { CrossPlatformText("REDInput.ini"), CrossPlatformText("DefaultInput.ini"), CrossPlatformText("BaseInput.ini") },
	    { CrossPlatformText("REDLightmass.ini"), CrossPlatformText("DefaultLightmass.ini"), CrossPlatformText("BaseLightmass.ini") },
	    { CrossPlatformText("REDSystemSettings.ini"), CrossPlatformText("DefaultSystemSettings.ini"), CrossPlatformText("BaseSystemSettings.ini") },
	    { CrossPlatformText("REDTMS.ini"), CrossPlatformText("DefaultTMS.ini"), CrossPlatformText("BaseTMS.ini") },
	    { CrossPlatformText("REDUI.ini"), CrossPlatformText("DefaultUI.ini"), CrossPlatformText("BaseUI.ini") }
	};
	
	for (const ThreeNames& threeName : iniNames) {
	    CrossPlatformString redPath = redgame_config_Folder + SEPARATOR_CHAR + threeName.red;
	    CrossPlatformString defaultPath = redgame_config_Folder + SEPARATOR_CHAR + threeName.default_;
	    if (!fileExists(redPath) || !fileExists(defaultPath)) continue;
	    
	    
	    FILE* defaultFile = NULL;
	    if (!crossPlatformOpenFile(&defaultFile, defaultPath)) continue;
	    
	    struct FileCloser {
	        ~FileCloser() {
	            if (file) fclose(file);
	        }
	        FILE* file = NULL;
	    } fileCloser { defaultFile };
	    
	    std::vector<char> defaultData;
	    if (!readWholeFile(defaultFile, defaultData)) continue;
        
        struct DesiredEdit {
            int charOffsetStart;
            int charOffsetEnd;  // non-inclusive
            std::string newText;
        };
        
	    struct SectionTracker {
	        
	        bool freshLine = true;
	        bool dontLikeLine = false;
            std::string sectionName;
            bool inSectionName = false;
            bool isInConfiguration = false;
            bool isInIniVersion = false;
            bool isComment = false;
            std::string keyName;
            std::string value;
            bool encounteredEqualSign = false;
            CrossPlatformString redgame_Folder;  // UE3 adds one more .. at the start of BasedOn values
            bool basedOnDetected = false;
            double basedOnTimestamp = 0.0;
            void(SectionTracker::*onLineEnd)(int charOffset) = nullptr;
            int lineStartOffset = -1;
            std::vector<double> newTimestamps;
            std::vector<DesiredEdit> desiredEdits;
	        
	        void onLineEnd_BasedOn(int charOffset) {
	            if (!dontLikeLine && !sectionName.empty() && !inSectionName) {
	                if (CrossPlatformStringCompareCaseInsensitive(sectionName.c_str(), "Configuration") == 0) {
	                    isInConfiguration = true;
	                } else {
	                    isInConfiguration = false;
	                }
	            } else if (isInConfiguration && !keyName.empty() && !value.empty()) {
                    trimLeft(value);
                    trimRight(value);
                    if (CrossPlatformStringCompareCaseInsensitive(keyName.c_str(), "BasedOn") == 0) {
                        CrossPlatformString currentPath = redgame_Folder;
                        CrossPlatformString currentPiece;
                        for (char c : value) {
                            if (c == '\\') {
                                if (!currentPiece.empty() && CrossPlatformPathCompare(currentPiece.c_str(), CrossPlatformText("..")) == 0) {
                                    currentPath = getParentDir(currentPath);
                                } else if (!currentPiece.empty()) {
                                    currentPath += SEPARATOR_CHAR + currentPiece;
                                }
                                currentPiece.clear();
                            } else {
                                currentPiece += (CrossPlatformChar)c;
                            }
                        }
                        if (!currentPiece.empty()) {
                            currentPath += SEPARATOR_CHAR + currentPiece;
                        }
                        
                        basedOnDetected = getTimestamp(currentPath, &basedOnTimestamp);
                        
                    }
	            }
	        }
	        
	        void onLineEnd_IniVersion(int charOffset) {
	            if (!dontLikeLine && !sectionName.empty() && !inSectionName) {
	                if (CrossPlatformStringCompareCaseInsensitive(sectionName.c_str(), "IniVersion") == 0) {
	                    isInIniVersion = true;
	                } else {
	                    isInIniVersion = false;
	                }
	            } else if (isInIniVersion && !keyName.empty() && !value.empty() && lineStartOffset != -1) {
                    int parsedIndex = -1;
                    if (strcmp(keyName.c_str(), "0") == 0) {
                        parsedIndex = 0;
                    } else if (strcmp(keyName.c_str(), "1") == 0) {
                        parsedIndex = 1;
                    }
                    if (parsedIndex != -1 && parsedIndex < (int)newTimestamps.size()) {
                        char strbuf[1024];
                        SPRINTF_NARROW(strbuf, "%d=%f", parsedIndex, newTimestamps[parsedIndex]);
                        
                        desiredEdits.emplace_back();
                        DesiredEdit& newEdit = desiredEdits.back();
                        newEdit.charOffsetStart = lineStartOffset;
                        newEdit.charOffsetEnd = charOffset;
                        newEdit.newText = strbuf;
                    }
	            }
	        }
	        
	        static bool getTimestamp(const CrossPlatformString& path, double* timestamp) {
	            
                if (fileExists(path)) {
                    
                    unsigned long long ticks = 0;
                    
                    #define SECS_1601_TO_1970  ((369 * 365 + 89) * (ULONGLONG)86400)
                    #define TICKSPERSEC 10000000
                    
                    #ifndef FOR_LINUX
                    CreateFileW(L"path",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
                    HANDLE baseFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (baseFile != INVALID_HANDLE_VALUE) {
                        
                        struct OwnFileCloser {
                            ~OwnFileCloser() {
                                if (file) CloseHandle(file);
                            }
                            HANDLE file = NULL;
                        } ownFileCloser { baseFile };
                        
                        FILETIME fileTime;
                        
                        if (GetFileTime(baseFile, NULL, NULL, &fileTime)) {
                            ticks = *(unsigned long long*)&fileTime;
                            ticks = (ticks - SECS_1601_TO_1970 * TICKSPERSEC) / TICKSPERSEC;
                        }
                    }
                    #else
                    struct stat buf;
                    int statResult = stat(path.c_str(), &buf);
                    if (statResult != -1) {
                        if (sizeof(time_t) == sizeof(int)) {
                            ticks = (ULONGLONG)(ULONG)buf.st_mtime;
                        } else {
                            ticks = (ULONGLONG)buf.st_mtime;
                        }
                    }
                    #endif
                    
                    if (ticks) {
                        *timestamp = (double)ticks;
                        return true;
                    }
                }
                return false;
	        }
	        
	        void reset() {
                lineStartOffset = -1;
                keyName.clear();
                value.clear();
                freshLine = true;
                dontLikeLine = false;
                sectionName.clear();
                inSectionName = false;
                encounteredEqualSign = false;
                isComment = false;
	        }
	        
	        void parseChar(char c, int charOffset) {
	            if (c == '\r' || c == '\n') {
	                (this->*onLineEnd)(charOffset);
	                reset();
	            } else if (freshLine && c == '[') {
                    inSectionName = true;
	                lineStartOffset = charOffset;
                    freshLine = false;
	            } else if (inSectionName) {
	                if (c == ']') {
	                    inSectionName = false;
	                } else {
	                    sectionName += c;
	                }
	            } else if (!freshLine && (c == '\t' || c == ' ') && !sectionName.empty()) {
	                // ok, allowed to have whitespace after a section ]
	                // UE3 only considers '\t' and ' ' to be whitespace
	            } else {
	                if (freshLine) {
                        if (c == ';') isComment = true;
	                    lineStartOffset = charOffset;
                        freshLine = false;
	                }
	                dontLikeLine = true;
	                if ((isInConfiguration || isInIniVersion) && !isComment) {
	                    if (c == '=' && !encounteredEqualSign) {
	                        encounteredEqualSign = true;
	                        trimLeft(keyName);
	                        trimRight(keyName);
	                    } else if (!encounteredEqualSign) {
	                        keyName += c;
	                    } else {
	                        value += c;
	                    }
	                }
	            }
	        }
	        
            void runLoop(const std::vector<char>& data, void(SectionTracker::*onLineEndParam)(int charOffset)) {
                onLineEnd = onLineEndParam;
                int cOffset = 0;
                for (char c : data) {
                    parseChar(c, cOffset);
                    ++cOffset;
                }
                (this->*onLineEnd)(cOffset);
            }
	        
            void applyEdits(std::vector<char>& data) {
                for (DesiredEdit& desiredEdit : desiredEdits) {
                    data.erase(data.begin() + desiredEdit.charOffsetStart, data.begin() + desiredEdit.charOffsetEnd);
                    data.insert(data.begin() + desiredEdit.charOffsetStart, desiredEdit.newText.size(), 0);
                    memcpy(data.data() + desiredEdit.charOffsetStart, desiredEdit.newText.c_str(), desiredEdit.newText.size());
                    int shift = (int)desiredEdit.newText.size()
                        - (desiredEdit.charOffsetEnd - desiredEdit.charOffsetStart);
                    for (DesiredEdit& desiredEditModif : desiredEdits) {
                        if (desiredEditModif.charOffsetStart > desiredEdit.charOffsetStart) {
                            desiredEditModif.charOffsetStart += shift;
                            desiredEditModif.charOffsetEnd += shift;
                        }
                    }
                }
            }
            
	    } sectionTracker;
	    sectionTracker.redgame_Folder = redgame_Folder;
	    
	    sectionTracker.runLoop(defaultData, &SectionTracker::onLineEnd_BasedOn);
	    sectionTracker.reset();
	    
	    if (sectionTracker.basedOnDetected) {
	        sectionTracker.newTimestamps.push_back(sectionTracker.basedOnTimestamp);
	    }
	    
	    fclose(defaultFile);
	    fileCloser.file = NULL;
	    
        double defaultTimestamp;
        if (SectionTracker::getTimestamp(defaultPath, &defaultTimestamp)) {
            sectionTracker.newTimestamps.push_back(defaultTimestamp);
        }
	    
	    FILE* redFile = NULL;
	    if (crossPlatformOpenFile(&redFile, redPath)) {
	        fileCloser.file = redFile;
	        
	        std::vector<char> redData;
	        if (readWholeFile(redFile, redData)) {
	            
	            sectionTracker.runLoop(redData, &SectionTracker::onLineEnd_IniVersion);
	            
	            if (sectionTracker.desiredEdits.size() == sectionTracker.newTimestamps.size() && !sectionTracker.desiredEdits.empty()) {
	                sectionTracker.applyEdits(redData);
	                overwriteWholeFile(redFile, redData);
	                CrossPlatformCout << "Updated IniVersion timestamps in " << getFileName(redPath).c_str() << '\n';
	                fclose(redFile);
	                fileCloser.file = NULL;
	            }
	            
	        }
            
	    }
	    
	    
	}
	
}

#ifdef FOR_LINUX
int main()
#else
int patcherMain()
#endif
{
	#ifndef FOR_LINUX
	int offset = (int)(
		(GetTickCount64() & 0xF000000000000000ULL) >> (63 - 4)
	);
	#else
	int offset = 0;
	#endif
	
	for (int i = 0; i < sizeof exeName - 1; ++i) {
		exeName[i] += offset + 10;
	}
	
    CrossPlatformCout <<
                  "This program patches " << exeName << " executable to permanently change how it reads timestamps of files, and does a"
                  " one-time update of timestamps in all RED* INI files.\n"
                  "This cannot be undone, and you should backup your " << exeName << " file before proceeding.\n"
                  "A backup copy will also be automatically created (but that may fail).\n"
                  "Only Guilty Gear Xrd Rev2 version 2211 supported.\n"
                  "Press Enter when ready...\n";

    CrossPlatformString ignoreLine;
    GetLine(ignoreLine);

    meatOfTheProgram();

    CrossPlatformCout << "Press Enter to exit...\n";
    GetLine(ignoreLine);
    return 0;
}

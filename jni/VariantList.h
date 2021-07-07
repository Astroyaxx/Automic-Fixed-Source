#pragma once
#include <string>
#include <vector>
#include "utils.hpp"

struct Vector2 { 
	int x, y; 

	Vector2(int x, int y) { this->x = x;  this->y = y; } 
	Vector2() {} 
} __attribute__((packed));

struct Vector3 {
	int x, y, z;

	Vector3(int x, int y, int z) { this->x = x; this->y = y; this->z = z; } 
	Vector3() {} 
} __attribute__((packed));

enum VarType {
	ERRTYPE,
	TFLOAT,
	TSTRING,
	TVECTOR2,
	TVECTOR3,
	TUINT,
	TINT = 9
};


struct Var {
#pragma pack(push, 1) 
	uint8_t byteVal;
	short shortVal;
	uint16_t ushortVal;
	int intVal;
	uint32_t uintVal;
	float floatVal;
	int64_t longVal;
	uint64_t ulongVal;
	double doubleVal;

	Vector2 vector2Val;
	Vector3 vector3Val;

	std::string strVal;

	VarType vType;
	uint8_t index;
#pragma pack(pop)
	operator uint8_t() const { return byteVal; }
	operator short() const { return shortVal; }
	operator uint16_t() const { return ushortVal; }
	operator int() const { return intVal; }
	operator uint32_t() const { return uintVal; }
	operator int64_t() const { return longVal; }
	operator uint64_t() const { return ulongVal; }
	operator double() const { return doubleVal; }
	operator std::string() const { return strVal; }
};

namespace Variant {
	class VariantList {
	public:
		uint8_t* Serialize();

		VariantList() {}
		VariantList(void* data); // Load variantlist from data!
		~VariantList() { varvec.clear(); }
		const bool IsValid();
		const std::string GetFuncName();
		const void destroy();
		const Var GetFuncArg(short index);
		const short GetCount();

		static void Kill(VariantList* vList);
		int executionDelay = -1;
		int netID = -1;


	private:
		std::vector<Var> varvec;
		uint8_t varcount;
	};
}
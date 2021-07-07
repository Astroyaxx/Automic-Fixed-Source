#include "VariantList.h"

Variant::VariantList::VariantList(void* data)
{
	uint8_t* dataPtr = (uint8_t*)data + 56;
	uint8_t count = dataPtr[0]; dataPtr++;
	
	for (int i = 0; i < count; i++) {
		Var v; // prepare Var
		v.index = dataPtr[0]; dataPtr++;
		v.vType = (VarType)dataPtr[0]; dataPtr++;

		switch (v.vType) {
		case VarType::TINT:
		{
			v.intVal = *(int*)dataPtr; dataPtr += 4;
			break;
		}
		case VarType::TSTRING:
		{
			int strLen = *(int*)dataPtr; dataPtr += 4;
			v.strVal.insert(v.strVal.end(), dataPtr, dataPtr + strLen); dataPtr += strLen;
			break;
		}
		case VarType::TUINT:
		{
			v.uintVal = *(unsigned int*)dataPtr; dataPtr += 4;
			break;
		}
		case VarType::TVECTOR2:
		{
			v.vector2Val = *(Vector2*)dataPtr; dataPtr += 8;
			break;
		}
		case VarType::TVECTOR3:
		{
			dataPtr += 12;
			break;
		}
		case VarType::TFLOAT:
		{
			v.floatVal = *(float*)dataPtr; dataPtr += 4;
			break;
		}

		default:
			v.vType = (VarType)0;
			break;
		}
		
		if (v.vType > 0) {
			varvec.push_back(v);
			varcount = count; // varcount that is supposed to be provided by servers.
		}
	}

}

const bool Variant::VariantList::IsValid()
{
	return varcount > 0 && varvec.size() == varcount;
}

const std::string Variant::VariantList::GetFuncName()
{
	if (IsValid()) {
		return varvec[0].strVal;
	}

	return "INVALID_FUNC_NAME";
}

const void Variant::VariantList::destroy()
{
	varvec.clear();
	delete this;
}

const Var Variant::VariantList::GetFuncArg(short index)
{
	if (IsValid()) {
		if (index < varcount)
			return varvec[index];
	}
	return Var();
}

const short Variant::VariantList::GetCount()
{
	return IsValid() ? varcount : 0;
}

void Variant::VariantList::Kill(VariantList* vList)
{
	vList->destroy();
	vList = NULL;
}

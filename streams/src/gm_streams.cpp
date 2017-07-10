#include "GarrysMod/Lua/Interface.h"
#include <stdio.h>
#include "stream.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

using namespace GarrysMod::Lua;

int g_iStreamMetaTable;
int g_iUInt64MetaTable;
int g_iInt64MetaTable;

#pragma region Macros
#define TYPE_STREAM 170
#define TYPE_UINT64 180
#define TYPE_INT64	190

typedef unsigned long long uint64;
typedef long long int64;

typedef unsigned int uint32;
typedef int int32;

#define GETSTREAM(idx) (Stream*)((UserData*)LUA->GetUserdata(idx))->data
#define GETUINT64(idx) (uint64*)((UserData*)LUA->GetUserdata(idx))->data
#define GETINT64(idx) (int64*)((UserData*)LUA->GetUserdata(idx))->data

#define CHECKSTREAM(idx) if ((GETSTREAM(idx))->IsClosed()) {\
	LUA->ArgError(idx, "stream is closed"); return 0;}

#define OPEN_APPEND			std::ios::binary | std::ios::in | std::ios::out | std::ios::ate
#define OPEN_TRUNCATE		std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc
#define OPEN_READONLY		std::ios::binary | std::ios::in
#define OPEN_WRITEONLY		std::ios::binary | std::ios::out
#define OPEN_READWRITE		std::ios::binary | std::ios::out | std::ios::in
	
// nice code noob
#define PUSH_GLOBAL(key, value, type) LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);\
	LUA->PushString(key);\
	LUA->Push##type(value);\
	LUA->SetTable(-3)

#define STREAM_BIND(name) LUA->PushCFunction(Stream_##name);\
	LUA->SetField(-2, #name)

#define STREAM_FUNCTION_WRITE(type, luaType, luaFunc, name) int Stream_Write##name (lua_State* state){\
	CHECKSTREAM(1)\
	LUA->CheckType(1, TYPE_STREAM);\
	LUA->CheckType(2, luaType);\
	Stream* stream = GETSTREAM(1);\
	stream->Write<type>((type)LUA->Get##luaFunc ());\
	return 0;}

#define STREAM_FUNCTION_READ(type, luaFunc, name) int Stream_Read##name (lua_State* state){\
	CHECKSTREAM(1)\
	LUA->CheckType(1, TYPE_STREAM);\
	Stream* stream = GETSTREAM(1);\
	LUA->Push##luaFunc(stream->Read<type>());\
	return 1;}
#pragma endregion

#pragma region Utility
void PushUInt64(lua_State* state, uint64 value)
{
	UserData* ud = (UserData*)LUA->NewUserdata(sizeof(UserData) + sizeof(uint64));
	ud->data = ud + 1;
	ud->type = TYPE_UINT64;

	LUA->ReferencePush(g_iUInt64MetaTable);
	LUA->SetMetaTable(-2);

	*(reinterpret_cast<uint64*>(ud->data)) = value;
}

void PushInt64(lua_State* state, int64 value)
{
	UserData* ud = (UserData*)LUA->NewUserdata(sizeof(UserData) + sizeof(int64));
	ud->data = ud + 1;
	ud->type = TYPE_INT64;

	LUA->ReferencePush(g_iUInt64MetaTable);
	LUA->SetMetaTable(-2);

	*(reinterpret_cast<int64*>(ud->data)) = value;
}
#pragma endregion

#pragma region Stream Methods

///
/// Stream_GC
///
int Stream_GC(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);

	Stream* stream = GETSTREAM(1);
	stream->Close();

	return 0;
}

///
/// Stream_ToString
///
int Stream_ToString(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	Stream* stream = GETSTREAM(1);

	std::ostringstream stringStream;


	if (!stream->IsClosed())
	{
		if (reinterpret_cast<std::fstream*>(stream->GetUnderlyingStream()) != 0)
		{
			stringStream << "Stream::File::" << std::hex << stream;
		}
		else
		{
			stringStream << "Stream::Memory::" << std::hex << stream;
		}
	}
	else
	{
		stringStream << "Stream::Closed::" << std::hex << stream;
	}

	LUA->PushString(stringStream.str().c_str());

	return 1;
}

///
/// Stream_Close
///
int Stream_Close(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	Stream* stream = GETSTREAM(1);

	if (stream->IsClosed())
	{
		LUA->ArgError(1, "stream is already closed");
		return 0;
	}

	stream->Close();

	return 0;
}

///
/// Stream_Clear
///
int Stream_Clear(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	CHECKSTREAM(1);
	Stream* stream = GETSTREAM(1);

	stream->Clear();
	return 0;
}

///
/// Stream_IsClosed
///
int Stream_IsClosed(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);

	Stream* stream = GETSTREAM(1);
	LUA->PushBool(stream->IsClosed());
	return 1;
}

///
/// Stream_TellRead
///
int Stream_TellRead(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	CHECKSTREAM(1);
	Stream* stream = GETSTREAM(1);
	
	PushUInt64(state, stream->TellRead());
	return 1;
}

///
/// Stream_TellWrite
///
int Stream_TellWrite(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	PushUInt64(state, stream->TellWrite());
	return 1;
}

///
/// Stream_SeekWrite
///
int Stream_SeekWrite(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	LUA->CheckType(2, TYPE_UINT64);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	stream->SeekWrite((std::streamoff)*GETUINT64(2));
	return 0;
}

///
/// Stream_SeekRead
///
int Stream_SeekRead(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	LUA->CheckType(2, TYPE_UINT64);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	stream->SeekRead((std::streamoff)*GETUINT64(2));
	return 0;
}


///
/// Stream_Flush
///
int Stream_Flush(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	stream->Flush();
	return 0;
}


///
/// Stream_Size
///
int Stream_Size(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	PushUInt64(state, stream->Size());
	return 1;
}

///
/// Stream_ReadString
///
int Stream_ReadString(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	LUA->CheckType(2, Type::NUMBER);
	CHECKSTREAM(1);

	int len = (int)LUA->GetNumber(2);
	char* buf = new char[len];

	Stream* stream = GETSTREAM(1);
	stream->ReadString(buf, len);
	LUA->PushString(buf, len);
	delete[] buf;
	return 1;
}

///
/// Stream_WriteString
///
int Stream_WriteString(lua_State* state)
{
	LUA->CheckType(1, TYPE_STREAM);
	LUA->CheckType(2, Type::STRING);
	CHECKSTREAM(1);

	Stream* stream = GETSTREAM(1);
	uint32 size = 0;
	const char* str = LUA->GetString(2, &size);

	stream->WriteString(str, size);
	return 0;
}

STREAM_FUNCTION_WRITE(char, Type::NUMBER, Number, Int8)
STREAM_FUNCTION_WRITE(short, Type::NUMBER, Number, Int16)
STREAM_FUNCTION_WRITE(int, Type::NUMBER, Number, Int32)
STREAM_FUNCTION_WRITE(long long, Type::NUMBER, Number, Int64)

STREAM_FUNCTION_WRITE(unsigned char, Type::NUMBER, Number, UInt8)
STREAM_FUNCTION_WRITE(unsigned short, Type::NUMBER, Number, UInt16)
STREAM_FUNCTION_WRITE(unsigned int, Type::NUMBER, Number, UInt32)

STREAM_FUNCTION_WRITE(float, Type::NUMBER, Number, Float)
STREAM_FUNCTION_WRITE(double, Type::NUMBER, Number, Double)

STREAM_FUNCTION_READ(char, Number, Int8)
STREAM_FUNCTION_READ(short, Number, Int16)
STREAM_FUNCTION_READ(int, Number, Int32)
STREAM_FUNCTION_READ(long long, Number, Int64)

STREAM_FUNCTION_READ(unsigned char, Number, UInt8)
STREAM_FUNCTION_READ(unsigned short, Number, UInt16)
STREAM_FUNCTION_READ(unsigned int, Number, UInt32)

STREAM_FUNCTION_READ(float, Number, Float)
STREAM_FUNCTION_READ(double, Number, Double)

int Stream_WriteUInt64(lua_State* state)
{
	CHECKSTREAM(1)
	LUA->CheckType(1, TYPE_STREAM);
	LUA->CheckType(2, TYPE_UINT64);

	Stream* stream = GETSTREAM(1);
	stream->Write<uint64>(*GETUINT64(2));
	return 0;
}

int Stream_ReadUInt64(lua_State* state)
{
	CHECKSTREAM(1)
	LUA->CheckType(1, TYPE_STREAM);
	Stream* stream = GETSTREAM(1);

	PushUInt64(state, stream->Read<uint64>());
	return 1;
}

#pragma endregion

#pragma region File Methods
///
/// File_Open
///
int File_Open(lua_State* state)
{
	LUA->CheckType(1, Type::STRING);
	std::string filename = std::string(LUA->GetString(1));

	std::fstream* fs;
	if (LUA->IsType(2, Type::NUMBER))
	{
		fs = new std::fstream(("garrysmod/data/" + filename).c_str(), (std::ios_base::openmode)LUA->GetNumber(2));
	}
	else
	{
		fs = new std::fstream(("garrysmod/data/" + filename).c_str());
	}

	if (!fs->good())
	{
		LUA->PushBool(false);
		return 1;
	}

	UserData* ud = (UserData*)LUA->NewUserdata(sizeof(UserData));
	ud->data = new Stream(fs);
	ud->type = TYPE_STREAM;
	LUA->ReferencePush(g_iStreamMetaTable);
	LUA->SetMetaTable(-2);

	return 1;
}

int File_Delete(lua_State* state)
{
	LUA->CheckType(1, Type::STRING);
	std::string path = "garrysmod/data/" + std::string(LUA->GetString(1));
	std::remove(path.c_str());
	return 0;
}

int File_Exists(lua_State* state)
{
	LUA->CheckType(1, Type::STRING);
	std::string path = "garrysmod/data/" + std::string(LUA->GetString(1));
	std::ifstream infile(path.c_str());

	LUA->PushBool(infile.good());
	infile.close();
	return 1;
}
#pragma endregion

#pragma region UInt64 Meta Methods

///
/// __tostring
///
int UInt64_ToString(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);

	std::stringstream s;
	s << *GETUINT64(1);
	LUA->PushString(s.str().c_str());
	return 1;
}

///
/// __unm
///
int UInt64_Neg(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	PushUInt64(state, *GETUINT64(1));
	return 1;
}

///
/// __add
///
int UInt64_Add(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETUINT64(1) + ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, *GETUINT64(1) + *GETUINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __sub
///
int UInt64_Sub(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETUINT64(1) - ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, *GETUINT64(1) - *GETUINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __mul
///
int UInt64_Mul(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETUINT64(1) * ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, *GETUINT64(1) * *GETUINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __div
///
int UInt64_Div(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETUINT64(1) / ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, *GETUINT64(1) / *GETUINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __mod
///
int UInt64_Mod(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETUINT64(1) % ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, *GETUINT64(1) % *GETUINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __pow
///
int UInt64_Pow(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, std::pow(*GETUINT64(1), ((uint32)LUA->GetNumber(2))));
	}
	else if (LUA->IsType(2, TYPE_UINT64))
	{
		PushUInt64(state, std::pow(*GETUINT64(1), *GETUINT64(2)));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __eq
///
int UInt64_Equal(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	LUA->CheckType(2, TYPE_UINT64);

	LUA->PushBool(*GETUINT64(1) == *GETUINT64(2));
	return 1;
}

///
/// __lt
///
int UInt64_LessThan(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	LUA->CheckType(2, TYPE_UINT64);

	LUA->PushBool(*GETUINT64(1) < *GETUINT64(2));
	return 1;
}

///
/// __le
///
int UInt64_LessThanOrEqual(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	LUA->CheckType(2, TYPE_UINT64);

	LUA->PushBool(*GETUINT64(1) <= *GETUINT64(2));
	return 1;
}

#pragma endregion

#pragma region Int64 Meta Methods

///
/// __tostring
///
int Int64_ToString(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);

	std::stringstream s;
	s << *GETINT64(1);
	LUA->PushString(s.str().c_str());
	return 1;
}

///
/// __unm
///
int Int64_Neg(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	PushInt64(state, -*GETINT64(1));
	return 1;
}

///
/// __add
///
int Int64_Add(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushInt64(state, *GETINT64(1) + ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushInt64(state, *GETINT64(1) + *GETINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __sub
///
int Int64_Sub(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushInt64(state, *GETINT64(1) - ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushInt64(state, *GETINT64(1) - *GETINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __mul
///
int Int64_Mul(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushUInt64(state, *GETINT64(1) * ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushUInt64(state, *GETINT64(1) * *GETINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __div
///
int Int64_Div(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushInt64(state, *GETINT64(1) / ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushInt64(state, *GETINT64(1) / *GETINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __mod
///
int Int64_Mod(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushInt64(state, *GETINT64(1) % ((uint32)LUA->GetNumber(2)));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushInt64(state, *GETINT64(1) % *GETINT64(2));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __pow
///
int Int64_Pow(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	if (LUA->IsType(2, Type::NUMBER))
	{
		PushInt64(state, std::pow(*GETINT64(1), ((uint32)LUA->GetNumber(2))));
	}
	else if (LUA->IsType(2, TYPE_INT64))
	{
		PushInt64(state, std::pow(*GETINT64(1), *GETINT64(2)));
	}
	else
	{
		LUA->ArgError(2, "bad type");
		return 0;
	}
	return 1;
}

///
/// __eq
///
int Int64_Equal(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	LUA->CheckType(2, TYPE_INT64);

	LUA->PushBool(*GETINT64(1) == *GETINT64(2));
	return 1;
}

///
/// __lt
///
int Int64_LessThan(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	LUA->CheckType(2, TYPE_INT64);

	LUA->PushBool(*GETINT64(1) < *GETINT64(2));
	return 1;
}

///
/// __le
///
int Int64_LessThanOrEqual(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	LUA->CheckType(2, TYPE_INT64);

	LUA->PushBool(*GETINT64(1) <= *GETINT64(2));
	return 1;
}

#pragma endregion

#pragma region Conversion

int NumberToUInt64(lua_State* state)
{
	LUA->CheckType(1, Type::NUMBER);
	uint64 num;

	if (((int)LUA->GetNumber(1)) == -1)
	{
		num = 0xFFFFFFFFFFFFFFFF;
	}
	else 
		num = (uint64)LUA->GetNumber(1);

	PushUInt64(state, num);
	return 1;
}

int UInt64ToNumber(lua_State* state)
{
	LUA->CheckType(1, TYPE_UINT64);
	LUA->PushNumber((int32)*GETUINT64(1));
	return 1;
}

int NumberToInt64(lua_State* state)
{
	LUA->CheckType(1, Type::NUMBER);
	PushInt64(state, (int64)LUA->GetNumber(1));
	return 1;
}

int Int64ToNumber(lua_State* state)
{
	LUA->CheckType(1, TYPE_INT64);
	LUA->PushNumber((int32)*GETINT64(1));
	return 1;
}

#pragma endregion

#pragma region Memory Stream
int Memory_OpenStream(lua_State* state)
{
	UserData* ud = (UserData*)LUA->NewUserdata(sizeof(UserData));
	ud->data = new Stream(new std::stringstream());
	ud->type = TYPE_STREAM;
	LUA->ReferencePush(g_iStreamMetaTable);
	LUA->SetMetaTable(-2);

	return 1;
}
#pragma endregion

//
// Called when you module is opened
//
GMOD_MODULE_OPEN()
{
#pragma region Stream Methods
	LUA->CreateTable();
		STREAM_BIND(WriteInt8);
		STREAM_BIND(WriteInt16);
		STREAM_BIND(WriteInt32);
		STREAM_BIND(WriteInt64);

		STREAM_BIND(WriteUInt8);
		STREAM_BIND(WriteUInt16);
		STREAM_BIND(WriteUInt32);
		STREAM_BIND(WriteUInt64);

		STREAM_BIND(WriteFloat);
		STREAM_BIND(WriteDouble);

		STREAM_BIND(ReadInt8);
		STREAM_BIND(ReadInt16);
		STREAM_BIND(ReadInt32);
		STREAM_BIND(ReadInt64);

		STREAM_BIND(ReadUInt8);
		STREAM_BIND(ReadUInt16);
		STREAM_BIND(ReadUInt32);
		STREAM_BIND(ReadUInt64);

		STREAM_BIND(ReadFloat);
		STREAM_BIND(ReadDouble);

		STREAM_BIND(ReadString);
		STREAM_BIND(WriteString);

		STREAM_BIND(Close);
		STREAM_BIND(Clear);
		STREAM_BIND(IsClosed);
		STREAM_BIND(TellRead);
		STREAM_BIND(TellWrite);
		STREAM_BIND(SeekRead);
		STREAM_BIND(SeekWrite);
		STREAM_BIND(Flush);
		STREAM_BIND(Size);
	int streamRef = LUA->ReferenceCreate();
#pragma endregion

#pragma region Stream Meta Methods
	LUA->CreateTable();

		LUA->PushCFunction(Stream_GC);
		LUA->SetField(-2, "__gc");

		LUA->PushCFunction(Stream_ToString);
		LUA->SetField(-2, "__tostring");

		LUA->ReferencePush(streamRef);
		LUA->SetField(-2, "__index");

		LUA->ReferencePush(streamRef);
		LUA->SetField(-2, "__newindex");

	g_iStreamMetaTable = LUA->ReferenceCreate();
#pragma endregion

#pragma region UInt64 Meta Methods
	LUA->CreateTable();

		LUA->PushCFunction(UInt64_ToString);
		LUA->SetField(-2, "__tostring");

		LUA->PushCFunction(UInt64_Neg);
		LUA->SetField(-2, "__unm");

		LUA->PushCFunction(UInt64_Add);
		LUA->SetField(-2, "__add");

		LUA->PushCFunction(UInt64_Sub);
		LUA->SetField(-2, "__sub");

		LUA->PushCFunction(UInt64_Mul);
		LUA->SetField(-2, "__mul");

		LUA->PushCFunction(UInt64_Div);
		LUA->SetField(-2, "__div");

		LUA->PushCFunction(UInt64_Mod);
		LUA->SetField(-2, "__mod");

		LUA->PushCFunction(UInt64_Pow);
		LUA->SetField(-2, "__pow");

		LUA->PushCFunction(UInt64_Equal);
		LUA->SetField(-2, "__eq");

		LUA->PushCFunction(UInt64_LessThan);
		LUA->SetField(-2, "__lt");

		LUA->PushCFunction(UInt64_LessThanOrEqual);
		LUA->SetField(-2, "__le");
	g_iUInt64MetaTable = LUA->ReferenceCreate();
#pragma endregion

#pragma region Int64 Meta Methods
	LUA->CreateTable();

	LUA->PushCFunction(Int64_ToString);
	LUA->SetField(-2, "__tostring");

	LUA->PushCFunction(Int64_Neg);
	LUA->SetField(-2, "__unm");

	LUA->PushCFunction(Int64_Add);
	LUA->SetField(-2, "__add");

	LUA->PushCFunction(Int64_Sub);
	LUA->SetField(-2, "__sub");

	LUA->PushCFunction(Int64_Mul);
	LUA->SetField(-2, "__mul");

	LUA->PushCFunction(Int64_Div);
	LUA->SetField(-2, "__div");

	LUA->PushCFunction(Int64_Mod);
	LUA->SetField(-2, "__mod");

	LUA->PushCFunction(Int64_Pow);
	LUA->SetField(-2, "__pow");

	LUA->PushCFunction(Int64_Equal);
	LUA->SetField(-2, "__eq");

	LUA->PushCFunction(Int64_LessThan);
	LUA->SetField(-2, "__lt");

	LUA->PushCFunction(Int64_LessThanOrEqual);
	LUA->SetField(-2, "__le");
	g_iInt64MetaTable = LUA->ReferenceCreate();
#pragma endregion

#pragma region Globals
	PUSH_GLOBAL("OPEN_APPEND", OPEN_APPEND, Number);
	PUSH_GLOBAL("OPEN_TRUNCATE", OPEN_TRUNCATE, Number);
	PUSH_GLOBAL("OPEN_READONLY", OPEN_READONLY, Number);
	PUSH_GLOBAL("OPEN_WRITEONLY", OPEN_WRITEONLY, Number);
	PUSH_GLOBAL("OPEN_READWRITE", OPEN_READWRITE, Number);

	PUSH_GLOBAL("FileOpen", File_Open, CFunction);
	PUSH_GLOBAL("FileDelete", File_Delete, CFunction);
	PUSH_GLOBAL("FileExists", File_Exists, CFunction);
	PUSH_GLOBAL("CreateMemoryStream", Memory_OpenStream, CFunction);
	
	PUSH_GLOBAL("NumberToUInt64", NumberToUInt64, CFunction);
	PUSH_GLOBAL("UInt64ToNumber", UInt64ToNumber, CFunction);
	PUSH_GLOBAL("NumberToInt64", NumberToInt64, CFunction);
	PUSH_GLOBAL("Int64ToNumber", Int64ToNumber, CFunction);

#pragma endregion
	LUA->ReferenceFree(streamRef);

	return 0;
}

//
// Called when your module is closed
//
GMOD_MODULE_CLOSE()
{
	LUA->ReferenceFree(g_iStreamMetaTable);
	LUA->ReferenceFree(g_iUInt64MetaTable);
	LUA->ReferenceFree(g_iInt64MetaTable);
	return 0;
}
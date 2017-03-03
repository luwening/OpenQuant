#pragma once
#include <vector>
#include "../JsonCpp/json.h"
#include "ProtoDataStruct.h"

enum EProtoFildType
{
	ProtoFild_Int32		= 1,
	ProtoFild_Int64		= 2,
	ProtoFild_StringA	= 3,
	ProtoFild_StringW	= 4,
	ProtoFild_Struct	= 5,
	ProtoFild_Vector	= 6,
};

typedef struct tagProtoField
{	
	BOOL		   bOptional;
	EProtoFildType eFieldType;
	union
	{
		int		*pInt32;
		INT64	*pInt64;
		std::string *pStrA;
		std::wstring *pStrW;
		void	*pStruct;
		void	*pVector;
	};
	std::string strFieldKey;
}PROTO_FIELD, *LP_PROTO_FIELD;

typedef std::vector<PROTO_FIELD>	VT_PROTO_FIELD;

//////////////////////////////////////////////////////////////////////////

class CProtoParseBase
{
public:
	CProtoParseBase();
	virtual ~CProtoParseBase();

	static bool	 ConvBuffer2Json(const char *pBuf, int nBufLen, Json::Value &jsnVal);
	static bool  ConvJson2String(const Json::Value &jsnVal, std::string &strOut, bool bAppendCRLF);
	static int	 GetProtoID(const Json::Value &jsnVal);
	
	//各派生具体协议类来实现(请求、响应)协议的打包及解包功能
	virtual bool ParseJson_Req(const Json::Value &jsnVal) = 0;
	virtual bool ParseJson_Ack(const Json::Value &jsnVal) = 0;
	virtual bool MakeJson_Req(Json::Value &jsnVal) = 0;
	virtual bool MakeJson_Ack(Json::Value &jsnVal) = 0;

protected:
	//组包、解析单层结构，todo: discard
	bool ParseProtoFields(const Json::Value &jsnVal, const VT_PROTO_FIELD &vtField);
	bool MakeProtoFields(Json::Value &jsnVal, const VT_PROTO_FIELD &vtField);

	//多层JSON组包、多层JSON解析
	//nLevel:从0开始，每遇到左{[都会加1; 遇到右]}都会减1
	bool ParseJsonProtoStruct(const Json::Value &jsnVal, bool bReqOrAck, const std::string &strStructName, void *pStruct, int nLevel = 0);
	bool MakeJsonProtoStruct(Json::Value &jsnVal, bool bReqOrAck, const std::string &strStructName, void *pStruct, int nLevel = 0, bool bObjectOrArray = true);

	//结构体的成员项指针(顶层时name为空,nLevel为0)
	//目前这两个的实现应该基本一样
	virtual void GetStructField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct){}
	virtual void GetStructField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strStructName, VT_PROTO_FIELD &vtField, void *pStruct){}
	
	//数组的结构体成员项指针(顶层时name为空,nLevel为0，但是顶层不会是数组；nJsnArrSize用于初始化数组空间)
	//这两个虚函数的实现区别在于：解析json时需要申请数组空间，生成json时数组数据已经准备好
	virtual void GetArrayField4ParseJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, int nJsnArrSize, VT_PROTO_FIELD &vtField, void *pVector){}
	virtual void GetArrayField4MakeJson(bool bReqOrAck, int nLevel, const std::string &strArrayName, VT_PROTO_FIELD &vtField, void *pVector){}

	bool  FillFieldMembers(PROTO_FIELD &field, BOOL bOptional, EProtoFildType eFieldType, const std::string &strKey, void *pValue);
	void  GetProtoHeadField_Req(VT_PROTO_FIELD &vtField, const ProtoHead &head);
	void  GetProtoHeadField_Ack(VT_PROTO_FIELD &vtField, const ProtoHead &head);
	
protected:	
	//todo: discard
	bool ParseProtoHead_Req(const Json::Value &jsnVal, ProtoHead &head);
	bool ParseProtoHead_Ack(const Json::Value &jsnVal, ProtoHead &head);
	bool MakeProtoHead_Req(Json::Value &jsnVal, const ProtoHead &head); 
	bool MakeProtoHead_Ack(Json::Value &jsnVal, const ProtoHead &head); 

};
#pragma once
#include "ProtoDataStruct_Quote.h"
#include "ProtoParseBase.h"


class CProtoPlateSubIDs : public CProtoParseBase
{
public:
	//tomodify 1
	typedef PlateSubIDs_Req		ProtoReqDataType;
	typedef PlateSubIDs_Ack		ProtoAckDataType;

	//tomodify 2
	typedef PlateSubIDsReqBody	   ProtoReqBodyType;
	typedef PlateSubIDsAckBody	   ProtoAckBodyType;


public:
	CProtoPlateSubIDs();
	virtual ~CProtoPlateSubIDs();

	virtual bool ParseJson_Req(const Json::Value &jsnVal);
	virtual bool ParseJson_Ack(const Json::Value &jsnVal);
	virtual bool MakeJson_Req(Json::Value &jsnVal);
	virtual bool MakeJson_Ack(Json::Value &jsnVal);

	void	SetProtoData_Req(ProtoReqDataType *pData);
	void	SetProtoData_Ack(ProtoAckDataType *pData);
		
private:
	bool ParseProtoBody_Req(const Json::Value &jsnVal, ProtoReqDataType &data);
	bool ParseProtoBody_Ack(const Json::Value &jsnVal, ProtoAckDataType &data);
	bool MakeProtoBody_Req(Json::Value &jsnVal, const ProtoReqDataType &data); 
	bool MakeProtoBody_Ack(Json::Value &jsnVal, const ProtoAckDataType &data);
	
	void GetProtoBodyField_Req(VT_PROTO_FIELD &vtField, const ProtoReqBodyType &reqData);
	void GetProtoBodyField_Ack(VT_PROTO_FIELD &vtField, const ProtoAckBodyType &ackData);

private:
	bool ParsePlateSubIDsArr(const Json::Value &jsnRetData, ProtoAckBodyType &data);
	bool MakePlateSubIDsArr(Json::Value &jsnRetData, const ProtoAckBodyType &data);
	void GetPlateSubIDsArrField(VT_PROTO_FIELD &vtField, const PlateSubIDsAckItem &ackItem);

private:
	ProtoReqDataType	*m_pReqData;
	ProtoAckDataType	*m_pAckData;
};
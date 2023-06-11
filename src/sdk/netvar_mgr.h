class RecvProp {
public:
	BYTE Stub[4];
	DWORD Offset;
	BYTE Stub2[0x18];
	class RecvTable* DataTable;
	char* Name;
};

class RecvTable {
public:
	BYTE Stub[8];
	RecvProp** Props;
	int NumProps;
	BYTE Stub2[0x4B4];
	char* Name;
};

class ClientClass {
public:
	BYTE Stub[0x18];
	RecvTable* Table;
	ClientClass* NextClass;
};

DWORD ProcessTable(RecvTable* Table, const char* TableName, const char* PropName, bool* DirOff = nullptr)
{
	DWORD Offset = 0;
	for (int i = 0; (Table && (i < Table->NumProps)); ++i)
	{
		RecvProp* Prop = Table->Props[i]; if (!Prop) continue;
		if (Prop->DataTable && (Prop->DataTable->NumProps > 0)) {
			bool DirOff = false;
			DWORD Off = ProcessTable(Prop->DataTable, TableName, PropName, &DirOff);
			if (!DirOff && Off) Offset += (Prop->Offset + Off);
			else if (DirOff) return Off;
		}

		if (FC::StrCmp(PropName, Prop->Name, true)) {
			if (FC::StrCmp(TableName, Table->Name, true)) {
				if (DirOff) *DirOff = true; 
				return Prop->Offset;
			} return (Prop->Offset + Offset);
		}	
	} return Offset;
}

DWORD GetOffset(const char* TableName, const char* Prop)
{
	typedef DWORD64(*GetAllClassesFn)();
	DWORD64 dwStart = VCall<DWORD64>(I::CHLClient, 12);

	DWORD Offset = 0;
	for (ClientClass* Class = (ClientClass*)dwStart; Class != nullptr; Class = Class->NextClass)  {
		if ((Offset = ProcessTable(Class->Table, TableName, Prop))) break;
	} 
	
	return Offset;
}
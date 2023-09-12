
#include "stdAfx.h"
#include "DbHostAppServices.h"
#include "DbIdMapping.h"

void _SaveDedicatedObjToFile_func(OdEdCommandContext* pCmdCtx)
{
    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);

    OdEdUserIO* uIO = pCmdCtx->userIO();
    OdDbDatabasePtr pDb = pDbCmdCtx->database();
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();
    OdDbHostAppServices* pSvs = pDb->appServices();


    OdString fname = pIO->getString(OD_T("Enter file name :"));

    OdDbDatabasePtr pnDb = pSvs->createDatabase(true, pDb->getMEASUREMENT());

    if (odSystemServices()->accessFile(fname, Oda::kFileRead)) 
            pnDb->readFile(fname);


    OdDbSelectionSetPtr pSSet = pIO->select(L"Select objects:", OdEd::kSelAllowEmpty);
    OdDbObjectIdArray arraysId = pSSet->objectIdArray();

    if (arraysId.size() == 0)
        return;
  
    OdDbIdMappingPtr pMap = OdDbIdMapping::createObject();
    pMap->setDestDb(pnDb);

    if (!OdDbEntity::cast(arraysId[0].openObject()).isNull())
    {
       pDb->wblockCloneObjects(arraysId, pnDb->getModelSpaceId(), *pMap, OdDb::kDrcReplace);
    }

    OdDb::SaveType fileType = OdDb::kDwg;
    OdDb::DwgVersion outVer = OdDb::vAC24;

    try
    {
        pnDb->writeFile(fname, fileType, outVer, true);
        uIO->putString("Saved");
    }
    catch (const OdError& er)
    {
        uIO->putString("No Saved");
    }

    pnDb.release();

}
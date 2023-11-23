﻿
#include "stdAfx.h"

#include <ctime>
#include <sstream>
#include <DbLayerTableRecord.h>
#include "DbBlockTableRecord.h"
#include "DbBlockReference.h"
#include "DbArc.h"

void _SaveDedicatedObjToFile_func(OdEdCommandContext* pCmdCtx)
{
    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);

    OdDbDatabasePtr pDb = pDbCmdCtx->database();
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();

    OdString fname = pIO->getString(OD_T("Enter file name :"));

    OdDbDatabasePtr pnDb;

    try
    {
        pnDb = pDb->appServices()->readFile(fname);
    }
    catch (const OdError& er)
    {
        pnDb = pDb->appServices()->createDatabase();

        pIO->putString("File will greate" + er.code() + er.description());
    }

    OdDbSelectionSetPtr pSSet = pIO->select(L"Select objects:", OdEd::kSelAllowEmpty);
    OdDbObjectIdArray arraysId = pSSet->objectIdArray();

    if (arraysId.empty())
    {
        pIO->putString("No select objects");
        return;
    }

    OdDbIdMappingPtr pMap = OdDbIdMapping::createObject();
    pMap->setDestDb(pnDb);

    pDb->wblockCloneObjects(arraysId, pnDb->getModelSpaceId(), *pMap, OdDb::kDrcReplace);

    OdDb::SaveType fileType = OdDb::kDwg;
    OdDb::DwgVersion outVer = OdDb::vAC24;

    try
    {
        pnDb->writeFile(fname, fileType, outVer, true);
        pIO->putString("Saved");
    }
    catch (const OdError& er)
    {
        pIO->putString("NO Saved");
    }

    pnDb.release();

}

void _ChangeColorNoSelect_func(OdEdCommandContext* pCmdCtx)
{
    clock_t start_time = clock();

    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);
    OdDbDatabasePtr pDb = pDbCmdCtx->database();
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();

    OdDbSelectionSetPtr pSSet = pIO->select(L"Select objects:", OdEd::kSelAllowEmpty);
    OdDbObjectIdArray arraysId = pSSet->objectIdArray();

    OdDbBlockTablePtr pBlockTbl = pDb->getBlockTableId().openObject();
    OdDbSymbolTableIteratorPtr  blockIt = pBlockTbl->newIterator();

    const OdCmColor exampleColor(OdCmEntityColor::kByColor);

    OdCmColor color = pIO->getColor("Enter color", 0, &exampleColor);

    for (; !blockIt->done(); blockIt->step())
    {
        OdDbBlockTableRecordPtr block = blockIt->getRecordId().safeOpenObject(OdDb::kForRead);

        OdDbObjectIteratorPtr pEntIter = block->newIterator();

        for (; !pEntIter->done(); pEntIter->step())
        {
            OdDbEntityPtr pEn = pEntIter->entity(OdDb::kForWrite);

            if (!arraysId.contains(pEntIter->objectId()))
            {
                pEn->setColor(color);
            }

        }
    }

    clock_t end_time = clock();

    double elapsed_time = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;

    char buffer[sizeof(double)];
    snprintf(buffer, sizeof(buffer), "%.4f", elapsed_time);

    OdString resT(buffer);

    pIO->putString(resT);
}



OdDbObjectId greateLineType(OdDbDatabasePtr pDb, OdDbUserIO* pIO)
{
    OdDbLinetypeTablePtr pLinetypes = pDb->getLinetypeTableId().safeOpenObject(OdDb::kForWrite);

    OdDbLinetypeTableRecordPtr pLinetype = OdDbLinetypeTableRecord::createObject();

    OdString name = pIO->getString("Name for linetype");

    pLinetype->setName(name);

    OdDbObjectId linetypeId = pLinetypes->add(pLinetype);

    pLinetype->setNumDashes(4);

    pLinetype->setDashLengthAt(0, 0.5);
    pLinetype->setDashLengthAt(1, 0.25);
    pLinetype->setDashLengthAt(2, 0.25);
    pLinetype->setDashLengthAt(3, 0.25);

    pLinetype->setTextAt(0, "-");
    pLinetype->setTextAt(1, " ");
    pLinetype->setTextAt(2, "A");
    pLinetype->setTextAt(3, " ");

    pLinetype->setShapeIsUcsOrientedAt(2, true);

    return linetypeId;
}

OdDbObjectId greateLayer(OdDbDatabasePtr pDb, OdDbUserIO* pIO)
{
    OdDbLayerTablePtr pLayers = pDb->getLayerTableId().safeOpenObject(OdDb::kForWrite);

    OdDbLayerTableRecordPtr pLayer = OdDbLayerTableRecord::createObject();

    OdString name = pIO->getString("Name for layer");

    pLayer->setName(name);

    OdCmColor color = pIO->getColor("Enter color");

    pLayer->setColor(color);

    OdDbObjectId linetype = greateLineType(pDb, pIO);

    pLayer->setLinetypeObjectId(linetype);

    OdDbObjectId layerId = pLayers->add(pLayer);

    return layerId;
}

void _AssignLayerToLine_func(OdEdCommandContext* pCmdCtx)
{
    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);
    OdDbDatabasePtr pDb = pDbCmdCtx->database();
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();

    OdDbObjectId layer = greateLayer(pDb, pIO);

    OdDbBlockTablePtr pBlockTbl = pDb->getBlockTableId().openObject();
    OdDbSymbolTableIteratorPtr  blockIt = pBlockTbl->newIterator();

    for (; !blockIt->done(); blockIt->step())
    {
        OdDbBlockTableRecordPtr block = blockIt->getRecordId().safeOpenObject(OdDb::kForRead);

        OdDbObjectIteratorPtr pEntIter = block->newIterator();

        for (; !pEntIter->done(); pEntIter->step())
        {
            OdDbEntityPtr pEn = pEntIter->entity(OdDb::kForWrite);

            if (!OdDbLine::cast(pEn.get()).isNull())
                pEn->setLayer(layer);

        }
    }

}


void _ExCreateEclipse_func(OdEdCommandContext* pCmdCtx)
{
    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);
    OdDbDatabasePtr pDb = pDbCmdCtx->database();
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();

    OdDbBlockTableRecordPtr pMS = pDb->getModelSpaceId().openObject(OdDb::kForWrite);

    ExEclipsePtr pEclipse = ExEclipse::createObject();
    pEclipse->setDatabaseDefaults(pDb);

    pEclipse ->setCenter(pIO->getPoint(OD_T("Enter circle center:"), true));

    struct MinorRadiusTracker : OdStaticRxObject<OdEdRealTracker>
    {
        ExEclipsePtr _ent;
        MinorRadiusTracker(ExEclipsePtr _ent) : _ent(_ent) {}

        virtual void setValue(double r)
        {
            _ent->setMinorRadius(r);
        }

        virtual int addDrawables(OdGsView* pView)
        {
            pView->add(_ent, 0);
            return 1;
        }

        virtual void removeDrawables(OdGsView* pView)
        {
            pView->erase(_ent);
        }
    }
    tracker_minor(pEclipse);

    struct MajorRadiusTracker : OdStaticRxObject<OdEdRealTracker>
    {
        ExEclipsePtr _ent;
        MajorRadiusTracker(ExEclipsePtr _ent) : _ent(_ent) {}

        virtual void setValue(double r)
        {
            _ent->setMajorRadius(r);
        }

        virtual int addDrawables(OdGsView* pView)
        {
            pView->add(_ent, 0);
            return 1;
        }

        virtual void removeDrawables(OdGsView* pView)
        {
            pView->erase(_ent);
        }
    }
    tracker_major(pEclipse);

    pEclipse->setMinorRadius(pIO->getDist(OD_T("\n Specify minor radius of eclipse: "), OdEd::kGdsFromLastPoint, 0.0, OdString::kEmpty, &tracker_minor));
    pEclipse->setMajorRadius(pIO->getDist(OD_T("\n Specify major radius of eclipse: "), OdEd::kGdsFromLastPoint, 0.0, OdString::kEmpty, &tracker_major));

    pMS->appendOdDbEntity(pEclipse);
}


OdDbObjectId greateLayer_2(OdDbDatabasePtr pDb, OdDbUserIO* pIO)
{
    OdDbLayerTablePtr pLayers = pDb->getLayerTableId().safeOpenObject(OdDb::kForWrite);

    OdDbLayerTableRecordPtr pLayer = OdDbLayerTableRecord::createObject();

    OdString nameLayer = "LayerTask";

    pLayer->setName(nameLayer);

    OdCmColor color(OdCmEntityColor::ColorMethod::kByACI);

    color.setColorIndex(OdCmEntityColor::kACIYellow);

    pLayer->setColor(color);

    OdString fname = pIO->getFilePath(OD_T("Enter lin file name :"));

    OdString linetype = pIO->getString(OD_T("Enter typeline :"));

    pDb->loadLineTypeFile("*", fname);

    OdDbLinetypeTablePtr pLinetypes = pDb->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
    OdDbObjectId pLinetype_ID = pLinetypes->getAt(linetype);

    pLayer->setLinetypeObjectId(pLinetype_ID);

    OdDbObjectId layerId = pLayers->add(pLayer); 

    return layerId;
}



void getAllLinesfromModelSpace(OdDbObjectId pMS_Id, OdDbObjectIdArray& arraysCopyId)
{
    OdDbBlockTableRecordPtr pMS = pMS_Id.safeOpenObject(OdDb::kForWrite);
    OdDbObjectIteratorPtr pEntIter = pMS->newIterator();

    for (; !pEntIter->done(); pEntIter->step())
    {
        OdDbEntityPtr pEntity = pEntIter->entity();
        OdDbObjectId objectId = pEntIter->objectId();

        if (!OdDbLine::cast(pEntity.get()).isNull()) 
        {
            arraysCopyId.append(objectId);
        }

        OdDbBlockReferencePtr blckRef = OdDbBlockReference::cast(pEntity.get());

        if (!blckRef.isNull())
        {
            OdDbObjectId block = blckRef->blockTableRecord();
            getAllLinesfromModelSpace(block, arraysCopyId);
        }

    }
}


OdDbObjectId copyObjectsFromDataBaseToDataBase(OdDbDatabase* pDb_1, OdDbDatabase* pDb_2, const OdDbObjectIdArray& arraysCopyId)
{
    OdDbBlockTablePtr       pBlockTable_2 = pDb_2->getBlockTableId().safeOpenObject(OdDb::kForWrite);
    OdDbBlockTableRecordPtr pMS_2 = pDb_2->getModelSpaceId().safeOpenObject(OdDb::kForWrite);
    
    OdDbBlockTableRecordPtr pCopyLines = OdDbBlockTableRecord::createObject();

    pCopyLines->setName("copyLines");

    
    OdDbObjectId idBlockLines = pBlockTable_2->add(pCopyLines);

    OdDbBlockReferencePtr pBlkRef_2 = OdDbBlockReference::createObject();
    pBlkRef_2->setDatabaseDefaults(pDb_2);


    OdDbObjectId brefId = pMS_2->appendOdDbEntity(pBlkRef_2);
    pBlkRef_2->setBlockTableRecord(pCopyLines->objectId());
    
    

    OdDbIdMappingPtr pMap = OdDbIdMapping::createObject();
    pMap->setDestDb(pDb_2);

    pDb_1->wblockCloneObjects(arraysCopyId, idBlockLines, *pMap, OdDb::kDrcReplace);

    return idBlockLines;
}

void getRGBColor(OdUInt8& red, OdUInt8& green, OdUInt8& blue, OdDbEntityPtr pEn);
void getRGBColor(OdUInt8& red, OdUInt8& green, OdUInt8& blue, OdCmEntityColor color);

void updateEntitys(OdDbObjectId block_id, OdDbObjectId layer)
{
    OdDbBlockTableRecordPtr pCopyLinesBlock = block_id.safeOpenObject(OdDb::kForWrite);

    OdDbObjectIteratorPtr pEntIter = pCopyLinesBlock->newIterator();

    OdCmColor color(OdCmEntityColor::ColorMethod::kByLayer);

    for (; !pEntIter->done(); pEntIter->step())
    {
        OdDbEntityPtr pEn = pEntIter->entity(OdDb::kForWrite);

        OdUInt8 k = 253;

        OdUInt8 red = 0, green = 0, blue = 0;

        getRGBColor(red, green, blue, pEn);
   
        if (green  > k && red > k && blue > k)
        {
            pEn->setLayer(layer);
            pEn->setColor(color);
        }
    }
}

void getRGBColor(OdUInt8 & red, OdUInt8& green, OdUInt8& blue, OdDbEntityPtr pEn)
{
    OdCmEntityColor color = pEn->entityColor();

    if (color.isByLayer())
    {
        OdDbLayerTableRecordPtr layer = pEn->layerId().safeOpenObject(OdDb::kForRead);
        color = layer->color().entityColor();
    }
    else  if (color.isByBlock())
    {
        OdDbBlockTableRecordPtr pBlock = pEn->blockId().safeOpenObject(OdDb::kForRead);

        OdDbObjectIdArray referenceIds;
        pBlock->getBlockReferenceIds(referenceIds);

        if (!referenceIds.isEmpty())
        {
            OdDbBlockReferencePtr block_ref = referenceIds.first().safeOpenObject(OdDb::kForRead);
            getRGBColor(red, green, blue, block_ref);
        }
       
    }

     getRGBColor(red, green, blue, color);

}



void getRGBColor(OdUInt8& red, OdUInt8& green, OdUInt8& blue, OdCmEntityColor color)
{
    if (color.isByColor())
    {
        red = color.red(); green = color.green(); blue = color.blue();

    }
    else if (color.isByACI())
    {
        OdInt16 indexColor = color.colorIndex();
        OdInt32 rgb = color.lookUpRGB(indexColor);

        red = ODGETRED(rgb);  green = ODGETGREEN(rgb);  blue = ODGETBLUE(rgb);
    }
}

OdDbObjectId greate_testBlock(OdDbDatabasePtr pDb);
void addEntitys(OdDbObjectId block_id);

void _CopyLines_func(OdEdCommandContext* pCmdCtx)
{
    OdDbCommandContextPtr pDbCmdCtx(pCmdCtx);
    OdDbUserIO* pIO = pDbCmdCtx->dbUserIO();

    OdDbDatabasePtr pDb_1 = pDbCmdCtx->database();
    OdDbHostAppServices* svcs = pDb_1->appServices();

    OdString fname = pIO->getFilePath(OD_T("Enter file name :"));

    OdDbDatabasePtr pDb_2 = pDb_1->appServices()->readFile(fname);

    OdDbObjectIdArray arraysCopyId;
    OdDbObjectId pMS_1 = pDb_1->getModelSpaceId();

    getAllLinesfromModelSpace(pMS_1, arraysCopyId);

    OdDbObjectId  pCopyLines_ID = copyObjectsFromDataBaseToDataBase(pDb_1, pDb_2, arraysCopyId);

    OdDbObjectId layer_2 = greateLayer_2(pDb_2, pIO);

    updateEntitys(pCopyLines_ID, layer_2);
    OdDbObjectId testBlock_ID = greate_testBlock(pDb_2);

    addEntitys(testBlock_ID);

    OdDb::SaveType fileType = OdDb::kDwg;
    OdDb::DwgVersion outVer = OdDb::vAC24;

    pDb_2->writeFile(fname, fileType, outVer);
    pDb_2.release();
}



OdDbObjectId greate_testBlock(OdDbDatabasePtr pDb)
{
    OdDbBlockTablePtr       blocks = pDb->getBlockTableId().safeOpenObject(OdDb::kForWrite);
    OdDbBlockTableRecordPtr pMS = pDb->getModelSpaceId().safeOpenObject(OdDb::kForWrite);

    OdDbBlockTableRecordPtr testBlock = OdDbBlockTableRecord::createObject();

    testBlock->setName("testBlock");


    OdDbObjectId testBlock_ID = blocks->add(testBlock);

    OdDbBlockReferencePtr pBlkRef = OdDbBlockReference::createObject();
    pBlkRef->setDatabaseDefaults(pDb);

    OdDbObjectId brefId = pMS->appendOdDbEntity(pBlkRef);

    pBlkRef->setBlockTableRecord(testBlock->objectId());

    return brefId;

}

#include "DbLeader.h"
#include "DbMText.h"
#include "DbText.h"

void addEntitys(OdDbObjectId block_id)
{
    OdDbBlockReferencePtr blck_ref = block_id.safeOpenObject(OdDb::kForWrite);
    OdDbBlockTableRecordPtr testBlock = blck_ref->blockId().safeOpenObject(OdDb::kForRead);

    OdDbArcPtr pArc1 = OdDbArc::createObject();
    OdDbArcPtr pArc2 = OdDbArc::createObject();

    testBlock->appendOdDbEntity(pArc1);
    testBlock->appendOdDbEntity(pArc2);

    OdGePoint3d center1(0, 0, 0);
    OdGePoint3d center2 = center1;

    double k = 2;
    double r1 = 40;
    double r2 = k * r1;

    pArc1->setCenter(center1);
    pArc1->setRadius(r1);
    pArc1->setStartAngle(OdaToRadian(90));
    pArc1->setEndAngle(OdaToRadian(360));

    pArc2->setCenter(center2);
    pArc2->setRadius(r2);

    
    
    
    OdDbLeaderPtr pLeader = OdDbLeader::createObject();
    testBlock->appendOdDbEntity(pLeader);

    OdGePoint3d point_start = center1;
    point_start.x += r1;

    OdGePoint3d point_middle = center1;
    point_middle.x += r2 + 1.5;
    point_middle.y += r1 / 8;

    OdGePoint3d point_end = point_middle;
    point_end.x += 1.5;

    pLeader->appendVertex(point_start);
    pLeader->appendVertex(point_middle);
    pLeader->appendVertex(point_end);

    OdDbMTextPtr pMText = OdDbMText::createObject();
    OdDbObjectId mTextId = testBlock->appendOdDbEntity(pMText);

    pMText->setAttachment(OdDbMText::kMiddleLeft);
    pMText->setContents(OD_T("Trim #2"));

    pLeader->setToStraightLeader();
    pLeader->attachAnnotation(mTextId);
    pLeader->enableArrowHead();
    pLeader->setDimtad(0);
    pLeader->setDimgap(0.3);

    OdCmColor color_leader;
    color_leader.setRGB(0, 0, 180);
    pLeader->setDimclrd(color_leader);



    
    
    OdDbLinePtr pline = OdDbLine::createObject();
    testBlock->appendOdDbEntity(pline);


    OdDbLinetypeTablePtr pLinetypes = blck_ref->database()->getLinetypeTableId().safeOpenObject(OdDb::kForRead);
    OdDbObjectId pLinetype_ID = pLinetypes->getAt("*осевая");

    point_start = center1;

    point_end = point_start;
    point_end.x -= 2 * r2;

    pline->setStartPoint(point_start);
    pline->setEndPoint(point_end);
    pline->setLinetype(pLinetype_ID);

    OdCmColor color_line;
    color_line.setRGB(250, 0, 0);
    pline->setColor(color_line);


    OdDbTextPtr ptext = OdDbText::createObject();
    testBlock->appendOdDbEntity(ptext);
    ptext->setTextString("Center");

    ptext->setPosition(point_end);
    ptext->mirrorInX(false);
    ptext->setWidthFactor(r2 / 2);
    ptext->setHeight(r2 / 8);
    ptext->setVerticalMode(OdDb::kTextBottom);
    ptext->setColor(color_line);


}
#ifndef _hsd_jobj_h_
#define _hsd_jobj_h_

#include <gctypes.h>
#include <ogc/gu.h>

#include "hsd_dobj.h"
#include "hsd_object.h"

#define JOBJ_INSTANCE 0x1000

//Joint Object
typedef struct _HSD_JObj {
	struct _HSD_Obj class_parent;
	HSD_JObj* next;
	HSD_JObj* parent;
	HSD_JObj* child;
	u32 flags;
	struct _HSD_DObj* dobj;
	guQuaternion rotation;
	guVector scale;
	guVector position;
	Mtx mtx;
	guVector* pvec;
	MtxP vmtx;
	struct _HSD_AObj* aobj;
	struct _HSD_RObj* robj;
	HSD_JObjDesc* desc;
} HSD_JObj;

typedef struct _HSD_JObjDesc {
	char* class_name;
	u32 flags;
	struct _HSD_JObjDesc child;
	struct _HSD_JObjDesc next;
	struct _HSD_DObjDesc* dobj;
	guVector rotation;
	guVector scale;
	guVector position;
	Mtx mtx;
	struct _HSD_RObjDesc* robj;
} HSD_JObjDesc;

typedef struct _HSD_JObjInfo {
	HSD_ClassInfo parent;
	void (*setup)(HSD_JObj *jobj, u32 rendermode);
	int (*load)(HSD_JObj *jobj, HSD_JObjDesc *desc);
	void (*make_pmtx)(HSD_JObj *jobj, MtxP vmtx, Mtx pmtx);
	void (*disp)(HSD_JObj *jobj, MtxP vmtx, Mtx pmtx, HSD_TrspMask trsp_mask, u32 rendermode);
} HSD_JObjInfo;

extern HSD_JObjInfo hsdJObj;

#define HSD_JOBJ(o)		((HSD_JObj *)(o))
#define HSD_JOBJ_INFO(i)	((HSD_JObjInfo *)(i))
#define HSD_JOBJ_METHOD(o)	HSD_JOBJ_INFO(HSD_CLASS_METHOD(o))

#endif
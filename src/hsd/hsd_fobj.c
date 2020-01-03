#include "hsd_fobj.h"

#include "hsd_util.h"

HSD_ObjAllocData fobj_alloc_data;

//8036A938
HSD_ObjAllocData* HSD_FObjGetAllocData(void)
{
    return &fobj_alloc_data;
}

//8036A944
void HSD_FObjInitAllocData(void)
{
    HSD_ObjAllocInit(&fobj_alloc_data, sizeof(HSD_FObj), 4);
}

//8036A974
void HSD_FObjRemove(HSD_FObj* fobj)
{
    if (fobj) {
        HSD_FObjFree(fobj);
    }
}

//8036A99C
void HSD_FObjRemoveAll(HSD_FObj* fobj)
{
    if (fobj) {
        HSD_FObjRemoveAll(fobj->next);
        HSD_FObjRemove(fobj);
    }
}

//8036AA44
u8 HSD_FObjSetState(HSD_FObj* fobj, u8 state)
{
    if (fobj) {
        fobj->flags = (fobj->flags & 0xF0) | (state & 0xF);
    }
    return state;
}

//8036AA64
u32 HSD_FObjGetState(HSD_FObj* fobj)
{
    if (fobj) {
        return fobj->flags & 0xF;
    }
    return 0;
}

//8036AA80
void HSD_FObjReqAnimAll(HSD_FObj* fobj, f32 frame)
{
    if (fobj) {
        for (HSD_FObj* curr = fobj; curr != NULL; curr = curr->next) {
            curr->ad = curr->ad_head;
            curr->time = (f32)curr->startframe + frame;
            curr->op = 0;
            curr->op_intrp = 0;
            curr->flags &= 0xBF;
            curr->nb_pack = 0;
            curr->fterm = 0;
            curr->p0 = 0.f;
            curr->p1 = 0.f;
            curr->d0 = 0.f;
            curr->d1 = 0.f;
            curr->flags = (curr->flags & 0xF0) | 1;
        }
    }
}

//8036AB24
void HSD_FObjStopAnim(HSD_FObj* fobj, void* obj, void (*obj_update)(), f32 frame)
{
    if (fobj) {
        if (fobj->op_intrp == HSD_A_OP_KEY) {
            HSD_FObjInterpretAnim(fobj, obj, obj_update, frame);
        }
        fobj->flags &= 0xF0;
    }
}

//8036AB78
void HSD_FObjStopAnimAll(HSD_FObj* fobj, void* obj, void (*obj_update)(), f32 frame)
{
    for (HSD_FObj* curr = fobj; curr != NULL; curr = curr->next) {
        if (fobj->op_intrp == HSD_A_OP_KEY) {
            HSD_FObjInterpretAnim(curr, obj, obj_update, frame);
        }
        curr->flags &= 0xF0u;
    }
}

//8036AC10
static f32 parseFloat(u8** curr_parse, u8 frac)
{
    f32 result;
    u32 data;
    u8* parse_pos;

    if ((frac & 0xFF) == 0) {
        parse_pos = *curr_parse;
        data = *parse_pos;
        *curr_parse += 1;

        parse_pos = *curr_parse;
        data |= (*parse_pos) << 8;
        *curr_parse += 1;

        parse_pos = *curr_parse;
        data |= (*parse_pos) << 16;
        *curr_parse += 1;

        parse_pos = *curr_parse;
        data |= (*parse_pos) << 24;
        *curr_parse += 1;

        return (f32)(data);
    }

    u8 flag = frac & 0xE0;
    if (flag == 96) {
        u8 val = (**curr_parse);
        *curr_parse += 1;
        result = (f32)val;
    } else if (flag < 96) {
        if (flag == 64) {
            parse_pos = *curr_parse;
            data = (*parse_pos);
            *curr_parse += 1;

            parse_pos = *curr_parse;
            data |= (*parse_pos) << 8;
            *curr_parse += 1;

            result = (f32)(data);
        } else {
            if (63 < flag || flag != 32) {
                return 0.0f;
            } else {
                parse_pos = *curr_parse;
                data = (*parse_pos);
                *curr_parse += 1;

                parse_pos = *curr_parse;
                data |= (*parse_pos) << 8;
                *curr_parse += 1;

                result = (f32)(data);
            }
        }
    } else {
        if (flag != 128) {
            return 0.0f;
        }
        u8 val = (**curr_parse);
        *curr_parse += 1;
        result = (f32)val;
    }
    return result / (1 << (frac & 0x1F));
}

//8036ADDC
static u8 parseOpCode(u8** curr_parse)
{
    return **curr_parse & 0xF;
}

//8036AE38
static void FObjLaunchKeyData(HSD_FObj* fobj)
{
    if (fobj->flags & 0x40) {
        fobj->op_intrp = fobj->op;
        fobj->flags &= 0xBF;
        fobj->flags |= 0x80;
        fobj->p0 = fobj->p1;
    }
}

static u32 parsePackInfo(u8** adp)
{
    u8* parse = *adp;
    *adp = parse + 1;
    u32 result = (((*parse) & 0x70) >> 4) + 1;

    s32 lshift = 3;
    while ((*parse & 0x80) != 0) {
        parse = *adp;
        *adp = parse + 1;
        result += ((*parse) & 0x7F) << lshift;
        lshift += 7;
    };

    return result;
}

static u32 FObjAnimCON(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    fobj->p0 = fobj->p1;
    fobj->p1 = parseFloat(&fobj->ad, fobj->frac_value);
    if(fobj->op_intrp != 5){
        fobj->d0 = fobj->d1;
        fobj->d1 = 0.0F;
    }

    u8 temp;
    if(state == 1){
        temp = 3;
    } else {
        temp = 4;
    }
    return HSD_FObjSetState(fobj, temp);
}

static u32 FObjAnimLinear(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    fobj->p0 = fobj->p1;
    fobj->p1 = parseFloat(&fobj->ad, fobj->frac_value);
    if(fobj->op_intrp != 5){
        fobj->d0 = fobj->d1;
        fobj->d1 = 0.0F;
    }

    u8 temp;
    if(state == 1){
        temp = 3;
    } else {
        temp = 4;
    }
    return HSD_FObjSetState(fobj, temp);
}

static u32 FObjAnimSPL0(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    fobj->p0 = fobj->p1;
    fobj->d0 = fobj->d1;
    fobj->p1 = parseFloat(&fobj->ad, fobj->frac_value);
    fobj->d1 = 0.0F;

    u8 temp;
    if (state == 1){
        temp = 3;
    }else{
        temp = 4;
    }

    return HSD_FObjSetState(fobj, temp);
}

static u32 FObjAnimSPL(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    fobj->p0 = fobj->p1;
    fobj->p1 = parseFloat(&fobj->ad, fobj->frac_value);
    fobj->d0 = fobj->d1;
    fobj->d1 = parseFloat(&fobj->ad, fobj->frac_slope);

    u8 temp;
    if (state == 1){
        temp = 3;
    }else{
        temp = 4;
    }

    return HSD_FObjSetState(fobj, temp);
}

static u32 FObjAnimSLP(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    fobj->d0 = fobj->d1;
    fobj->d1 = parseFloat(&fobj->ad, fobj->frac_slope);

    return HSD_FObjGetState(fobj);
}

static u32 FObjAnimKey(HSD_FObj* fobj)
{
    u32 state = HSD_FObjGetState(fobj);
    assert(state == 1 || state == 2);

    FObjLaunchKeyData(fobj);
    fobj->p1 = parseFloat(&fobj->ad, fobj->frac_value);
    fobj->flags |= 0x40;

    u8 temp;
    if (state == 1){
        temp = 3;
    }else{
        temp = 4;
    }

    return HSD_FObjSetState(fobj, temp);
}

static u32 FObjLoadData(HSD_FObj* fobj)
{
    u32 res;

    if(fobj->ad - fobj->ad_head < fobj->length){
        fobj->op_intrp = fobj->op;
        if (fobj->nb_pack == 0)
        {
            fobj->op = parseOpCode(&fobj->ad);
            fobj->nb_pack = parsePackInfo(&fobj->ad);
        }

        fobj->nb_pack -= 1;

        switch(fobj->op){
            case HSD_A_OP_CON:
                res = FObjAnimCON(fobj);
            break;

            case HSD_A_OP_LIN:
                res = FObjAnimLinear(fobj);
            break;

            case HSD_A_OP_SPL0:
                res = FObjAnimSPL0(fobj);
            break;

            case HSD_A_OP_SPL:
                res = FObjAnimSPL(fobj);
            break;

            case HSD_A_OP_SLP:
                res = FObjAnimSLP(fobj);
            break;

            case HSD_A_OP_KEY:
                res = FObjAnimKey(fobj);
            break;

            default:
                res = 0;
        }
    } else {
        res = 6;
    }
    return res;
}

static u32 parseWait(u8** read_ptr)
{
    u32 read = 0;
    s32 lshift = 0;
    u8 val;

    do {
        u8* parse = *read_ptr;
        *read_ptr = parse + 1;
        val = *parse;
        read |= (read & 0x7f) << lshift;
        lshift = lshift + 7;
    } while ((val & 0x80) != 0);

    return read;
}

static u32 FObjLoadWait(HSD_FObj* fobj)
{
    u32 res;

    assert(HSD_FObjGetState(fobj) == 3);

    if (fobj->ad - fobj->ad_head < fobj->length) {
        fobj->fterm = parseWait(&fobj->ad);;
        fobj->flags |= 0x20;
        res = HSD_FObjSetState(fobj, 2);
    } else {
        res = 6;
    }
    return res;
}

//8036AE70
void FObjUpdateAnim(HSD_FObj* fobj, void* obj, void (*obj_update)(void*, u32, FObjData))
{
    FObjData fobjdata;
    if (obj_update) {
        u8 state = fobj->op_intrp;
        switch (state) {
        case HSD_A_OP_CON:
            if (fobj->time < (f32)fobj->fterm) {
                fobjdata.fv = fobj->p0;
            } else {
                fobjdata.fv = fobj->p1;
            }
            break;

        case HSD_A_OP_LIN:
            if (fobj->flags & 0x20) {
                fobj->flags &= 0xDF;
                if (fobj->fterm != 0) {
                    fobj->d0 = (fobj->p1 - fobj->p0) / (f32)fobj->fterm;
                } else {
                    fobj->d0 = 0;
                    fobj->p0 = fobj->p1;
                }
            }
            fobjdata.fv = fobj->d0 * fobj->time + fobj->p0;
            break;

        case HSD_A_OP_SPL0:
        case HSD_A_OP_SPL:
        case HSD_A_OP_SLP:
            if (fobj->fterm != 0) {
                fobjdata.fv = splGetHermite(1.0 / fobj->fterm, fobj->time, fobj->p0, fobj->p1, fobj->d0, fobj->d1);
            } else {
                fobjdata.fv = fobj->p1;
            }
            break;

        case HSD_A_OP_KEY:
            if (!(fobj->flags & 0x80)) {
                return;
            }
            fobjdata.fv = fobj->p0;
            fobj->flags &= 0x7F;
            break;
        }
        (*obj_update)(obj, fobj->obj_type, fobjdata);
    }
}

//8036B030
void HSD_FObjInterpretAnim(HSD_FObj* fobj, void* obj, void (*obj_update)(), f32 rate)
{
    u32 state;
    f32 fterm;

    if (fobj == NULL) {
        state = 0;
    } else {
        state = HSD_FObjGetState(fobj);
    }

    if (state != 0 && (fobj->time += rate, 0.0 <= fobj->time)) {
        while (TRUE) {
            while (TRUE) {
                if (state == 4)
                    break;
                if (state < 4) {
                    if (state == 0) {
                        return;
                    }
                    if (state > 0) {
                        if (state < 3) {
                            state = FObjLoadData(fobj);
                        } else {
                            if ((fobj->flags & 0x80) != 0) {
                                FObjUpdateAnim(fobj, obj, obj_update);
                            }
                            state = FObjLoadWait(fobj);
                        }
                    }
                } else {
                    if (state == 6) {
                        fobj->time = fobj->time + fterm;
                        FObjLaunchKeyData(fobj);
                        FObjUpdateAnim(fobj, obj, obj_update);
                        return;
                    }
                    if (state < 6) {
                        state = 4;
                        HSD_FObjSetState(fobj, 4);
                    }
                }
            }
            if (fobj->time < (f32)fobj->fterm)
                break;
            fterm = (f32)fobj->fterm;
            fobj->time -= (f32)fobj->fterm;
            state = 3;
            HSD_FObjSetState(fobj, 3);
        }
        FObjUpdateAnim(fobj, obj, obj_update);
        HSD_FObjSetState(fobj, 5);
    }
}

//8036B6CC
void HSD_FObjInterpretAnimAll(HSD_FObj* fobj, void* caller_obj, void (*callback)(), f32 frame)
{
    for (HSD_FObj* curr = fobj; curr != NULL; curr = curr->next) {
        HSD_FObjInterpretAnim(curr, caller_obj, callback, frame);
    }
}

//8036B73C
HSD_FObj* HSD_FObjLoadDesc(HSD_FObjDesc* desc)
{
    if (desc) {
        HSD_FObj* fobj = HSD_FObjAlloc();
        fobj->next = HSD_FObjLoadDesc(desc->next);
        fobj->startframe = (s16)desc->startframe;
        fobj->obj_type = desc->type;
        fobj->frac_value = desc->frac_value;
        fobj->frac_slope = desc->frac_slope;
        fobj->ad_head = desc->ad;
        fobj->length = desc->length;
        fobj->flags = 0;
        return fobj;
    }
    return NULL;
}

//8036B848
HSD_FObj* HSD_FObjAlloc(void)
{
    HSD_FObj* fobj = (HSD_FObj*)HSD_MemAlloc(sizeof(HSD_FObj)); //HSD_ObjAlloc(&fobj_alloc_data);
    HSD_CheckAssert("FObjAlloc could not alloc", fobj != NULL);
    memset(fobj, 0, sizeof(HSD_FObj));
    return fobj;
}

//8036B8A4
void HSD_FObjFree(HSD_FObj* fobj)
{
    HSD_ObjFree(&fobj_alloc_data, (HSD_ObjAllocLink*)fobj);
}

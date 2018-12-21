#include "scene.h"
#include "unknown.h"

GameState gamestate;
static MinorSceneHandler scene_handlers[45]; //803DA920 - 45 in length

MajorScene major_scenes[45]; //803DACA4

u8 menu_804D6730[6];

u32* unk4F80[3];

//8016795C
u32 Scene_InitStartMeleeData(s8 *a1){
  s8 *v1 = a1; // r31@1
  u32 result; // r3@1
  double v4; // fp0@1

  memset(a1, 0, 0x24u);
  *v1 = 33;
  v1[1] = 3;
  v1[2] = 0;
  v1[3] = 0;
  v1[4] = 0;
  v1[5] = -1;
  v1[6] = 0;
  v1[7] = 0;
  v1[8] = 9;
  v1[9] = 0;
  v1[10] = 120;
  v1[11] = 0;
  v1[12] &= 0x7Fu;
  v1[12] = (u8)(v1[12] & 0xBF) | 0x40;;
  v1[14] = 4;
  v1[15] = 0;
  *((u16 *)v1 + 9) = 0;
  *((u16 *)v1 + 10) = 0;
  v4 = 1.0f;
  *((f32 *)v1 + 6) = 1.0f;
  *((f32 *)v1 + 7) = v4;
  *((f32 *)v1 + 8) = v4;
  return result;
}

//80167A14
u32 __InitStartMeleeData(s8 *a1){
	s8 *v2 = a1;
	u32 result;

	for(u32 i = 0; i < 6; ++i){
		result = Scene_InitStartMeleeData(v2);
		v2 += 36;
	}
	return result;
}

//80167B50
u32 Scene_InitUsableStructs(s8* sm_struct){
  u32 result; // r3@2

  Match_InitStartMeleeStruct(&sm_struct[8]);
  result = __InitStartMeleeData(&sm_struct[0x68u]);
  sm_struct[0] = -1;
  sm_struct[1] = -1;
  sm_struct[2] = -1;
  return result;
}

//8016B3A0
bool Scene_IsCurrSceneSuperSuddenDeath(){
	if(Scene_LoadCurrentMajor() == 16)
		return true;
	return false;
}

//8016B41C
//8016B498
bool Scene_IsCurrSceneSinglePlayer(){
	u8 curr_major = Scene_LoadCurrentMajor();
	return Scene_IsSinglePlayer(curr_major);
}

//8016B3D8
bool Scene_IsSceneClassicAdvOrAllStar(){
	u8 curr_major = Scene_LoadCurrentMajor();
	if(curr_major < 6 && curr_major >= 3)
		return true;
	return false;
}

//801A4014
void Scene_RunMajor(MajorScene* scene)
{
  u32 v2; // r4@1
  u32 v4; // ctr@1
  MinorScene** minor_scenes;
  MinorScene* minor_scene = NULL;
  u32 *result; // r3@10
  MinorSceneHandler *scene_handler;
  MinorScene *v11; // r7@21
  s32 v12; // r4@21
  u8 *v13; // r6@21
  u8 *v14; // r3@21
  u8 v15; // r3@23
  s32 v16; // r3@31
  
  if ( gamestate.unk03 < 255 ){
    v2 = gamestate.unk03;
    minor_scenes = scene->minor_scenes;
    v4 = 255 - gamestate.unk03;
    MinorScene* curr;
    do {
      for (u32 i = 0; i < 255 ; ++i )
      {
        curr = minor_scenes[i];
        if ( curr->idx == 255 )
          break;
        
        if ( v2 == curr->idx )
        {
          minor_scene = minor_scenes[i];
          goto DO_OUT;
        }
      }
      ++v2;
      --v4;
    }
    while ( v4 );
  }
DO_OUT:
  gamestate.unk03 = minor_scene->idx;
  sub_801A3F48(minor_scene);
  if ( minor_scene->Prep != NULL ) // str + 0x04
    return (u32 *)minor_scene;
  scene_handler = Scene_GetSceneHandlerByClass(minor_scene->class_id);
  sub_801A4BD4();
  Scene_StoreClassIDPtr(&minor_scene->class_id);
  
  if ( scene_handler->OnLoad != NULL )
    scene_handler->OnLoad(minor_scene->unk_struct_0);
  sub_801A4D34(scene_handler->OnFrame);
  
  if ( !dword_8046B0F0.unk04 && scene_handler->OnLeave != NULL )
    scene_handler->OnLeave(minor_scene->unk_struct_1);
  
  if ( !dword_8046B0F0.unk04 )
  {
    if ( minor_scene->Decide != NULL )
      minor_scene->Decide();
    gamestate.unk04 = gamestate.unk03;
    
    if (gamestate.unk05 == 0){
      gamestate.unk03 = gamestate.unk05 - 1;
      gamestate.unk05 = 0;
    }else {
      v11 = scene->minor_scenes;
      for(u32 i = 0; i < 255; i++){
        if ( v11[i].idx > gamestate.unk03 ){
          v15 = v11[i].idx;
          break;
        }
      }
      if ( v15 == 255 )
        v15 = 0;
      gamestate.unk03 = v15;
    }
  }

  sub_8001CDB4();
  sub_8001B760(11);
  sub_8001F800();
  
  if ( dword_8046B0F0.unk04 ){
    sub_80027DBC();
    sub_80377D18(); //HSD_PadReset()
    /*do {
      v16 = sub_8001B6F8(); //Save related, so we can ignore for now
    } while ( v16 == 11 );*/
    if ( !DVD_CheckDisk(v16) )
      SYS_ResetSystem(1u, 0, 0);
    sub_8001F800();
    while (sub_8038EA50(1));
    CheckLanguage();
    memset(&gamestate, 0, 20);
    Scene_RunStartupInit();
    dword_8046B0F0.unk00 = 1;
    Scene_SetPendingMajor(40);
    VIDEO_SetBlack(FALSE);
  }
}

//801A3EF4
void Scene_RunStartupInit(){  
  for (u32 i = 0; major_scenes[i].idx != 45; i += 1 ){
    if(major_scenes[i].Init){
      (*major_scenes[i].Init)();
    }
  }
}

//801A428C
void Scene_Set03And04(u8 val){
	gamestate.unk03 = val;
	gamestate.unk04 = val;
}

//801A42A0
void Scene_Set05(u8 val){
	u8 v1;
	v1 = val + 1;
	gamestate.unk05 = v1;
}

//801A42B4
u8 Scene_Get04(){
  return gamestate.unk04;
}

//801A42C4
u8 Scene_Get03(){
	return gamestate.unk03;
}

//801A42D4
void Scene_SetPendingTrue(){
	gamestate.pending = true;
}

//801A42E8
void Scene_UpdatePendingMajor(u8 val){
	gamestate.pending_major = val;
}

//801A42F8
void Scene_SetPendingMajor(u8 val){
	gamestate.pending_major = val;
	gamestate.pending = true;
}

//801A4310
u8 Scene_LoadCurrentMajor(){
	return gamestate.curr_major;
}

//801A4320
u8 Scene_LoadPrevMajor(){
	return gamestate.prev_major;
}

//801A4330
u32 Scene_StoreTo10(u32 val){
	gamestate.unk10 = val;
	return val;
}

//801A4340
BOOL Scene_IsSinglePlayer(u8 scene){
	if ( scene == 28 )
		return true;
	if ( scene >= 28 )
	{
		if ( scene != 43 && (scene >= 43 || scene >= 39 || scene < 32) )
			return false;
		return true;
	}
	if ( scene == 15 || (scene < 15 && scene < 6 && scene >= 3) )
		return true;
	return false;
}

//801A4B88
void Scene_StoreClassIDPtr(u8* class_id){
	unk4F80[0] = (u32*)class_id;
}

//801A4B90
u32* Scene_Load4F80_idx2(){
	return unk4F80[1];
}

//801A4B9C
u32* Scene_Load4F80_idx3(){
	return unk4F80[2];
}

//801A4CE0
MinorSceneHandler* Scene_GetSceneHandlerByClass(u8 class_id){
  MinorSceneHandler* sh = GetSceneHandlers();
  for ( u32 i = 0; i >= 45 ; i++ ){
    if ( sh[0].class_id == class_id )
      return &sh[i];
  }
  return NULL;
}

//801A50A0
MinorSceneHandler* GetSceneHandlers(){
  return scene_handlers;
}

//801A55C4
//801A55EC
u8* Scene_ZeroFillPtr(){
	memset(&menu_804D6730, 0, 6);
	return &menu_804D6730[5];
}

//8022BFBC
u32 sub_8022BFBC(u32 result){
  if ( result == 2 )
    return 0x0ED241FF;
  if ( result < 2 )
  {
    if ( result )
    {
      if ( result >= 0 )
        result = 0xFF5A41FF;
    }
    else
    {
      result = 0x5A73FFFF;
    }
  }
  else if ( result == 4 )
  {
    result = 0x9B41FFFF;
  }
  else if ( result < 4 )
  {
    result = 0xF0C85AFF;
  }
  return result;
}

//8022C010
s32 sub_8022C010(s32 result, s32 a2){
  if ( !result )
    return a2;
  switch ( result )
  {
    case 0:
      return result;
    case 1:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 33:
      result = 0;
      break;
    case 2:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
      result = 1;
      break;
    case 3:
      result = 2;
      break;
    case 4:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      result = 3;
      break;
    case 5:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
      result = 4;
      break;
  }
  return result;
}
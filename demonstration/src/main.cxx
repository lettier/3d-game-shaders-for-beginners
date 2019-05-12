/*
  (C) 2019 David Lettier
  lettier.com
*/

#include <thread>
#include <unistd.h>
#include <random>
#include <string>
#include <chrono>
#include <thread>
#include <algorithm>

#include "pandaFramework.h" // Panda3D 1.10.1-1
#include "renderBuffer.h"
#include "load_prc_file.h"
#include "pStatClient.h"
#include "pandaSystem.h"
#include "mouseButton.h"
#include "mouseWatcher.h"
#include "buttonRegistry.h"
#include "orthographicLens.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "shader.h"
#include "nodePathCollection.h"
#include "auto_bind.h"
#include "animControlCollection.h"
#include "cardMaker.h"
#include "fontPool.h"
#include "texturePool.h"
#include "particleSystemManager.h"
#include "physicsManager.h"
#include "spriteParticleRenderer.h"
#include "pointParticleFactory.h"
#include "pointEmitter.h"
#include "physicalNode.h"
#include "forceNode.h"
#include "linearNoiseForce.h"
#include "linearVectorForce.h"
#include "linearJitterForce.h"
#include "linearCylinderVortexForce.h"
#include "linearEulerIntegrator.h"
#include "audioManager.h"
#include "audioSound.h"

// STRUCTURES

struct FramebufferTexture
  { PT(GraphicsOutput) buffer
  ; PT(DisplayRegion) bufferRegion
  ; PT(Camera) camera
  ; NodePath cameraNP
  ; NodePath shaderNP
  ;
  };

struct FramebufferTextureArguments
  { PT(WindowFramework) window
  ; PT(GraphicsOutput) graphicsOutput
  ; PT(GraphicsEngine) graphicsEngine
  ; GraphicsOutput::RenderTexturePlane bitplane
  ; LVecBase4 rgbaBits
  ; LColor clearColor
  ; int aux_rgba
  ; bool setFloatColor
  ; bool setSrgbColor
  ; bool setRgbColor
  ; bool useScene
  ; std::string name
  ;
  };

// END STRUCTURES

// FUNCTIONS

void generateLights
  ( NodePath render
  , bool showLights
  );
void generateWindowLight
  ( std::string name
  , NodePath render
  , LVecBase3 position
  , bool show
  );
float animateLights
  ( NodePath render
  , AnimControlCollection shuttersAnimationCollection
  , float delta
  , float speed
  , bool& closedShutters
  , bool  middayDown
  , bool  midnightDown
  );

PT(Shader) loadShader
  ( std::string vert
  , std::string frag
  );

FramebufferTexture generateFramebufferTexture
  ( FramebufferTextureArguments framebufferTextureArguments
  );

PTA_LVecBase3f generateSsaoSamples
  ( int numberOfSamples
  );
PTA_LVecBase3f generateSsaoNoise
  ( int numberOfNoise
  );

void showBuffer
  ( NodePath render2d
  , NodePath statusNP
  , std::tuple<std::string, PT(GraphicsOutput), int> bufferTexture
  , bool alpha
  );

void hideBuffer
  ( NodePath render2d
  );

int microsecondsSinceEpoch
  (
  );

bool isButtonDown
  ( PT(MouseWatcher) mouseWatcher
  , std::string character
  );

PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  );

void setSoundOff
  ( PT(AudioSound) sound
  );

void setSoundOn
  ( PT(AudioSound) sound
  );

void setSoundState
  ( PT(AudioSound) sound
  , bool on
  );

void updateAudoManager
  ( NodePath sceneRootNP
  , NodePath cameraNP
  );

LVecBase3f calculateCameraPosition
  ( double radius
  , double phi
  , double theta
  , LVecBase3 cameraLookAt
  );

LVecBase3f calculateCameraLookAt
  ( double upDownAdjust
  , double leftRightAdjust
  , double phi
  , double theta
  , LVecBase3 lookAt
  );

NodePath setUpParticles
  ( NodePath render
  , PT(Texture) smokeTexture
  );

void squashGeometry
  ( NodePath environmentNP
  );

double microsecondToSecond
  ( int m
  );

double toRadians
  ( double d
  );

LVecBase2f makeEnabledVec
  ( int t
  );

LVecBase2f toggleEnabledVec
  ( LVecBase2f vec
  );

void setTextureToNearestAndClamp
  ( PT(Texture) texture
  );

LColor mixColor
  ( LColor a
  , LColor b
  , float amount
  );

// END FUNCTIONS

// GLOBALS

const float TO_RAD               = M_PI / 180.0;
const LVecBase2f PI_SHADER_INPUT = LVecBase2f(M_PI, TO_RAD);

const float GAMMA                   = 2.2;
const float GAMMA_REC               = 1.0 / GAMMA;
const LVecBase2f GAMMA_SHADER_INPUT = LVecBase2f(GAMMA, GAMMA_REC);

const int BACKGROUND_RENDER_SORT_ORDER = 10;
const int UNSORTED_RENDER_SORT_ORDER   = 50;

const int SSAO_SAMPLES = 8;
const int SSAO_NOISE   = 4;

const int SHADOW_SIZE = 2048;

LVecBase4f sunlightColor0 =
  LVecBase4f
    ( 0.612
    , 0.365
    , 0.306
    , 1
    );
LVecBase4f sunlightColor1 =
  LVecBase4f
    ( 0.765
    , 0.573
    , 0.400
    , 1
    );
LVecBase4f moonlightColor0 =
  LVecBase4f
    ( 0.247
    , 0.384
    , 0.404
    , 1
    );
LVecBase4f moonlightColor1 =
  LVecBase4f
    ( 0.392
    , 0.537
    , 0.571
    , 1
    );
LVecBase4f windowLightColor =
  LVecBase4f
    ( 0.765
    , 0.573
    , 0.400
    , 1
    );

std::uniform_real_distribution<float>
  randomFloats
    ( 0.0
    , 1.0
    );
std::default_random_engine generator;

PT(AsyncTaskManager) taskManager = AsyncTaskManager::get_global_ptr();

PT(AudioManager) audioManager = AudioManager::create_AudioManager();

ParticleSystemManager particleSystemManager = ParticleSystemManager();

PhysicsManager physicsManager = PhysicsManager();

// END GLOBALS

// MAIN

int main
  ( int argc
  , char *argv[]
  ) {
  LColor backgroundColor [] =
    { LColor
        ( 0.392
        , 0.537
        , 0.561
        , 1
        )
    , LColor
        ( 0.953
        , 0.733
        , 0.525
        , 1
        )
    };

  double     cameraRotatePhiInitial    =   67.5095;
  double     cameraRotateThetaInitial  =  231.721;
  double     cameraRotateRadiusInitial = 1100.83;
  LVecBase3  cameraLookAtInitial       = LVecBase3(1.00839, 1.20764, 5.85055);
  float      cameraFov                 =    1.0;
  int        cameraNear                =  150;
  int        cameraFar                 = 2000;
  LVecBase2f cameraNearFar             = LVecBase2f(cameraNear, cameraFar);
  double     cameraRotateRadius        = cameraRotateRadiusInitial;
  double     cameraRotatePhi           = cameraRotatePhiInitial;
  double     cameraRotateTheta         = cameraRotateThetaInitial;
  LVecBase3  cameraLookAt              = cameraLookAtInitial;

  float fogNearInitial = 2.0;
  float fogFarInitial  = 9.0;
  float fogNear        = fogNearInitial;
  float fogFar         = fogFarInitial;
  float fogAdjust      = 0.1;

  LVecBase2f foamDepthInitial = LVecBase2f(1.5, 1.5);
  float      foamDepthAdjust  = 0.1;
  LVecBase2f foamDepth        = foamDepthInitial;

  LVecBase2f mouseThen = LVecBase2f(0.0, 0.0);
  LVecBase2f mouseNow  = mouseThen;
  bool mouseWheelDown  = false;
  bool mouseWheelUp    = false;

  LVecBase2f riorInitial = LVecBase2f(1.05, 1.05);
  float      riorAdjust  = 0.005;
  LVecBase2f rior        = riorInitial;

  LVecBase2f mouseFocusPointInitial = LVecBase2f(0.509167, 0.598);
  LVecBase2f mouseFocusPoint        = mouseFocusPointInitial;

  float sunlightP       = 260;
  bool  animateSunlight = true;

  bool  soundEnabled  = true;
  bool  soundStarted  = false;
  float startSoundAt  = 0.5;

  bool closedShutters  = true;

  float       statusAlpha       = 1.0;
  LColor      statusColor       = LColor(0.9, 0.9, 1.0, statusAlpha);
  LColor      statusShadowColor = LColor(0.1, 0.1, 0.3, statusAlpha);
  float       statusFadeRate    = 2.0;
  std::string statusText        = "Ready";

  LVecBase2f ssaoEnabled         = makeEnabledVec(1);
  LVecBase2f blinnPhongEnabled   = makeEnabledVec(1);
  LVecBase2f fresnelEnabled      = makeEnabledVec(1);
  LVecBase2f rimLightEnabled     = makeEnabledVec(1);
  LVecBase2f refractionEnabled   = makeEnabledVec(1);
  LVecBase2f reflectionEnabled   = makeEnabledVec(1);
  LVecBase2f fogEnabled          = makeEnabledVec(1);
  LVecBase2f outlineEnabled      = makeEnabledVec(1);
  LVecBase2f celShadingEnabled   = makeEnabledVec(1);
  LVecBase2f normalMapsEnabled   = makeEnabledVec(1);
  LVecBase2f bloomEnabled        = makeEnabledVec(1);
  LVecBase2f sharpenEnabled      = makeEnabledVec(1);
  LVecBase2f depthOfFieldEnabled = makeEnabledVec(1);
  LVecBase2f filmGrainEnabled    = makeEnabledVec(1);
  LVecBase2f flowMapsEnabled     = makeEnabledVec(1);
  LVecBase2f lookupTableEnabled  = makeEnabledVec(1);
  LVecBase2f painterlyEnabled    = makeEnabledVec(0);
  LVecBase2f motionBlurEnabled   = makeEnabledVec(0);
  LVecBase2f posterizeEnabled    = makeEnabledVec(0);
  LVecBase2f pixelizeEnabled     = makeEnabledVec(0);

  LVecBase4 rgba8  = ( 8,  8,  8,  8);
  LVecBase4 rgba16 = (16, 16, 16, 16);
  LVecBase4 rgba32 = (32, 32, 32, 32);

  load_prc_file("panda3d-prc-file.prc");

  PT(TextFont) font = FontPool::load_font("fonts/font.ttf");

  std::vector<PT(AudioSound)> sounds =
    { audioManager->get_sound("sounds/wheel.ogg", true)
    , audioManager->get_sound("sounds/water.ogg", true)
    };

  PT(Texture) blankTexture             = TexturePool::load_texture("images/blank.png");
  PT(Texture) foamPatternTexture       = TexturePool::load_texture("images/foam-pattern.png");
  PT(Texture) stillFlowTexture         = TexturePool::load_texture("images/still-flow.png");
  PT(Texture) upFlowTexture            = TexturePool::load_texture("images/up-flow.png");
  PT(Texture) colorLookupTableTextureN = TexturePool::load_texture("images/lookup-table-neutral.png");
  PT(Texture) colorLookupTableTexture0 = TexturePool::load_texture("images/lookup-table-0.png");
  PT(Texture) colorLookupTableTexture1 = TexturePool::load_texture("images/lookup-table-1.png");
  PT(Texture) smokeTexture             = TexturePool::load_texture("images/smoke.png");
  PT(Texture) colorNoiseTexture        = TexturePool::load_texture("images/color-noise.png");

  setTextureToNearestAndClamp(colorLookupTableTextureN);
  setTextureToNearestAndClamp(colorLookupTableTexture0);
  setTextureToNearestAndClamp(colorLookupTableTexture1);

  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("3D Game Shaders For Beginners By David Lettier");

  PT(WindowFramework)         window                = framework.open_window();
  PT(GraphicsWindow)          graphicsWindow        = window->get_graphics_window();
  PT(GraphicsOutput)          graphicsOutput        = window->get_graphics_output();
  PT(GraphicsStateGuardian)   graphicsStateGuardian = graphicsOutput->get_gsg();
  PT(GraphicsEngine)          graphicsEngine        = graphicsStateGuardian->get_engine();

  window->enable_keyboard();

  PT(DisplayRegion) displayRegion3d = window->get_display_region_3d();
  displayRegion3d->set_clear_color_active(true);
  displayRegion3d->set_clear_depth_active(true);
  displayRegion3d->set_clear_stencil_active(true);
  displayRegion3d->set_clear_color(backgroundColor[1]);
  displayRegion3d->set_clear_depth(1.0f);
  displayRegion3d->set_clear_stencil(0);

  NodePath render   = window->get_render();
  NodePath render2d = window->get_render_2d();

  PT(TextNode) status = new TextNode("status");
  status->set_font(font);
  status->set_text(statusText);
  status->set_text_color(statusColor);
  status->set_shadow(0.0, 0.06);
  status->set_shadow_color(statusShadowColor);
  NodePath statusNP = render2d.attach_new_node(status);
  statusNP.set_scale(0.05);
  statusNP.set_pos(-0.96, 0, -0.95);

  PT(MouseWatcher) mouseWatcher = getMouseWatcher(window);

  PT(Camera) mainCamera = window->get_camera(0);
  PT(Lens) mainLens = mainCamera->get_lens();
  mainLens->set_fov(cameraFov);
  mainLens->set_near_far(cameraNear, cameraFar);

  NodePath cameraNP = window->get_camera_group();

  cameraNP.set_pos
    ( calculateCameraPosition
        ( cameraRotateRadius
        , cameraRotatePhi
        , cameraRotateTheta
        , cameraLookAt
        )
    );
  cameraNP.look_at(cameraLookAt);

  PT(PandaNode) sceneRootPN = new PandaNode("sceneRoot");
  NodePath sceneRootNP      = NodePath(sceneRootPN);
  sceneRootNP.reparent_to(render);

  NodePath environmentNP =
    window
      ->load_model
        ( framework.get_models()
        , "eggs/mill-scene/mill-scene.bam"
        );
  environmentNP.reparent_to(sceneRootNP);
  NodePath shuttersNP =
    window
      ->load_model
        ( framework.get_models()
        , "eggs/mill-scene/shutters.bam"
        );
  shuttersNP.reparent_to(sceneRootNP);
  NodePath weatherVaneNP =
    window
      ->load_model
        ( framework.get_models()
        , "eggs/mill-scene/weather-vane.bam"
        );
  weatherVaneNP.reparent_to(sceneRootNP);
  NodePath bannerNP =
    window
      ->load_model
        ( framework.get_models()
        , "eggs/mill-scene/banner.bam"
        );
  bannerNP.reparent_to(sceneRootNP);

  NodePath wheelNP   = environmentNP.find("**/wheel-lp");
  NodePath waterNP   = environmentNP.find("**/water-lp");

  squashGeometry(environmentNP);

  NodePath smokeNP = setUpParticles(render, smokeTexture);

  waterNP.set_transparency(TransparencyAttrib::M_dual);
  waterNP.set_bin("fixed", 0);

  AnimControlCollection shuttersAnimationCollection;
  AnimControlCollection weatherVaneAnimationCollection;
  AnimControlCollection bannerAnimationCollection;
  auto_bind
    ( shuttersNP.node()
    , shuttersAnimationCollection
    ,   PartGroup::HMF_ok_wrong_root_name
      | PartGroup::HMF_ok_part_extra
      | PartGroup::HMF_ok_anim_extra
    );
  auto_bind
    ( weatherVaneNP.node()
    , weatherVaneAnimationCollection
    ,   PartGroup::HMF_ok_wrong_root_name
      | PartGroup::HMF_ok_part_extra
      | PartGroup::HMF_ok_anim_extra
    );
  auto_bind
    ( bannerNP.node()
    , bannerAnimationCollection
    ,   PartGroup::HMF_ok_wrong_root_name
      | PartGroup::HMF_ok_part_extra
      | PartGroup::HMF_ok_anim_extra
    );

  generateLights(render, false);

  PT(Shader) discardShader               = loadShader("discard", "discard");
  PT(Shader) baseShader                  = loadShader("base",    "base");
  PT(Shader) geometryBufferShader0       = loadShader("base",    "geometry-buffer-0");
  PT(Shader) geometryBufferShader1       = loadShader("base",    "geometry-buffer-1");
  PT(Shader) geometryBufferShader2       = loadShader("base",    "geometry-buffer-2");
  PT(Shader) foamShader                  = loadShader("basic",   "foam");
  PT(Shader) fogShader                   = loadShader("basic",   "fog");
  PT(Shader) boxBlurShader               = loadShader("basic",   "box-blur");
  PT(Shader) motionBlurShader            = loadShader("basic",   "motion-blur");
  PT(Shader) kuwaharaFilterShader        = loadShader("basic",   "kuwahara-filter");
  PT(Shader) dilationShader              = loadShader("basic",   "dilation");
  PT(Shader) sharpenShader               = loadShader("basic",   "sharpen");
  PT(Shader) outlineShader               = loadShader("basic",   "outline");
  PT(Shader) bloomShader                 = loadShader("basic",   "bloom");
  PT(Shader) ssaoShader                  = loadShader("basic",   "ssao");
  PT(Shader) screenSpaceRefractionShader = loadShader("basic",   "screen-space-refraction");
  PT(Shader) screenSpaceReflectionShader = loadShader("basic",   "screen-space-reflection");
  PT(Shader) refractionShader            = loadShader("basic",   "refraction");
  PT(Shader) reflectionColorShader       = loadShader("basic",   "reflection-color");
  PT(Shader) reflectionShader            = loadShader("basic",   "reflection");
  PT(Shader) baseCombineShader           = loadShader("basic",   "base-combine");
  PT(Shader) sceneCombineShader          = loadShader("basic",   "scene-combine");
  PT(Shader) depthOfFieldShader          = loadShader("basic",   "depth-of-field");
  PT(Shader) posterizeShader             = loadShader("basic",   "posterize");
  PT(Shader) pixelizeShader              = loadShader("basic",   "pixelize");
  PT(Shader) filmGrainShader             = loadShader("basic",   "film-grain");
  PT(Shader) lookupTableShader           = loadShader("basic",   "lookup-table");
  PT(Shader) gammaCorrectionShader       = loadShader("basic",   "gamma-correction");

  NodePath mainCameraNP = NodePath("mainCamera");
  mainCameraNP.set_shader(discardShader);
  mainCamera->set_initial_state(mainCameraNP.get_state());

  NodePath isWaterNP = NodePath("isWater");
  isWaterNP.set_shader_input("isWater",            LVecBase2f(1.0, 1.0));
  isWaterNP.set_shader_input("flowTexture",        upFlowTexture);
  isWaterNP.set_shader_input("foamPatternTexture", foamPatternTexture);

  NodePath isSmokeNP = NodePath("isSmoke");
  isSmokeNP.set_shader_input("isSmoke",    LVecBase2f(1.0, 1.0));
  isSmokeNP.set_shader_input("isParticle", LVecBase2f(1.0, 1.0));

  LMatrix4 currentViewWorldMat      = cameraNP.get_transform(render)->get_mat();
  LMatrix4 previousViewWorldMat     = previousViewWorldMat;

  FramebufferTextureArguments framebufferTextureArguments;
  framebufferTextureArguments.window         = window;
  framebufferTextureArguments.graphicsOutput = graphicsOutput;
  framebufferTextureArguments.graphicsEngine = graphicsEngine;

  framebufferTextureArguments.bitplane       = GraphicsOutput::RTP_color;
  framebufferTextureArguments.rgbaBits       = rgba32;
  framebufferTextureArguments.clearColor     = LColor(0, 0, 0, 0);
  framebufferTextureArguments.aux_rgba       = 1;
  framebufferTextureArguments.setFloatColor  = true;
  framebufferTextureArguments.setSrgbColor   = false;
  framebufferTextureArguments.setRgbColor    = true;
  framebufferTextureArguments.useScene       = true;
  framebufferTextureArguments.name           = "geometry0";

  FramebufferTexture geometryFramebufferTexture0 =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) geometryBuffer0 = geometryFramebufferTexture0.buffer;
  PT(Camera)         geometryCamera0 = geometryFramebufferTexture0.camera;
  NodePath           geometryNP0     = geometryFramebufferTexture0.shaderNP;
  geometryBuffer0->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_0
    );
  geometryBuffer0->set_clear_active(3, true);
  geometryBuffer0->set_clear_value( 3, framebufferTextureArguments.clearColor);
  geometryNP0.set_shader(geometryBufferShader0);
  geometryNP0.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  geometryCamera0->set_initial_state(geometryNP0.get_state());
  geometryCamera0->set_camera_mask(BitMask32::bit(1));
  PT(Texture) positionTexture0    = geometryBuffer0->get_texture(0);
  PT(Texture) normalTexture0      = geometryBuffer0->get_texture(1);
  PT(Lens)    geometryCameraLens0 = geometryCamera0->get_lens();
  waterNP.hide(BitMask32::bit(1));
  smokeNP.hide(BitMask32::bit(1));

  framebufferTextureArguments.aux_rgba = 4;
  framebufferTextureArguments.name     = "geometry1";

  FramebufferTexture geometryFramebufferTexture1 =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) geometryBuffer1 = geometryFramebufferTexture1.buffer;
  PT(Camera)         geometryCamera1 = geometryFramebufferTexture1.camera;
  NodePath           geometryNP1     = geometryFramebufferTexture1.shaderNP;
  geometryBuffer1->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_0
    );
  geometryBuffer1->set_clear_active(3, true);
  geometryBuffer1->set_clear_value( 3, framebufferTextureArguments.clearColor);
  geometryBuffer1->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_1
    );
  geometryBuffer1->set_clear_active(4, true);
  geometryBuffer1->set_clear_value( 4, framebufferTextureArguments.clearColor);
  geometryBuffer1->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_2
    );
  geometryBuffer1->set_clear_active(5, true);
  geometryBuffer1->set_clear_value( 5, framebufferTextureArguments.clearColor);
  geometryBuffer1->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_3
    );
  geometryBuffer1->set_clear_active(6, true);
  geometryBuffer1->set_clear_value( 6, framebufferTextureArguments.clearColor);
  geometryNP1.set_shader(geometryBufferShader1);
  geometryNP1.set_shader_input("normalMapsEnabled",  normalMapsEnabled);
  geometryNP1.set_shader_input("flowTexture",        stillFlowTexture);
  geometryNP1.set_shader_input("foamPatternTexture", blankTexture);
  geometryNP1.set_shader_input("flowMapsEnabled",    flowMapsEnabled);
  geometryCamera1->set_initial_state(geometryNP1.get_state());
  geometryCamera1->set_tag_state_key("geometryBuffer1");
  geometryCamera1->set_tag_state("isWater", isWaterNP.get_state());
  geometryCamera1->set_camera_mask(BitMask32::bit(2));
  PT(Texture) positionTexture1        = geometryBuffer1->get_texture(0);
  PT(Texture) normalTexture1          = geometryBuffer1->get_texture(1);
  PT(Texture) reflectionMaskTexture   = geometryBuffer1->get_texture(2);
  PT(Texture) refractionMaskTexture   = geometryBuffer1->get_texture(3);
  PT(Texture) foamMaskTexture         = geometryBuffer1->get_texture(4);
  PT(Lens)    geometryCameraLens1     = geometryCamera1->get_lens();
  waterNP.set_tag("geometryBuffer1", "isWater");
  smokeNP.hide(BitMask32::bit(2));

  framebufferTextureArguments.aux_rgba = 1;
  framebufferTextureArguments.name     = "geometry2";

  FramebufferTexture geometryFramebufferTexture2 =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) geometryBuffer2 = geometryFramebufferTexture2.buffer;
  PT(Camera)         geometryCamera2 = geometryFramebufferTexture2.camera;
  NodePath           geometryNP2     = geometryFramebufferTexture2.shaderNP;
  geometryBuffer2->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_0
    );
  geometryBuffer2->set_clear_active(3, true);
  geometryBuffer2->set_clear_value( 3, framebufferTextureArguments.clearColor);
  geometryBuffer2->set_sort(geometryBuffer1->get_sort() + 1);
  geometryNP2.set_shader(geometryBufferShader2);
  geometryNP2.set_shader_input("isSmoke",         LVecBase2f(0, 0));
  geometryNP2.set_shader_input("positionTexture", positionTexture1);
  geometryCamera2->set_initial_state(geometryNP2.get_state());
  geometryCamera2->set_tag_state_key("geometryBuffer2");
  geometryCamera2->set_tag_state("isSmoke", isSmokeNP.get_state());
  smokeNP.set_tag("geometryBuffer2", "isSmoke");
  PT(Texture) positionTexture2         = geometryBuffer2->get_texture(0);
  PT(Texture) smokeMaskTexture         = geometryBuffer2->get_texture(1);
  PT(Lens)    geometryCameraLens2      = geometryCamera2->get_lens();

  framebufferTextureArguments.rgbaBits      = rgba8;
  framebufferTextureArguments.aux_rgba      = 0;
  framebufferTextureArguments.clearColor    = LColor(0, 0, 0, 0);
  framebufferTextureArguments.setFloatColor = false;
  framebufferTextureArguments.useScene      = false;
  framebufferTextureArguments.name          = "fog";

  FramebufferTexture fogFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) fogBuffer = fogFramebufferTexture.buffer;
  PT(Camera)         fogCamera = fogFramebufferTexture.camera;
  NodePath           fogNP     = fogFramebufferTexture.shaderNP;
  fogBuffer->set_sort(geometryBuffer2->get_sort() + 1);
  fogNP.set_shader(fogShader);
  fogNP.set_shader_input("pi",               PI_SHADER_INPUT);
  fogNP.set_shader_input("gamma",            GAMMA_SHADER_INPUT);
  fogNP.set_shader_input("backgroundColor0", backgroundColor[0]);
  fogNP.set_shader_input("backgroundColor1", backgroundColor[1]);
  fogNP.set_shader_input("positionTexture0", positionTexture1);
  fogNP.set_shader_input("positionTexture1", positionTexture2);
  fogNP.set_shader_input("smokeMaskTexture", smokeMaskTexture);
  fogNP.set_shader_input("sunPosition",      LVecBase2f(sunlightP, 0));
  fogNP.set_shader_input("origin",           cameraNP.get_relative_point(render, environmentNP.get_pos()));
  fogNP.set_shader_input("nearFar",          LVecBase2f(fogNear, fogFar));
  fogNP.set_shader_input("enabled",          fogEnabled);
  fogCamera->set_initial_state(fogNP.get_state());
  PT(Texture) fogTexture = fogBuffer->get_texture();

  framebufferTextureArguments.clearColor = LColor(1, 1, 1, 0);
  framebufferTextureArguments.name       = "ssao";

  FramebufferTexture ssaoFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) ssaoBuffer = ssaoFramebufferTexture.buffer;
  PT(Camera)         ssaoCamera = ssaoFramebufferTexture.camera;
  NodePath           ssaoNP     = ssaoFramebufferTexture.shaderNP;
  ssaoBuffer->set_sort(geometryBuffer0->get_sort() + 1);
  ssaoNP.set_shader(ssaoShader);
  ssaoNP.set_shader_input("positionTexture", positionTexture0);
  ssaoNP.set_shader_input("normalTexture",   normalTexture0);
  ssaoNP.set_shader_input("samples",         generateSsaoSamples(SSAO_SAMPLES));
  ssaoNP.set_shader_input("noise",           generateSsaoNoise(SSAO_NOISE));
  ssaoNP.set_shader_input("lensProjection",  geometryCameraLens0->get_projection_mat());
  ssaoNP.set_shader_input("enabled",         ssaoEnabled);
  ssaoCamera->set_initial_state(ssaoNP.get_state());

  framebufferTextureArguments.name = "ssaoBlur";

  FramebufferTexture ssaoBlurFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) ssaoBlurBuffer = ssaoBlurFramebufferTexture.buffer;
  NodePath           ssaoBlurNP     = ssaoBlurFramebufferTexture.shaderNP;
  ssaoBlurBuffer->set_sort(ssaoBuffer->get_sort() + 1);
  ssaoBlurNP.set_shader(kuwaharaFilterShader);
  ssaoBlurNP.set_shader_input("colorTexture", ssaoBuffer->get_texture());
  ssaoBlurNP.set_shader_input("parameters",   LVecBase2f(1, 0));
  ssaoBlurFramebufferTexture.camera->set_initial_state(ssaoBlurNP.get_state());
  PT(Texture) ssaoBlurTexture = ssaoBlurBuffer->get_texture();

  framebufferTextureArguments.rgbaBits   = rgba16;
  framebufferTextureArguments.clearColor = LColor(0, 0, 0, 0);
  framebufferTextureArguments.name       = "refractionUv";

  FramebufferTexture refractionUvFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) refractionUvBuffer = refractionUvFramebufferTexture.buffer;
  PT(Camera)         refractionUvCamera = refractionUvFramebufferTexture.camera;
  NodePath           refractionUvNP     = refractionUvFramebufferTexture.shaderNP;
  refractionUvBuffer->set_sort(geometryBuffer1->get_sort() + 1);
  refractionUvNP.set_shader(screenSpaceRefractionShader);
  refractionUvNP.set_shader_input("positionFromTexture", positionTexture1);
  refractionUvNP.set_shader_input("positionToTexture",   positionTexture0);
  refractionUvNP.set_shader_input("normalFromTexture",   normalTexture1);
  refractionUvNP.set_shader_input("lensProjection",      geometryCameraLens0->get_projection_mat());
  refractionUvNP.set_shader_input("enabled",             refractionEnabled);
  refractionUvNP.set_shader_input("rior",                rior);
  refractionUvCamera->set_initial_state(refractionUvNP.get_state());
  PT(Texture) refractionUvTexture = refractionUvBuffer->get_texture();

  framebufferTextureArguments.name = "reflectionUv";

  FramebufferTexture reflectionUvFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) reflectionUvBuffer = reflectionUvFramebufferTexture.buffer;
  PT(Camera)         reflectionUvCamera = reflectionUvFramebufferTexture.camera;
  NodePath           reflectionUvNP     = reflectionUvFramebufferTexture.shaderNP;
  reflectionUvBuffer->set_sort(geometryBuffer1->get_sort() + 1);
  reflectionUvNP.set_shader(screenSpaceReflectionShader);
  reflectionUvNP.set_shader_input("positionTexture", positionTexture1);
  reflectionUvNP.set_shader_input("normalTexture",   normalTexture1);
  reflectionUvNP.set_shader_input("maskTexture",     reflectionMaskTexture);
  reflectionUvNP.set_shader_input("lensProjection",  geometryCameraLens0->get_projection_mat());
  reflectionUvNP.set_shader_input("enabled",         reflectionEnabled);
  reflectionUvCamera->set_initial_state(reflectionUvNP.get_state());
  PT(Texture) reflectionUvTexture = reflectionUvBuffer->get_texture();

  framebufferTextureArguments.rgbaBits = rgba8;
  framebufferTextureArguments.aux_rgba = 1;
  framebufferTextureArguments.useScene = true;
  framebufferTextureArguments.name     = "base";

  FramebufferTexture baseFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) baseBuffer = baseFramebufferTexture.buffer;
  PT(Camera)         baseCamera = baseFramebufferTexture.camera;
  NodePath           baseNP     = baseFramebufferTexture.shaderNP;
  baseBuffer->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_0
    );
  baseBuffer->set_clear_active(3, true);
  baseBuffer->set_clear_value( 3, framebufferTextureArguments.clearColor);
  baseBuffer->set_sort
    ( std::max
        ( ssaoBlurBuffer->get_sort() + 1
        , UNSORTED_RENDER_SORT_ORDER + 1
        )
    );
  baseNP.set_shader(baseShader);
  baseNP.set_shader_input("pi",                PI_SHADER_INPUT);
  baseNP.set_shader_input("gamma",             GAMMA_SHADER_INPUT);
  baseNP.set_shader_input("ssaoBlurTexture",   ssaoBlurTexture);
  baseNP.set_shader_input("flowTexture",       stillFlowTexture);
  baseNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  baseNP.set_shader_input("blinnPhongEnabled", blinnPhongEnabled);
  baseNP.set_shader_input("fresnelEnabled",    fresnelEnabled);
  baseNP.set_shader_input("rimLightEnabled",   rimLightEnabled);
  baseNP.set_shader_input("celShadingEnabled", celShadingEnabled);
  baseNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
  baseNP.set_shader_input("specularOnly",      LVecBase2f(0, 0));
  baseNP.set_shader_input("isParticle",        LVecBase2f(0, 0));
  baseNP.set_shader_input("isWater",           LVecBase2f(0, 0));
  baseNP.set_shader_input("sunPosition",       LVecBase2f(sunlightP, 0));
  baseCamera->set_initial_state(baseNP.get_state());
  baseCamera->set_tag_state_key("baseBuffer");
  baseCamera->set_tag_state("isParticle", isSmokeNP.get_state());
  baseCamera->set_tag_state("isWater",    isWaterNP.get_state());
  baseCamera->set_camera_mask(BitMask32::bit(6));
  smokeNP.set_tag("baseBuffer", "isParticle");
  waterNP.set_tag("baseBuffer", "isWater");
  PT(Texture) baseTexture     = baseBuffer->get_texture(0);
  PT(Texture) specularTexture = baseBuffer->get_texture(1);

  framebufferTextureArguments.aux_rgba = 0;
  framebufferTextureArguments.useScene = false;
  framebufferTextureArguments.name     = "refraction";

  FramebufferTexture refractionFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) refractionBuffer = refractionFramebufferTexture.buffer;
  PT(Camera)         refractionCamera = refractionFramebufferTexture.camera;
  NodePath           refractionNP     = refractionFramebufferTexture.shaderNP;
  refractionBuffer->set_sort(baseBuffer->get_sort() + 1);
  refractionNP.set_shader(refractionShader);
  refractionNP.set_shader_input("pi",                     PI_SHADER_INPUT);
  refractionNP.set_shader_input("gamma",                  GAMMA_SHADER_INPUT);
  refractionNP.set_shader_input("uvTexture",              refractionUvTexture);
  refractionNP.set_shader_input("maskTexture",            refractionMaskTexture);
  refractionNP.set_shader_input("positionFromTexture",    positionTexture1);
  refractionNP.set_shader_input("positionToTexture",      positionTexture0);
  refractionNP.set_shader_input("backgroundColorTexture", baseTexture);
  refractionNP.set_shader_input("sunPosition",            LVecBase2f(sunlightP, 0));
  refractionCamera->set_initial_state(refractionNP.get_state());
  PT(Texture) refractionTexture = refractionBuffer->get_texture();

  framebufferTextureArguments.name = "foam";

  FramebufferTexture foamFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) foamBuffer = foamFramebufferTexture.buffer;
  PT(Camera)         foamCamera = foamFramebufferTexture.camera;
  NodePath           foamNP     = foamFramebufferTexture.shaderNP;
  foamBuffer->set_sort(geometryBuffer1->get_sort() + 1);
  foamNP.set_shader(foamShader);
  foamNP.set_shader_input("pi",                  PI_SHADER_INPUT);
  foamNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
  foamNP.set_shader_input("maskTexture",         foamMaskTexture);
  foamNP.set_shader_input("foamDepth",           foamDepth);
  foamNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
  foamNP.set_shader_input("viewWorldMat",        currentViewWorldMat);
  foamNP.set_shader_input("positionFromTexture", positionTexture1);
  foamNP.set_shader_input("positionToTexture",   positionTexture0);
  foamCamera->set_initial_state(foamNP.get_state());
  PT(Texture) foamTexture = foamBuffer->get_texture();

  framebufferTextureArguments.name = "reflectionColor";

  FramebufferTexture reflectionColorFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) reflectionColorBuffer = reflectionColorFramebufferTexture.buffer;
  PT(Camera)         reflectionColorCamera = reflectionColorFramebufferTexture.camera;
  NodePath           reflectionColorNP     = reflectionColorFramebufferTexture.shaderNP;
  reflectionColorBuffer->set_sort(refractionBuffer->get_sort() + 1);
  reflectionColorNP.set_shader(reflectionColorShader);
  reflectionColorNP.set_shader_input("colorTexture", refractionTexture);
  reflectionColorNP.set_shader_input("uvTexture",    reflectionUvTexture);
  reflectionColorCamera->set_initial_state(reflectionColorNP.get_state());
  PT(Texture) reflectionColorTexture = reflectionColorBuffer->get_texture();

  framebufferTextureArguments.name = "reflectionColorBlur";

  FramebufferTexture reflectionColorBlurFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) reflectionColorBlurBuffer = reflectionColorBlurFramebufferTexture.buffer;
  PT(Camera)         reflectionColorBlurCamera = reflectionColorBlurFramebufferTexture.camera;
  NodePath           reflectionColorBlurNP     = reflectionColorBlurFramebufferTexture.shaderNP;
  reflectionColorBlurBuffer->set_sort(reflectionColorBuffer->get_sort() + 1);
  reflectionColorBlurNP.set_shader(boxBlurShader);
  reflectionColorBlurNP.set_shader_input("colorTexture", reflectionColorTexture);
  reflectionColorBlurNP.set_shader_input("parameters",   LVecBase2f(8, 1));
  reflectionColorBlurCamera->set_initial_state(reflectionColorBlurNP.get_state());
  PT(Texture) reflectionColorBlurTexture = reflectionColorBlurBuffer->get_texture();

  framebufferTextureArguments.name = "reflection";

  FramebufferTexture reflectionFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) reflectionBuffer = reflectionFramebufferTexture.buffer;
  NodePath           reflectionNP     = reflectionFramebufferTexture.shaderNP;
  reflectionBuffer->set_sort(reflectionColorBlurBuffer->get_sort() + 1);
  reflectionNP.set_shader(reflectionShader);
  reflectionNP.set_shader_input("colorTexture",     reflectionColorTexture);
  reflectionNP.set_shader_input("colorBlurTexture", reflectionColorBlurTexture);
  reflectionNP.set_shader_input("maskTexture",      reflectionMaskTexture);
  reflectionFramebufferTexture.camera->set_initial_state(reflectionNP.get_state());
  PT(Texture) reflectionTexture = reflectionBuffer->get_texture();

  framebufferTextureArguments.name = "baseCombine";

  FramebufferTexture baseCombineFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) baseCombineBuffer = baseCombineFramebufferTexture.buffer;
  PT(Camera)         baseCombineCamera = baseCombineFramebufferTexture.camera;
  NodePath           baseCombineNP     = baseCombineFramebufferTexture.shaderNP;
  baseCombineBuffer->set_sort(reflectionBuffer->get_sort() + 1);
  baseCombineNP.set_shader(baseCombineShader);
  baseCombineNP.set_shader_input("baseTexture",       baseTexture);
  baseCombineNP.set_shader_input("refractionTexture", refractionTexture);
  baseCombineNP.set_shader_input("foamTexture",       foamTexture);
  baseCombineNP.set_shader_input("reflectionTexture", reflectionTexture);
  baseCombineNP.set_shader_input("specularTexture",   specularTexture);
  baseCombineCamera->set_initial_state(baseCombineNP.get_state());
  PT(Texture) baseCombineTexture = baseCombineBuffer->get_texture();

  framebufferTextureArguments.name = "sharpen";

  FramebufferTexture sharpenFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) sharpenBuffer = sharpenFramebufferTexture.buffer;
  NodePath           sharpenNP     = sharpenFramebufferTexture.shaderNP;
  sharpenBuffer->set_sort(baseCombineBuffer->get_sort() + 1);
  sharpenNP.set_shader(sharpenShader);
  sharpenNP.set_shader_input("colorTexture", baseCombineTexture);
  sharpenNP.set_shader_input("enabled",      sharpenEnabled);
  PT(Camera) sharpenCamera = sharpenFramebufferTexture.camera;
  sharpenCamera->set_initial_state(sharpenNP.get_state());
  PT(Texture) sharpenTexture = sharpenBuffer->get_texture();

  framebufferTextureArguments.name = "posterize";

  FramebufferTexture posterizeFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) posterizeBuffer = posterizeFramebufferTexture.buffer;
  NodePath           posterizeNP     = posterizeFramebufferTexture.shaderNP;
  posterizeBuffer->set_sort(sharpenBuffer->get_sort() + 1);
  posterizeNP.set_shader(posterizeShader);
  posterizeNP.set_shader_input("gamma",           GAMMA_SHADER_INPUT);
  posterizeNP.set_shader_input("colorTexture",    sharpenTexture);
  posterizeNP.set_shader_input("positionTexture", positionTexture2);
  posterizeNP.set_shader_input("enabled",         posterizeEnabled);
  PT(Camera) posterizeCamera = posterizeFramebufferTexture.camera;
  posterizeCamera->set_initial_state(posterizeNP.get_state());
  PT(Texture) posterizeTexture = posterizeBuffer->get_texture();

  framebufferTextureArguments.name = "bloom";

  FramebufferTexture bloomFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) bloomBuffer = bloomFramebufferTexture.buffer;
  PT(Camera)         bloomCamera = bloomFramebufferTexture.camera;
  NodePath           bloomNP     = bloomFramebufferTexture.shaderNP;
  bloomBuffer->set_sort(posterizeBuffer->get_sort() + 1);
  bloomNP.set_shader(bloomShader);
  bloomNP.set_shader_input("colorTexture", posterizeTexture);
  bloomNP.set_shader_input("enabled",      bloomEnabled);
  bloomCamera->set_initial_state(bloomNP.get_state());
  PT(Texture) bloomTexture = bloomBuffer->get_texture();

  framebufferTextureArguments.name = "sceneCombine";

  FramebufferTexture sceneCombineFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) sceneCombineBuffer = sceneCombineFramebufferTexture.buffer;
  PT(Camera)         sceneCombineCamera = sceneCombineFramebufferTexture.camera;
  NodePath           sceneCombineNP     = sceneCombineFramebufferTexture.shaderNP;
  sceneCombineBuffer->set_sort(bloomBuffer->get_sort() + 1);
  sceneCombineNP.set_shader(sceneCombineShader);
  sceneCombineNP.set_shader_input("pi",                  PI_SHADER_INPUT);
  sceneCombineNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
  sceneCombineNP.set_shader_input("lookupTableTextureN", colorLookupTableTextureN);
  sceneCombineNP.set_shader_input("backgroundColor0",    backgroundColor[0]);
  sceneCombineNP.set_shader_input("backgroundColor1",    backgroundColor[1]);
  sceneCombineNP.set_shader_input("baseTexture",         posterizeTexture);
  sceneCombineNP.set_shader_input("bloomTexture",        bloomTexture);
  sceneCombineNP.set_shader_input("fogTexture",          fogTexture);
  sceneCombineNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
  PT(Texture) sceneCombineTexture = sceneCombineBuffer->get_texture();
  sceneCombineCamera->set_initial_state(sceneCombineNP.get_state());

  framebufferTextureArguments.clearColor = backgroundColor[1];
  framebufferTextureArguments.name       = "outOfFocus";

  FramebufferTexture outOfFocusFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) outOfFocusBuffer = outOfFocusFramebufferTexture.buffer;
  PT(Camera)         outOfFocusCamera = outOfFocusFramebufferTexture.camera;
  NodePath           outOfFocusNP     = outOfFocusFramebufferTexture.shaderNP;
  outOfFocusBuffer->set_sort(sceneCombineBuffer->get_sort() + 1);
  outOfFocusNP.set_shader(boxBlurShader);
  outOfFocusNP.set_shader_input("colorTexture", sceneCombineTexture);
  outOfFocusNP.set_shader_input("parameters",   LVecBase2f(2, 2));
  outOfFocusCamera->set_initial_state(outOfFocusNP.get_state());
  PT(Texture) outOfFocusTexture = outOfFocusBuffer->get_texture();

  framebufferTextureArguments.name = "dilatedOutOfFocus";

  FramebufferTexture dilatedOutOfFocusFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) dilatedOutOfFocusBuffer = dilatedOutOfFocusFramebufferTexture.buffer;
  PT(Camera)         dilatedOutOfFocusCamera = dilatedOutOfFocusFramebufferTexture.camera;
  NodePath           dilatedOutOfFocusNP     = dilatedOutOfFocusFramebufferTexture.shaderNP;
  dilatedOutOfFocusBuffer->set_sort(outOfFocusBuffer->get_sort() + 1);
  dilatedOutOfFocusNP.set_shader(dilationShader);
  dilatedOutOfFocusNP.set_shader_input("colorTexture", outOfFocusTexture);
  dilatedOutOfFocusNP.set_shader_input("parameters",   LVecBase2f(4, 2));
  dilatedOutOfFocusCamera->set_initial_state(dilatedOutOfFocusNP.get_state());
  PT(Texture) dilatedOutOfFocusTexture = dilatedOutOfFocusBuffer->get_texture();

  framebufferTextureArguments.aux_rgba = 1;
  framebufferTextureArguments.name     = "depthOfField";

  FramebufferTexture depthOfFieldFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) depthOfFieldBuffer = depthOfFieldFramebufferTexture.buffer;
  NodePath           depthOfFieldNP     = depthOfFieldFramebufferTexture.shaderNP;
  depthOfFieldBuffer->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_aux_rgba_0
    );
  depthOfFieldBuffer->set_clear_active(3, true);
  depthOfFieldBuffer->set_clear_value( 3, framebufferTextureArguments.clearColor);
  depthOfFieldBuffer->set_sort(dilatedOutOfFocusBuffer->get_sort() + 1);
  depthOfFieldNP.set_shader(depthOfFieldShader);
  depthOfFieldNP.set_shader_input("positionTexture",   positionTexture0);
  depthOfFieldNP.set_shader_input("focusTexture",      sceneCombineTexture);
  depthOfFieldNP.set_shader_input("outOfFocusTexture", dilatedOutOfFocusTexture);
  depthOfFieldNP.set_shader_input("mouseFocusPoint",   mouseFocusPoint);
  depthOfFieldNP.set_shader_input("nearFar",           cameraNearFar);
  depthOfFieldNP.set_shader_input("enabled",           depthOfFieldEnabled);
  PT(Camera) depthOfFieldCamera = depthOfFieldFramebufferTexture.camera;
  depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());
  PT(Texture) depthOfFieldTexture0 = depthOfFieldBuffer->get_texture(0);
  PT(Texture) depthOfFieldTexture1 = depthOfFieldBuffer->get_texture(1);

  framebufferTextureArguments.aux_rgba = 0;
  framebufferTextureArguments.name     = "outline";

  FramebufferTexture outlineFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) outlineBuffer = outlineFramebufferTexture.buffer;
  PT(Camera)         outlineCamera = outlineFramebufferTexture.camera;
  NodePath           outlineNP     = outlineFramebufferTexture.shaderNP;
  outlineBuffer->set_sort(depthOfFieldBuffer->get_sort() + 1);
  outlineNP.set_shader(outlineShader);
  outlineNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
  outlineNP.set_shader_input("positionTexture",     positionTexture0);
  outlineNP.set_shader_input("colorTexture",        depthOfFieldTexture0);
  outlineNP.set_shader_input("noiseTexture",        colorNoiseTexture);
  outlineNP.set_shader_input("depthOfFieldTexture", depthOfFieldTexture1);
  outlineNP.set_shader_input("fogTexture",          fogTexture);
  outlineNP.set_shader_input("nearFar",             cameraNearFar);
  outlineNP.set_shader_input("enabled",             outlineEnabled);
  outlineCamera->set_initial_state(outlineNP.get_state());
  PT(Texture) outlineTexture = outlineBuffer->get_texture();

  framebufferTextureArguments.name = "painterly";

  FramebufferTexture painterlyFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) painterlyBuffer = painterlyFramebufferTexture.buffer;
  NodePath           painterlyNP     = painterlyFramebufferTexture.shaderNP;
  painterlyBuffer->set_sort(outlineBuffer->get_sort() + 1);
  painterlyNP.set_shader(kuwaharaFilterShader);
  painterlyNP.set_shader_input("colorTexture", outlineTexture);
  painterlyNP.set_shader_input("parameters",   LVecBase2f(0, 0));
  PT(Camera) painterlyCamera = painterlyFramebufferTexture.camera;
  painterlyCamera->set_initial_state(painterlyNP.get_state());
  PT(Texture) painterlyTexture = painterlyBuffer->get_texture();

  framebufferTextureArguments.name = "pixelize";

  FramebufferTexture pixelizeFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) pixelizeBuffer = pixelizeFramebufferTexture.buffer;
  NodePath           pixelizeNP     = pixelizeFramebufferTexture.shaderNP;
  pixelizeBuffer->set_sort(painterlyBuffer->get_sort() + 1);
  pixelizeNP.set_shader(pixelizeShader);
  pixelizeNP.set_shader_input("colorTexture",    painterlyTexture);
  pixelizeNP.set_shader_input("positionTexture", positionTexture2);
  pixelizeNP.set_shader_input("parameters",      LVecBase2f(5, 0));
  pixelizeNP.set_shader_input("enabled",         pixelizeEnabled);
  PT(Camera) pixelizeCamera = pixelizeFramebufferTexture.camera;
  pixelizeCamera->set_initial_state(pixelizeNP.get_state());
  PT(Texture) pixelizeTexture = pixelizeBuffer->get_texture();

  framebufferTextureArguments.name = "motionBlur";

  FramebufferTexture motionBlurFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) motionBlurBuffer = motionBlurFramebufferTexture.buffer;
  NodePath           motionBlurNP     = motionBlurFramebufferTexture.shaderNP;
  motionBlurBuffer->set_sort(pixelizeBuffer->get_sort() + 1);
  motionBlurNP.set_shader(motionBlurShader);
  motionBlurNP.set_shader_input("previousViewWorldMat",    previousViewWorldMat);
  motionBlurNP.set_shader_input("worldViewMat",            render.get_transform(cameraNP)->get_mat());
  motionBlurNP.set_shader_input("lensProjection",          geometryCameraLens2->get_projection_mat());
  motionBlurNP.set_shader_input("positionTexture",         positionTexture2);
  motionBlurNP.set_shader_input("colorTexture",            pixelizeTexture);
  motionBlurNP.set_shader_input("motionBlurEnabled",       motionBlurEnabled);
  motionBlurNP.set_shader_input("parameters",              LVecBase2f(2, 1.0));
  PT(Camera) motionBlurCamera = motionBlurFramebufferTexture.camera;
  motionBlurCamera->set_initial_state(motionBlurNP.get_state());
  PT(Texture) motionBlurTexture = motionBlurBuffer->get_texture();

  framebufferTextureArguments.name = "filmGrain";

  FramebufferTexture filmGrainFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) filmGrainBuffer = filmGrainFramebufferTexture.buffer;
  NodePath           filmGrainNP     = filmGrainFramebufferTexture.shaderNP;
  filmGrainBuffer->set_sort(motionBlurBuffer->get_sort() + 1);
  filmGrainNP.set_shader(filmGrainShader);
  filmGrainNP.set_shader_input("pi",           PI_SHADER_INPUT);
  filmGrainNP.set_shader_input("colorTexture", motionBlurTexture);
  filmGrainNP.set_shader_input("enabled",      filmGrainEnabled);
  PT(Camera) filmGrainCamera = filmGrainFramebufferTexture.camera;
  filmGrainCamera->set_initial_state(filmGrainNP.get_state());
  PT(Texture) filmGrainTexture = filmGrainBuffer->get_texture();

  framebufferTextureArguments.name = "lookupTable";

  FramebufferTexture lookupTableFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) lookupTableBuffer = lookupTableFramebufferTexture.buffer;
  NodePath           lookupTableNP     = lookupTableFramebufferTexture.shaderNP;
  lookupTableBuffer->set_sort(filmGrainBuffer->get_sort() + 1);
  lookupTableNP.set_shader(lookupTableShader);
  lookupTableNP.set_shader_input("pi",                  PI_SHADER_INPUT);
  lookupTableNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
  lookupTableNP.set_shader_input("colorTexture",        filmGrainTexture);
  lookupTableNP.set_shader_input("lookupTableTextureN", colorLookupTableTextureN);
  lookupTableNP.set_shader_input("lookupTableTexture0", colorLookupTableTexture0);
  lookupTableNP.set_shader_input("lookupTableTexture1", colorLookupTableTexture1);
  lookupTableNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
  lookupTableNP.set_shader_input("enabled",             lookupTableEnabled);
  PT(Camera) lookupTableCamera = lookupTableFramebufferTexture.camera;
  lookupTableCamera->set_initial_state(lookupTableNP.get_state());
  PT(Texture) lookupTableTexture = lookupTableBuffer->get_texture();

  framebufferTextureArguments.name = "gammaCorrection";

  FramebufferTexture gammaCorrectionFramebufferTexture =
    generateFramebufferTexture
      ( framebufferTextureArguments
      );
  PT(GraphicsOutput) gammaCorrectionBuffer = gammaCorrectionFramebufferTexture.buffer;
  NodePath           gammaCorrectionNP     = gammaCorrectionFramebufferTexture.shaderNP;
  gammaCorrectionBuffer->set_sort(lookupTableBuffer->get_sort() + 1);
  gammaCorrectionNP.set_shader(gammaCorrectionShader);
  gammaCorrectionNP.set_shader_input("gamma",        GAMMA_SHADER_INPUT);
  gammaCorrectionNP.set_shader_input("colorTexture", lookupTableTexture);
  PT(Camera) gammaCorrectionCamera = gammaCorrectionFramebufferTexture.camera;
  gammaCorrectionCamera->set_initial_state(gammaCorrectionNP.get_state());

  graphicsOutput->set_sort(gammaCorrectionBuffer->get_sort() + 1);

  int showBufferIndex = 0;

  std::vector<std::tuple<std::string, PT(GraphicsOutput), int>> bufferArray =
    { std::make_tuple("Positions 0",          geometryBuffer0,           0)
    , std::make_tuple("Normals 0",            geometryBuffer0,           1)
    , std::make_tuple("Positions 1",          geometryBuffer1,           0)
    , std::make_tuple("Normals 1",            geometryBuffer1,           1)
    , std::make_tuple("Reflection Mask",      geometryBuffer1,           2)
    , std::make_tuple("Refraction Mask",      geometryBuffer1,           3)
    , std::make_tuple("Foam Mask",            geometryBuffer1,           4)
    , std::make_tuple("Positions 2",          geometryBuffer2,           0)
    , std::make_tuple("Smoke Mask",           geometryBuffer2,           1)
    , std::make_tuple("SSAO",                 ssaoBuffer,                0)
    , std::make_tuple("SSAO Blur",            ssaoBlurBuffer,            0)
    , std::make_tuple("Refraction UV",        refractionUvBuffer,        0)
    , std::make_tuple("Refraction",           refractionBuffer,          0)
    , std::make_tuple("Reflection UV",        reflectionUvBuffer,        0)
    , std::make_tuple("Reflection Color",     reflectionColorBuffer,     0)
    , std::make_tuple("Reflection Blur",      reflectionColorBlurBuffer, 0)
    , std::make_tuple("Reflection",           reflectionBuffer,          0)
    , std::make_tuple("Foam",                 foamBuffer,                0)
    , std::make_tuple("Base",                 baseBuffer,                0)
    , std::make_tuple("Specular",             baseBuffer,                1)
    , std::make_tuple("Base Combine",         baseCombineBuffer,         0)
    , std::make_tuple("Painterly",            painterlyBuffer,           0)
    , std::make_tuple("Posterize",            posterizeBuffer,           0)
    , std::make_tuple("Bloom",                bloomBuffer,               0)
    , std::make_tuple("Outline",              outlineBuffer,             0)
    , std::make_tuple("Fog",                  fogBuffer,                 0)
    , std::make_tuple("Scene Combine",        sceneCombineBuffer,        0)
    , std::make_tuple("Out of Focus",         outOfFocusBuffer,          0)
    , std::make_tuple("Dilation",             dilatedOutOfFocusBuffer,   0)
    , std::make_tuple("Depth of Field Blur",  depthOfFieldBuffer,        1)
    , std::make_tuple("Depth of Field",       depthOfFieldBuffer,        0)
    , std::make_tuple("Pixelize",             pixelizeBuffer,            0)
    , std::make_tuple("Motion Blur",          motionBlurBuffer,          0)
    , std::make_tuple("Film Grain",           filmGrainBuffer,           0)
    , std::make_tuple("Lookup Table",         lookupTableBuffer,         0)
    , std::make_tuple("Gamma Correction",     gammaCorrectionBuffer,     0)
    };

  showBufferIndex = bufferArray.size() - 1;

  showBuffer
    ( render2d
    , statusNP
    , bufferArray[showBufferIndex]
    , false
    );

  shuttersAnimationCollection.play(   "close-shutters"          );
  weatherVaneAnimationCollection.loop("weather-vane-shake", true);
  bannerAnimationCollection.loop(     "banner-swing",       true);

  int then          = microsecondsSinceEpoch();
  int loopStartedAt = then;
  int now           = then;
  int keyTime       = now;

  auto beforeFrame =
    [&]() -> void {

    WindowProperties windowProperties = graphicsWindow->get_properties();
    if (windowProperties.get_minimized()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    now = microsecondsSinceEpoch();

    // Avoids a loud audio pop.
    if (!soundStarted && microsecondToSecond(now - loopStartedAt) >= startSoundAt) {
      for_each
        ( sounds.begin()
        , sounds.end()
        , [](PT(AudioSound) sound) { sound->set_loop(true); sound->play(); }
        );
      soundStarted = true;
    }

    double delta = microsecondToSecond(now - then);

    then = now;

    double movement = 100 * delta;

    double timeSinceKey = microsecondToSecond(now - keyTime);
    bool   keyDebounced = timeSinceKey >= 0.2;

    double cameraUpDownAdjust    = 0;
    double cameraLeftRightAdjust = 0;

    bool shiftDown        = isButtonDown(mouseWatcher, "shift");
    bool tabDown          = isButtonDown(mouseWatcher, "tab");

    bool resetDown        = isButtonDown(mouseWatcher, "r");

    bool fogNearDown      = isButtonDown(mouseWatcher, "[");
    bool fogFarDown       = isButtonDown(mouseWatcher, "]");

    bool equalDown        = isButtonDown(mouseWatcher, "=");
    bool minusDown        = isButtonDown(mouseWatcher, "-");

    bool deleteDown       = isButtonDown(mouseWatcher, "delete");

    bool wDown            = isButtonDown(mouseWatcher, "w");
    bool aDown            = isButtonDown(mouseWatcher, "a");
    bool dDown            = isButtonDown(mouseWatcher, "d");
    bool sDown            = isButtonDown(mouseWatcher, "s");
    bool zDown            = isButtonDown(mouseWatcher, "z");
    bool xDown            = isButtonDown(mouseWatcher, "x");

    bool arrowUpDown      = isButtonDown(mouseWatcher, "arrow_up");
    bool arrowDownDown    = isButtonDown(mouseWatcher, "arrow_down");
    bool arrowLeftDown    = isButtonDown(mouseWatcher, "arrow_left");
    bool arrowRightDown   = isButtonDown(mouseWatcher, "arrow_right");

    bool middayDown       = isButtonDown(mouseWatcher, "1");
    bool midnightDown     = isButtonDown(mouseWatcher, "2");
    bool fresnelDown      = isButtonDown(mouseWatcher, "3");
    bool rimLightDown     = isButtonDown(mouseWatcher, "4");
    bool particlesDown    = isButtonDown(mouseWatcher, "5");
    bool motionBlurDown   = isButtonDown(mouseWatcher, "6");
    bool painterlyDown    = isButtonDown(mouseWatcher, "7");
    bool celShadingDown   = isButtonDown(mouseWatcher, "8");
    bool lookupTableDown  = isButtonDown(mouseWatcher, "9");
    bool blinnPhongDown   = isButtonDown(mouseWatcher, "0");
    bool ssaoDown         = isButtonDown(mouseWatcher, "y");
    bool outlineDown      = isButtonDown(mouseWatcher, "u");
    bool bloomDown        = isButtonDown(mouseWatcher, "i");
    bool normalMapsDown   = isButtonDown(mouseWatcher, "o");
    bool fogDown          = isButtonDown(mouseWatcher, "p");
    bool depthOfFieldDown = isButtonDown(mouseWatcher, "h");
    bool posterizeDown    = isButtonDown(mouseWatcher, "j");
    bool pixelizeDown     = isButtonDown(mouseWatcher, "k");
    bool sharpenDown      = isButtonDown(mouseWatcher, "l");
    bool filmGrainDown    = isButtonDown(mouseWatcher, "n");
    bool reflectionDown   = isButtonDown(mouseWatcher, "m");
    bool refractionDown   = isButtonDown(mouseWatcher, ",");
    bool flowMapsDown     = isButtonDown(mouseWatcher, ".");
    bool sunlightDown     = isButtonDown(mouseWatcher, "/");

    bool mouseLeftDown    = mouseWatcher->is_button_down(MouseButton::one());
    bool mouseMiddleDown  = mouseWatcher->is_button_down(MouseButton::two());
    bool mouseRightDown   = mouseWatcher->is_button_down(MouseButton::three());

    if (wDown) {
      cameraRotatePhi -= movement * 0.5;
    }

    if (sDown) {
      cameraRotatePhi += movement * 0.5;
    }

    if (aDown) {
      cameraRotateTheta += movement * 0.5;
    }

    if (dDown) {
      cameraRotateTheta -= movement * 0.5;
    }

    if (zDown || mouseWheelUp) {
      cameraRotateRadius -= movement * 4 + 50 * mouseWheelUp;
      mouseWheelUp = false;
    }

    if (xDown || mouseWheelDown) {
      cameraRotateRadius += movement * 4 + 50 * mouseWheelDown;
      mouseWheelDown = false;
    }

    if (cameraRotatePhi    <  1)               cameraRotatePhi    = 1;
    if (cameraRotatePhi    >  179)             cameraRotatePhi    = 179;
    if (cameraRotatePhi    <  0)               cameraRotatePhi    = 360 - cameraRotateTheta;
    if (cameraRotateTheta  >  360)             cameraRotateTheta  = cameraRotateTheta - 360;
    if (cameraRotateTheta  <  0)               cameraRotateTheta  = 360 - cameraRotateTheta;
    if (cameraRotateRadius < cameraNear +  5)  cameraRotateRadius = cameraNear +  5;
    if (cameraRotateRadius > cameraFar  - 10)  cameraRotateRadius = cameraFar  - 10;

    if (arrowUpDown) {
      cameraUpDownAdjust = -2 * delta;
    } else if (arrowDownDown) {
      cameraUpDownAdjust =  2 * delta;
    }

    if (arrowLeftDown) {
      cameraLeftRightAdjust =  2 * delta;
    } else if (arrowRightDown) {
      cameraLeftRightAdjust = -2 * delta;
    }

    if (mouseWatcher->has_mouse()) {
      mouseNow[0] = mouseWatcher->get_mouse_x();
      mouseNow[1] = mouseWatcher->get_mouse_y();

      if (mouseLeftDown) {
        cameraRotateTheta += (mouseThen[0] - mouseNow[0] ) * movement;
        cameraRotatePhi   += (mouseNow[1]  - mouseThen[1]) * movement;
      } else if (mouseRightDown) {
        cameraLeftRightAdjust = (mouseThen[0] - mouseNow[0]) * movement;
        cameraUpDownAdjust    = (mouseThen[1] - mouseNow[1]) * movement;
      } else if (mouseMiddleDown) {
        mouseFocusPoint =
          LVecBase2f
            ( (mouseNow[0] + 1.0) / 2.0
            , (mouseNow[1] + 1.0) / 2.0
            );
      }

      if (!mouseLeftDown) {
        mouseThen = mouseNow;
      }
    }

    if (shiftDown && fogNearDown) {
      fogNear += fogAdjust;

      statusAlpha = 1.0;
      statusText  = "Fog Near " + std::to_string(fogNear);
    } else if (fogNearDown) {
      fogNear -= fogAdjust;

      statusAlpha = 1.0;
      statusText  = "Fog Near " + std::to_string(fogNear);
    }

    if (shiftDown && fogFarDown) {
      fogFar -= fogAdjust;

      statusAlpha = 1.0;
      statusText  = "Fog Far " + std::to_string(fogFar);
    } else if (fogFarDown) {
      fogFar += fogAdjust;

      statusAlpha = 1.0;
      statusText  = "Fog Far " + std::to_string(fogFar);
    }

    if (shiftDown && equalDown) {
      rior[0] -= riorAdjust;

      statusAlpha = 1.0;
      statusText  = "Refractive Index " + std::to_string(rior[0]);
    } else if (equalDown) {
      rior[0] += riorAdjust;

      statusAlpha = 1.0;
      statusText  = "Refractive Index " + std::to_string(rior[0]);
    }
    rior[1] = rior[0];

    if (shiftDown && minusDown) {
      foamDepth[0] -= foamDepthAdjust;
      if (foamDepth[0] < 0.001) { foamDepth[0] = 0.001; };

      statusAlpha = 1.0;
      statusText  = "Foam Depth " + std::to_string(foamDepth[0]);
    } else if (minusDown) {
      foamDepth[0] += foamDepthAdjust;

      statusAlpha = 1.0;
      statusText  = "Foam Depth " + std::to_string(foamDepth[0]);
    }
    foamDepth[1] = foamDepth[0];

    if (keyDebounced) {
      if (tabDown) {
        if (shiftDown) {
          showBufferIndex -= 1;
          if (showBufferIndex < 0) showBufferIndex = bufferArray.size() - 1;
        } else {
          showBufferIndex += 1;
          if (showBufferIndex >= bufferArray.size()) showBufferIndex = 0;
        }

        std::string bufferName = std::get<0>(bufferArray[showBufferIndex]);
        bool showAlpha =
              bufferName == "Outline"
          ||  bufferName == "Foam"
          ||  bufferName == "Fog"
          ;

        showBuffer
          ( render2d
          , statusNP
          , bufferArray[showBufferIndex]
          , showAlpha
          );

        keyTime = now;

        statusAlpha = 1.0;
        statusText  = bufferName + " Buffer";
      }

      if (resetDown) {
        cameraRotateRadius = cameraRotateRadiusInitial;
        cameraRotatePhi    = cameraRotatePhiInitial;
        cameraRotateTheta  = cameraRotateThetaInitial;
        cameraLookAt       = cameraLookAtInitial;

        fogNear = fogNearInitial;
        fogFar  = fogFarInitial;

        foamDepth = foamDepthInitial;
        rior      = riorInitial;

        mouseFocusPoint = mouseFocusPointInitial;

        keyTime = now;

        statusAlpha = 1.0;
        statusText  = "Reset";
      }

      auto toggleStatus =
        [&](LVecBase2f enabled, std::string effect) -> void {
          statusAlpha = 1.0;
          if (enabled[0] == 1) {
            statusText = effect + " On";
          } else {
            statusText = effect + " Off";
          }
        };

      if (ssaoDown) {
        ssaoEnabled = toggleEnabledVec(ssaoEnabled);
        keyTime = now;

        toggleStatus
          ( ssaoEnabled
          , "SSAO"
          );
      }

      if (refractionDown) {
        refractionEnabled = toggleEnabledVec(refractionEnabled);
        keyTime = now;

        toggleStatus
          ( refractionEnabled
          , "Refraction"
          );
      }

      if (reflectionDown) {
        reflectionEnabled = toggleEnabledVec(reflectionEnabled);
        keyTime = now;

        toggleStatus
          ( reflectionEnabled
          , "Reflection"
          );
      }

      if (bloomDown) {
        bloomEnabled = toggleEnabledVec(bloomEnabled);
        keyTime = now;

        toggleStatus
          ( bloomEnabled
          , "Bloom"
          );
      }

      if (normalMapsDown){
        normalMapsEnabled = toggleEnabledVec(normalMapsEnabled);
        keyTime = now;

        toggleStatus
          ( normalMapsEnabled
          , "Normal Maps"
          );
      }

      if (fogDown) {
        fogEnabled = toggleEnabledVec(fogEnabled);
        keyTime = now;

        toggleStatus
          ( fogEnabled
          , "Fog"
          );
      }

      if (outlineDown) {
        outlineEnabled = toggleEnabledVec(outlineEnabled);
        keyTime = now;

        toggleStatus
          ( outlineEnabled
          , "Outline"
          );
      }

      if (celShadingDown) {
        celShadingEnabled = toggleEnabledVec(celShadingEnabled);
        keyTime = now;

        toggleStatus
          ( celShadingEnabled
          , "Cel Shading"
          );
      }

      if (lookupTableDown) {
        lookupTableEnabled = toggleEnabledVec(lookupTableEnabled);
        keyTime = now;

        toggleStatus
          ( lookupTableEnabled
          , "Lookup Table"
          );
      }

      if (fresnelDown) {
        fresnelEnabled = toggleEnabledVec(fresnelEnabled);
        keyTime = now;

        toggleStatus
          ( fresnelEnabled
          , "Fresnel"
          );
      }

      if (rimLightDown) {
        rimLightEnabled = toggleEnabledVec(rimLightEnabled);
        keyTime = now;

        toggleStatus
          ( rimLightEnabled
          , "Rim Light"
          );
      }

      if (blinnPhongDown) {
        blinnPhongEnabled = toggleEnabledVec(blinnPhongEnabled);
        keyTime = now;

        toggleStatus
          ( blinnPhongEnabled
          , "Blinn-Phong"
          );
      }

      if (sharpenDown) {
        sharpenEnabled = toggleEnabledVec(sharpenEnabled);
        keyTime = now;

        toggleStatus
          ( sharpenEnabled
          , "Sharpen"
          );
      }

      if (depthOfFieldDown) {
        depthOfFieldEnabled = toggleEnabledVec(depthOfFieldEnabled);
        keyTime = now;

        toggleStatus
          ( depthOfFieldEnabled
          , "Depth of Field"
          );
      }

      if (painterlyDown) {
        painterlyEnabled = toggleEnabledVec(painterlyEnabled);
        keyTime = now;

        toggleStatus
          ( painterlyEnabled
          , "Painterly"
          );
      }

      if (motionBlurDown) {
        motionBlurEnabled = toggleEnabledVec(motionBlurEnabled);
        keyTime = now;

        toggleStatus
          ( motionBlurEnabled
          , "Motion Blur"
          );
      }

      if (posterizeDown) {
        posterizeEnabled = toggleEnabledVec(posterizeEnabled);
        keyTime = now;

        toggleStatus
          ( posterizeEnabled
          , "Posterize"
          );
      }

      if (pixelizeDown) {
        pixelizeEnabled = toggleEnabledVec(pixelizeEnabled);
        keyTime = now;

        toggleStatus
          ( pixelizeEnabled
          , "Pixelize"
          );
      }

      if (filmGrainDown) {
        filmGrainEnabled = toggleEnabledVec(filmGrainEnabled);
        keyTime = now;

        toggleStatus
          ( filmGrainEnabled
          , "Film Grain"
          );
      }

      if (flowMapsDown) {
        flowMapsEnabled = toggleEnabledVec(flowMapsEnabled);
        if (flowMapsEnabled[0] == 1 && soundEnabled) {
          for_each(sounds.begin(), sounds.end(), setSoundOn);
        } else if (flowMapsEnabled[0] != 1) {
          for_each(sounds.begin(), sounds.end(), setSoundOff);
        }
        keyTime = now;

        toggleStatus
          ( flowMapsEnabled
          , "Flow Maps"
          );
      }

      if (deleteDown) {
        if (soundEnabled) {
          for_each(sounds.begin(), sounds.end(), setSoundOff);
          soundEnabled = false;
        } else {
          if (flowMapsEnabled[0] == 1) {
            for_each(sounds.begin(), sounds.end(), setSoundOn);
          }
          soundEnabled = true;
        }
        keyTime = now;

        toggleStatus
          ( LVecBase2f(soundEnabled ? 1 : 0, 0)
          , "Sound"
          );
      }

      if (sunlightDown) {
        animateSunlight = animateSunlight ? false : true;
        keyTime = now;

        toggleStatus
          ( LVecBase2f(animateSunlight ? 1 : 0, 0)
          , "Sun Animation"
          );
      }

      if (particlesDown) {
        keyTime = now;
        statusAlpha = 1.0;

        if (smokeNP.is_hidden()) {
          smokeNP.show();
          statusText = "Particles On";
        } else {
          smokeNP.hide();
          statusText = "Particles Off";
        }
      }
    }

    if (flowMapsEnabled[0]) {
      float             wheelP  = wheelNP.get_p();
                        wheelP += -90.0 * delta;
      if (wheelP > 360) wheelP  =   0;
      if (wheelP <   0) wheelP  = 360;
      wheelNP.set_p(wheelP);
    }

    if (animateSunlight || middayDown || midnightDown) {
      sunlightP =
        animateLights
          ( render
          , shuttersAnimationCollection
          , delta
          , -360.0 / 64.0
          , closedShutters
          , middayDown
          , midnightDown
          );

      if (middayDown) {
        statusAlpha = 1.0;
        statusText = "Midday";
      } else if (midnightDown) {
        statusAlpha = 1.0;
        statusText = "Midnight";
      }
    }

    cameraLookAt =
      calculateCameraLookAt
        ( cameraUpDownAdjust
        , cameraLeftRightAdjust
        , cameraRotatePhi
        , cameraRotateTheta
        , cameraLookAt
        );

    cameraNP.set_pos
      ( calculateCameraPosition
          ( cameraRotateRadius
          , cameraRotatePhi
          , cameraRotateTheta
          , cameraLookAt
          )
      );

    cameraNP.look_at(cameraLookAt);

    currentViewWorldMat = cameraNP.get_transform(render)->get_mat();

    geometryNP0.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    geometryNP0.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    geometryCamera0->set_initial_state(geometryNP0.get_state());

    geometryNP1.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    geometryNP1.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    geometryCamera1->set_initial_state(geometryNP1.get_state());

    fogNP.set_shader_input("sunPosition",   LVecBase2f(sunlightP, 0));
    fogNP.set_shader_input("origin",        cameraNP.get_relative_point(render, environmentNP.get_pos()));
    fogNP.set_shader_input("nearFar",       LVecBase2f(fogNear, fogFar));
    fogNP.set_shader_input("enabled",       fogEnabled);
    fogCamera->set_initial_state(fogNP.get_state());

    ssaoNP.set_shader_input("lensProjection", geometryCameraLens0->get_projection_mat());
    ssaoNP.set_shader_input("enabled",        ssaoEnabled);
    ssaoCamera->set_initial_state(ssaoNP.get_state());

    refractionUvNP.set_shader_input("lensProjection", geometryCameraLens1->get_projection_mat());
    refractionUvNP.set_shader_input("enabled",        refractionEnabled);
    refractionUvNP.set_shader_input("rior",           rior);
    refractionUvCamera->set_initial_state(refractionUvNP.get_state());

    reflectionUvNP.set_shader_input("lensProjection", geometryCameraLens1->get_projection_mat());
    reflectionUvNP.set_shader_input("enabled",        reflectionEnabled);
    reflectionUvCamera->set_initial_state(reflectionUvNP.get_state());

    foamNP.set_shader_input("foamDepth",    foamDepth);
    foamNP.set_shader_input("viewWorldMat", currentViewWorldMat);
    foamNP.set_shader_input("sunPosition",  LVecBase2f(sunlightP, 0));
    foamCamera->set_initial_state(foamNP.get_state());

    bloomNP.set_shader_input("enabled", bloomEnabled);
    bloomCamera->set_initial_state(bloomNP.get_state());

    outlineNP.set_shader_input("enabled",             outlineEnabled);
    outlineCamera->set_initial_state(outlineNP.get_state());

    baseNP.set_shader_input("sunPosition",       LVecBase2f(sunlightP, 0));
    baseNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    baseNP.set_shader_input("blinnPhongEnabled", blinnPhongEnabled);
    baseNP.set_shader_input("fresnelEnabled",    fresnelEnabled);
    baseNP.set_shader_input("rimLightEnabled",   rimLightEnabled);
    baseNP.set_shader_input("celShadingEnabled", celShadingEnabled);
    baseNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    baseCamera->set_initial_state(baseNP.get_state());

    refractionNP.set_shader_input("sunPosition", LVecBase2f(sunlightP, 0));
    refractionCamera->set_initial_state(refractionNP.get_state());

    sharpenNP.set_shader_input("enabled", sharpenEnabled);
    sharpenCamera->set_initial_state(sharpenNP.get_state());

    sceneCombineNP.set_shader_input("sunPosition", LVecBase2f(sunlightP, 0));
    sceneCombineCamera->set_initial_state(sceneCombineNP.get_state());

    depthOfFieldNP.set_shader_input("mouseFocusPoint", mouseFocusPoint);
    depthOfFieldNP.set_shader_input("enabled",         depthOfFieldEnabled);
    depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());

    painterlyNP.set_shader_input("parameters", LVecBase2f(painterlyEnabled[0] == 1 ? 3 : 0, 0));
    painterlyCamera->set_initial_state(painterlyNP.get_state());

    motionBlurNP.set_shader_input("previousViewWorldMat",   previousViewWorldMat);
    motionBlurNP.set_shader_input("worldViewMat",           render.get_transform(cameraNP)->get_mat());
    motionBlurNP.set_shader_input("lensProjection",         geometryCameraLens1->get_projection_mat());
    motionBlurNP.set_shader_input("motionBlurEnabled",      motionBlurEnabled);
    motionBlurCamera->set_initial_state(motionBlurNP.get_state());

    posterizeNP.set_shader_input("enabled", posterizeEnabled);
    posterizeCamera->set_initial_state(posterizeNP.get_state());

    pixelizeNP.set_shader_input("enabled", pixelizeEnabled);
    pixelizeCamera->set_initial_state(pixelizeNP.get_state());

    filmGrainNP.set_shader_input("enabled", filmGrainEnabled);
    filmGrainCamera->set_initial_state(filmGrainNP.get_state());

    lookupTableNP.set_shader_input("enabled",     lookupTableEnabled);
    lookupTableNP.set_shader_input("sunPosition", LVecBase2f(sunlightP, 0));
    lookupTableCamera->set_initial_state(lookupTableNP.get_state());

    previousViewWorldMat = currentViewWorldMat;

    statusAlpha          = statusAlpha - ((1.0 / statusFadeRate) * delta);
    statusAlpha          = statusAlpha < 0.0 ? 0.0 : statusAlpha;
    statusColor[3]       = statusAlpha;
    statusShadowColor[3] = statusAlpha;
    status->set_text_color(statusColor);
    status->set_shadow_color(statusShadowColor);
    status->set_text(statusText);

    updateAudoManager
      ( sceneRootNP
      , cameraNP
      );

    particleSystemManager.do_particles(delta);
    physicsManager.do_physics(delta);
    };

  auto beforeFrameRunner =
    [](GenericAsyncTask* task, void* arg)
      -> AsyncTask::DoneStatus {
          (*static_cast<decltype(beforeFrame)*>(arg))();
          return AsyncTask::DS_cont;
      };

  taskManager->add
    ( new GenericAsyncTask
        ( "beforeFrame"
        , beforeFrameRunner
        , &beforeFrame
        )
    );

  auto setMouseWheelUp =
    [&]() {
    mouseWheelUp = true;
    };

  auto setMouseWheelDown =
    [&]() {
    mouseWheelDown = true;
    };

  framework.define_key
    ( "wheel_up"
    , "Mouse Wheel Up"
    , [](const Event*, void* arg) {
      (*static_cast<decltype(setMouseWheelUp)*>(arg))();
      }
    , &setMouseWheelUp
    );

  framework.define_key
    ( "wheel_down"
    , "Mouse Wheel Down"
    , [](const Event*, void* arg) {
      (*static_cast<decltype(setMouseWheelDown)*>(arg))();
      }
    , &setMouseWheelDown
    );

  physicsManager.attach_linear_integrator
    ( new LinearEulerIntegrator()
    );

  LVector3f wheelNPRelPos = wheelNP.get_pos(sceneRootNP);
  sounds[0]->set_3d_attributes
    ( wheelNPRelPos[0]
    , wheelNPRelPos[1]
    , wheelNPRelPos[2]
    , 0
    , 0
    , 0
    );
  LVector3f waterNPRelPos = waterNP.get_pos(sceneRootNP);
  sounds[1]->set_3d_attributes
    ( waterNPRelPos[0]
    , waterNPRelPos[1]
    , waterNPRelPos[2]
    , 0
    , 0
    , 0
    );

  sounds[0]->set_3d_min_distance(60);
  sounds[1]->set_3d_min_distance(50);

  framework.main_loop();

  audioManager->shutdown();

  framework.close_framework();

  return 0;
  }

// END MAIN

void generateLights
  ( NodePath render
  , bool showLights
  ) {
  PT(AmbientLight) ambientLight = new AmbientLight("ambientLight");
  ambientLight->set_color
    ( LVecBase4
        ( 0.388
        , 0.356
        , 0.447
        , 1
        )
    );
  NodePath ambientLightNP = render.attach_new_node(ambientLight);
  render.set_light(ambientLightNP);

  PT(DirectionalLight) sunlight = new DirectionalLight("sunlight");
  sunlight->set_color(sunlightColor1);
  sunlight->set_shadow_caster(true, SHADOW_SIZE, SHADOW_SIZE);
  sunlight->get_lens()->set_film_size(35, 35);
  sunlight->get_lens()->set_near_far(5.0, 35.0);
  if (showLights) sunlight->show_frustum();
  NodePath sunlightNP = render.attach_new_node(sunlight);
  sunlightNP.set_name("sunlight");
  render.set_light(sunlightNP);

  PT(DirectionalLight) moonlight = new DirectionalLight("moonlight");
  moonlight->set_color(moonlightColor1);
  moonlight->set_shadow_caster(true, SHADOW_SIZE, SHADOW_SIZE);
  moonlight->get_lens()->set_film_size(35, 35);
  moonlight->get_lens()->set_near_far(5.0, 35);
  if (showLights) moonlight->show_frustum();
  NodePath moonlightNP = render.attach_new_node(moonlight);
  moonlightNP.set_name("moonlight");
  render.set_light_off(moonlightNP);

  NodePath sunlightPivotNP = NodePath("sunlightPivot");
  sunlightPivotNP.reparent_to(render);
  sunlightPivotNP.set_pos(0, 0.5, 15.0);
  sunlightNP.reparent_to(sunlightPivotNP);
  sunlightNP.set_pos(0, -17.5, 0);
  sunlightPivotNP.set_hpr(135, 340, 0);

  NodePath moonlightPivotNP = NodePath("moonlightPivot");
  moonlightPivotNP.reparent_to(render);
  moonlightPivotNP.set_pos(0, 0.5, 15.0);
  moonlightNP.reparent_to(moonlightPivotNP);
  moonlightNP.set_pos(0, -17.5, 0);
  moonlightPivotNP.set_hpr(135, 160, 0);

  generateWindowLight
    ( "windowLight"
    , render
    , LVecBase3
        ( 1.5
        , 2.49
        , 7.9
        )
    , showLights
    );
  generateWindowLight
    ( "windowLight1"
    , render
    , LVecBase3
        ( 3.5
        , 2.49
        , 7.9
        )
    , showLights
    );
  generateWindowLight
    ( "windowLight2"
    , render
    , LVecBase3
        ( 3.5
        , 1.49
        , 4.5
        )
    , showLights
    );
  }

void generateWindowLight
  ( std::string name
  , NodePath render
  , LVecBase3 position
  , bool show
  ) {
  PT(Spotlight) windowLight = new Spotlight(name);
  windowLight->set_color(windowLightColor);
  windowLight->set_exponent(5);
  windowLight->set_attenuation(LVecBase3(1, 0.008, 0));
  windowLight->set_max_distance(37);

  PT(PerspectiveLens) windowLightLens = new PerspectiveLens();
  windowLightLens->set_near_far(0.5, 12);
  windowLightLens->set_fov(140);
  windowLight->set_lens(windowLightLens);

  if (show) windowLight->show_frustum();

  NodePath windowLightNP = render.attach_new_node(windowLight);
  windowLightNP.set_name(name);
  windowLightNP.set_pos(position);
  windowLightNP.set_hpr(180, 0, 0);
  render.set_light(windowLightNP);
  }

float animateLights
  ( NodePath render
  , AnimControlCollection shuttersAnimationCollection
  , float delta
  , float speed
  , bool& closedShutters
  , bool  middayDown
  , bool  midnightDown
  ) {
  auto clamp =
    []
    ( float a
    , float mn
    , float mx
    ) -> float {
      if (a > mx) { a = mx; }
      if (a < mn) { a = mn; }
      return a;
    };

  NodePath sunlightPivotNP  = render.find("**/sunlightPivot");
  NodePath moonlightPivotNP = render.find("**/moonlightPivot");
  NodePath sunlightNP       = render.find("**/sunlight");
  NodePath moonlightNP      = render.find("**/moonlight");

  PT(DirectionalLight) sunlight =
    DCAST(DirectionalLight, sunlightNP.node());
  PT(DirectionalLight) moonlight =
    DCAST(DirectionalLight, moonlightNP.node());

  float           p  = sunlightPivotNP.get_p();
                  p += speed * delta;
  if (p > 360)    p =   0;
  if (p < 0)      p = 360;

  if (middayDown) {
    p = 270;
  } else if (midnightDown) {
    p = 90;
  }

  sunlightPivotNP.set_p( p      );
  moonlightPivotNP.set_p(p - 180);

  float mixFactor = 1.0 - (sin(toRadians(p)) / 2.0 + 0.5);

  LColor sunlightColor  = mixColor(sunlightColor0,  sunlightColor1, mixFactor);
  LColor moonlightColor = mixColor(moonlightColor1, sunlightColor0, mixFactor);
  LColor lightColor     = mixColor(moonlightColor,  sunlightColor,  mixFactor);

  float dayTimeLightMagnitude   = clamp(-1 * sin(toRadians(p)), 0.0, 1.0);
  float nightTimeLightMagnitude = clamp(     sin(toRadians(p)), 0.0, 1.0);

  sunlight->set_color( lightColor * dayTimeLightMagnitude);
  moonlight->set_color(lightColor * nightTimeLightMagnitude);

  if (dayTimeLightMagnitude > 0.0) {
    sunlight->set_shadow_caster(true, SHADOW_SIZE, SHADOW_SIZE);
    render.set_light(sunlightNP);
  } else {
    sunlight->set_shadow_caster(false, 0, 0);
    render.set_light_off(sunlightNP);
  }

  if (nightTimeLightMagnitude > 0.0) {
    moonlight->set_shadow_caster(true, SHADOW_SIZE, SHADOW_SIZE);
    render.set_light(moonlightNP);
  } else {
    moonlight->set_shadow_caster(false, 0, 0);
    render.set_light_off(moonlightNP);
  }

  auto updateWindowLight =
    [&]
    ( std::string name
    ) -> void {
    NodePath windowLightNP = render.find("**/" + name);
    PT(Spotlight) windowLight = DCAST(Spotlight, windowLightNP.node());

    float windowLightMagnitude = pow(nightTimeLightMagnitude, 0.4);

    windowLight->set_color(windowLightColor * windowLightMagnitude);

    if (windowLightMagnitude <= 0.0) {
      windowLight->set_shadow_caster(false, 0, 0);
      render.set_light_off(windowLightNP);
    } else {
      windowLight->set_shadow_caster(true, SHADOW_SIZE, SHADOW_SIZE);
      render.set_light(windowLightNP);
    }
    };

  updateWindowLight("windowLight");
  updateWindowLight("windowLight1");
  updateWindowLight("windowLight2");

  if (mixFactor >= 0.3 && mixFactor <= 0.35 && closedShutters || midnightDown) {
    closedShutters = false;

    shuttersAnimationCollection.play("open-shutters");
  } else if (mixFactor >= 0.6 && mixFactor <= 0.7 && !closedShutters || middayDown) {
    closedShutters = true;

    shuttersAnimationCollection.play("close-shutters");
  }

  return p;
  }

PT(Shader) loadShader
  ( std::string vert
  , std::string frag
  ) {
  return Shader::load
    ( Shader::SL_GLSL
    , "shaders/vertex/"   + vert + ".vert"
    , "shaders/fragment/" + frag + ".frag"
    );
  }

PTA_LVecBase3f generateSsaoSamples
  ( int numberOfSamples
  ) {
  auto lerp = [](float a, float b, float f) -> float {
    return a + f * (b - a);
  };

  PTA_LVecBase3f ssaoSamples = PTA_LVecBase3f();

  for (int i = 0; i < numberOfSamples; ++i) {
    LVecBase3f sample =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator)
        ).normalized();

    float rand = randomFloats(generator);
    sample[0] *= rand;
    sample[1] *= rand;
    sample[2] *= rand;

    float scale = (float) i / (float) numberOfSamples;
    scale = lerp(0.1, 1.0, scale * scale);
    sample[0] *= scale;
    sample[1] *= scale;
    sample[2] *= scale;

    ssaoSamples.push_back(sample);
  }

  return ssaoSamples;
  }

PTA_LVecBase3f generateSsaoNoise
  ( int numberOfNoise
  ) {
  PTA_LVecBase3f ssaoNoise = PTA_LVecBase3f();

  for (int i = 0; i < numberOfNoise; ++i) {
    LVecBase3f noise =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , 0.0
        );

    ssaoNoise.push_back(noise);
  }

  return ssaoNoise;
  }

FramebufferTexture generateFramebufferTexture
  ( FramebufferTextureArguments framebufferTextureArguments
  ) {
  PT(WindowFramework)                window         = framebufferTextureArguments.window;
  PT(GraphicsOutput)                 graphicsOutput = framebufferTextureArguments.graphicsOutput;
  PT(GraphicsEngine)                 graphicsEngine = framebufferTextureArguments.graphicsEngine;
  LVecBase4                          rgbaBits       = framebufferTextureArguments.rgbaBits;
  GraphicsOutput::RenderTexturePlane bitplane       = framebufferTextureArguments.bitplane;
  int                                aux_rgba       = framebufferTextureArguments.aux_rgba;
  bool                               setFloatColor  = framebufferTextureArguments.setFloatColor;
  bool                               setSrgbColor   = framebufferTextureArguments.setSrgbColor;
  bool                               setRgbColor    = framebufferTextureArguments.setRgbColor;
  bool                               useScene       = framebufferTextureArguments.useScene;
  std::string                        name           = framebufferTextureArguments.name;
  LColor                             clearColor     = framebufferTextureArguments.clearColor;

  FrameBufferProperties fbp = FrameBufferProperties::get_default();
  fbp.set_back_buffers(0);
  fbp.set_rgba_bits
    ( rgbaBits[0]
    , rgbaBits[1]
    , rgbaBits[2]
    , rgbaBits[3]
    );
  fbp.set_aux_rgba(aux_rgba);
  fbp.set_float_color(setFloatColor);
  fbp.set_srgb_color (setSrgbColor );
  fbp.set_rgb_color  (setRgbColor  );

  PT(GraphicsOutput) buffer =
    graphicsEngine
      ->make_output
        ( graphicsOutput->get_pipe()
        , name + "Buffer"
        , BACKGROUND_RENDER_SORT_ORDER - 1
        , fbp
        , WindowProperties::size(0, 0),
            GraphicsPipe::BF_refuse_window
          | GraphicsPipe::BF_resizeable
          | GraphicsPipe::BF_can_bind_every
          | GraphicsPipe::BF_rtt_cumulative
          | GraphicsPipe::BF_size_track_host
        , graphicsOutput->get_gsg()
        , graphicsOutput->get_host()
        );
  buffer->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , bitplane
    );
  buffer->set_clear_color(clearColor);

  NodePath   cameraNP = NodePath("");
  PT(Camera) camera   = NULL;

  if (useScene) {
    cameraNP = window->make_camera();
    camera   = DCAST(Camera, cameraNP.node());
    camera->set_lens(window->get_camera(0)->get_lens());
  } else {
    camera = new Camera(name + "Camera");
    PT(OrthographicLens) lens = new OrthographicLens();
    lens->set_film_size(2, 2);
    lens->set_film_offset(0, 0);
    lens->set_near_far(-1, 1);
    camera->set_lens(lens);
    cameraNP = NodePath(camera);
  }

  PT(DisplayRegion) bufferRegion =
    buffer->make_display_region(0, 1, 0, 1);
  bufferRegion->set_camera(cameraNP);

  NodePath shaderNP = NodePath(name + "Shader");

  if (!useScene) {
    NodePath renderNP = NodePath(name + "Render");
    renderNP.set_depth_test( false);
    renderNP.set_depth_write(false);
    cameraNP.reparent_to(renderNP);
    CardMaker card = CardMaker(name);
    card.set_frame_fullscreen_quad();
    card.set_has_uvs(true);
    NodePath cardNP = NodePath(card.generate());
    cardNP.reparent_to(renderNP);
    cardNP.set_pos(0, 0, 0);
    cardNP.set_hpr(0, 0, 0);
    cameraNP.look_at(cardNP);
  }

  FramebufferTexture result;
  result.buffer       = buffer;
  result.bufferRegion = bufferRegion;
  result.camera       = camera;
  result.cameraNP     = cameraNP;
  result.shaderNP     = shaderNP;
  return result;
  }

void showBuffer
  ( NodePath render2d
  , NodePath statusNP
  , std::tuple<std::string, PT(GraphicsOutput), int> bufferTexture
  , bool alpha
  ) {
  hideBuffer
    ( render2d
    );
  std::string bufferName;
  PT(GraphicsOutput) buffer;
  int texture;
  std::tie(bufferName, buffer, texture) = bufferTexture;

  NodePath nodePath = buffer->get_texture_card();
  nodePath.set_texture(buffer->get_texture(texture));
  nodePath.reparent_to(render2d);
  nodePath.set_y(0);

  if (alpha)
    nodePath.set_transparency
      ( TransparencyAttrib::Mode::M_alpha
      );

  statusNP.reparent_to(nodePath);
  }

void hideBuffer
  ( NodePath render2d
  ) {
  NodePath nodePath = render2d.find("**/texture card");
  if (nodePath)
    nodePath.detach_node();
  }

int microsecondsSinceEpoch
  (
  ) {
  return std::chrono::duration_cast
    <std::chrono::microseconds>
      ( std::chrono::system_clock::now().time_since_epoch()
      ).count();
  }

bool isButtonDown
  ( PT(MouseWatcher) mouseWatcher
  , std::string character
  ) {
  return
    mouseWatcher
      ->is_button_down
        ( ButtonRegistry::ptr()->find_button(character)
        );
  }

PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  ) {
  return DCAST
    ( MouseWatcher
    , window->get_mouse().node()
    );
  }

void setSoundOff
  ( PT(AudioSound) sound
  ) {
    setSoundState(sound, false);
  }

void setSoundOn
  ( PT(AudioSound) sound
  ) {
    setSoundState(sound, true);
  }

void setSoundState
  ( PT(AudioSound) sound
  , bool on
  ) {
    if (!on && sound->status() == AudioSound::PLAYING) {
      sound->stop();
    } else if (on && sound->status() != AudioSound::PLAYING) {
      sound->play();
    }
  }

void updateAudoManager
  ( NodePath sceneRootNP
  , NodePath cameraNP
  ) {
  LVector3f f = sceneRootNP.get_relative_vector(cameraNP, LVector3f::forward());
  LVector3f u = sceneRootNP.get_relative_vector(cameraNP, LVector3f::up());
  LVector3f v = LVector3f(0, 0, 0);
  LVector3f p = cameraNP.get_pos(sceneRootNP);

  audioManager->audio_3d_set_listener_attributes
    ( p[0], p[1], p[2]
    , v[0], v[1], v[2]
    , f[0], f[1], f[2]
    , u[0], u[1], u[2]
    );

  audioManager->update();
  }

LVecBase3f calculateCameraPosition
  ( double radius
  , double phi
  , double theta
  , LVecBase3 lookAt
  ) {
  double x = radius * sin(toRadians(phi)) * cos(toRadians(theta)) + lookAt[0];
  double y = radius * sin(toRadians(phi)) * sin(toRadians(theta)) + lookAt[1];
  double z = radius * cos(toRadians(phi))                         + lookAt[2];
  return LVecBase3f(x, y, z);
  }

LVecBase3f calculateCameraLookAt
  ( double upDownAdjust
  , double leftRightAdjust
  , double phi
  , double theta
  , LVecBase3 lookAt
  ) {
  lookAt[0] +=  upDownAdjust * sin(toRadians(-theta - 90)) * cos(toRadians(phi));
  lookAt[1] +=  upDownAdjust * cos(toRadians(-theta - 90)) * cos(toRadians(phi));
  lookAt[2] -= -upDownAdjust * sin(toRadians(phi));

  lookAt[0] += leftRightAdjust * sin(toRadians(-theta));
  lookAt[1] += leftRightAdjust * cos(toRadians(-theta));
  return lookAt;
  }

NodePath setUpParticles
  ( NodePath render
  , PT(Texture) smokeTexture
  ) {
  PT(ParticleSystem) smokePS = new ParticleSystem();
  PT(ForceNode)      smokeFN = new ForceNode("smoke");
  PT(PhysicalNode)   smokePN = new PhysicalNode("smoke");

  smokePS->set_pool_size(75);
  smokePS->set_birth_rate(0.01);
  smokePS->set_litter_size(1);
  smokePS->set_litter_spread(2);
  smokePS->set_system_lifespan(0.0);
  smokePS->set_local_velocity_flag(true);
  smokePS->set_system_grows_older_flag(false);

  PT(PointParticleFactory) smokePPF = new PointParticleFactory();
  smokePPF->set_lifespan_base(0.1);
  smokePPF->set_lifespan_spread(3);
  smokePPF->set_mass_base(1);
  smokePPF->set_mass_spread(0);
  smokePPF->set_terminal_velocity_base(400);
  smokePPF->set_terminal_velocity_spread(0);
  smokePS->set_factory(smokePPF);

  PT(SpriteParticleRenderer) smokeSPR = new SpriteParticleRenderer();
  smokeSPR->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
  smokeSPR->set_user_alpha(1.0);
  smokeSPR->set_texture(smokeTexture);
  smokeSPR->set_color(LColor(1.0, 1.0, 1.0, 1.0));
  smokeSPR->set_x_scale_flag(true);
  smokeSPR->set_y_scale_flag(true);
  smokeSPR->set_anim_angle_flag(true);
  smokeSPR->set_initial_x_scale(0.0000001);
  smokeSPR->set_final_x_scale(  0.007);
  smokeSPR->set_initial_y_scale(0.0000001);
  smokeSPR->set_final_y_scale(  0.007);
  smokeSPR->set_nonanimated_theta(209.0546);
  smokeSPR->set_alpha_blend_method(BaseParticleRenderer::PP_BLEND_CUBIC);
  smokeSPR->set_alpha_disable(false);
  smokeSPR->get_color_interpolation_manager()->add_linear
    ( 0.0
    , 1.0
    , LColor(1.0,   1.0,   1.0,   1.0)
    , LColor(0.039, 0.078, 0.156, 1.0)
    , true
    );
  smokePS->set_renderer(smokeSPR);

  PT(PointEmitter) smokePE = new PointEmitter();
  smokePE->set_emission_type(BaseParticleEmitter::ET_EXPLICIT);
  smokePE->set_amplitude(0.0);
  smokePE->set_amplitude_spread(1.0);
  smokePE->set_offset_force(LVector3f(0.0, 0.0, 2.0));
  smokePE->set_explicit_launch_vector(LVector3f(0.0, 0.1, 0.0));
  smokePE->set_radiate_origin(LPoint3f(0.0, 0.0, 0.0));
  smokePE->set_location(LPoint3f(0.0, 0.0, 0.0));
  smokePS->set_emitter(smokePE);

  PT(LinearVectorForce) smokeLVF = new LinearVectorForce(LVector3f(3.0, -2.0, 0.0), 1.0, false);
  smokeLVF->set_vector_masks(true, true, true);
  smokeLVF->set_active(true);
  smokeFN->add_force(smokeLVF);
  smokePS->add_linear_force(smokeLVF);

  PT(LinearJitterForce) smokeLJF = new LinearJitterForce(2.0, false);
  smokeLJF->set_vector_masks(true, true, true);
  smokeLJF->set_active(true);
  smokeFN->add_force(smokeLJF);
  smokePS->add_linear_force(smokeLJF);

  PT(LinearCylinderVortexForce) smokeLCVF = new LinearCylinderVortexForce(10.0, 1.0, 4.0, 1.0, false);
  smokeLCVF->set_vector_masks(true, true, true);
  smokeLCVF->set_active(true);
  smokeFN->add_force(smokeLCVF);
  smokePS->add_linear_force(smokeLCVF);

  smokePN->insert_physical(0, smokePS);
  smokePS->set_render_parent(smokePN);
  NodePath smokeNP = render.attach_new_node(smokePN);
  smokeNP.attach_new_node(smokeFN);

  particleSystemManager.attach_particlesystem(smokePS);
  physicsManager.attach_physical(smokePS);

  smokeNP.set_pos(0.47, 4.5, 8.9);
  smokeNP.set_transparency(TransparencyAttrib::M_dual);
  smokeNP.set_bin("fixed", 0);

  return smokeNP;
  }

void squashGeometry
  ( NodePath environmentNP
  ) {
  for (int i = 0; i < 4; ++i) {
    std::string index = std::to_string(i);
    NodePathCollection treeCollection = environmentNP.find_all_matches("**/tree" + index);
    NodePath treesNP = NodePath("treeCollection" + index);
    treesNP.reparent_to(environmentNP);
    treeCollection.reparent_to(treesNP);
    treesNP.flatten_strong();
  }

  NodePathCollection barrelCollection = environmentNP.find_all_matches("**/barrel-wood*");
  NodePath barrelNP = NodePath("barrels");
  barrelNP.reparent_to(environmentNP);
  barrelCollection.reparent_to(barrelNP);
  barrelNP.flatten_strong();

  NodePath squashNP = NodePath("squash");
  squashNP.reparent_to(environmentNP);

  NodePathCollection squashCollection = environmentNP.find_all_matches("**/*");
  for (int i = 0; i < squashCollection.size(); ++i) {
    if  (   squashCollection[i].get_name() == "wheel-lp"
        ||  squashCollection[i].get_name() == "water-lp"
        ||  squashCollection[i].get_name() == "squash"
        ) { continue; }
    squashCollection[i].reparent_to(squashNP);
  }
  squashNP.flatten_strong();
  }

double microsecondToSecond
  ( int m
  ) {
  return m / 1000000.0;
  }

double toRadians
  ( double d
  ) {
  return d * M_PI / 180.0;
  }

LVecBase2f makeEnabledVec
  ( int t
  ) {
  if (t >= 1) { t = 1; } else { t = 0; }
  return LVecBase2f(t, t);
  }

LVecBase2f toggleEnabledVec
  ( LVecBase2f vec
  ) {
  int t = vec[0];
  if (t >= 1) { t = 0; } else { t = 1; }
  vec[0] = t;
  vec[1] = t;
  return vec;
  }

void setTextureToNearestAndClamp
  ( PT(Texture) texture
  ) {
    texture->set_magfilter(SamplerState::FT_nearest);
    texture->set_minfilter(SamplerState::FT_nearest);
    texture->set_wrap_u(SamplerState::WM_clamp);
    texture->set_wrap_v(SamplerState::WM_clamp);
    texture->set_wrap_w(SamplerState::WM_clamp);
  }

LColor mixColor
  ( LColor a
  , LColor b
  , float factor
  ) {
    return a * (1 - factor) + b * factor;
  }

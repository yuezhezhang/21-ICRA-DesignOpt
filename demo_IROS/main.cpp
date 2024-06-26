#include <Kin/kin.h>
#include <KOMO/komo.h>
#include <Gui/opengl.h>
#include <Kin/animation.h>

//===========================================================================

void solve(rai::Configuration C, KOMO& komo, const Skeleton& S, OptOptions options = OptOptions()){
//  komo.animateOptimization = 4;
  komo.verbose=4;
//  komo.reportProblem();

  options.maxStep = .1;
  options.aulaMuInc = 2.;

//  ofstream fil("z.log");  komo.logFile = &fil;

  komo.optimize(.01, options);
//  komo.checkGradients();

  //-- KKT data
  FILE("z.graph") <<komo.getProblemGraph(true);

  //-- more output
//  FILE("z.contacts") <<komo.getContacts() <<endl;
//  FILE("z.energy") <<catCol( komo.getPath_tau(), komo.getPath_energies() ) <<endl;

  //-- nicer display:
  komo.displayCamera();
  komo.gl->resize(700,500);
  rai::String txt;
  double lastPhase=0.;
  for(const SkeletonEntry& s:S){
    if(s.phase0>0.){
      if(s.phase0>lastPhase) txt <<'\n';
      txt <<'(' <<s.symbol;
      for(const rai::String& f:s.frames) txt <<' ' <<f;
      txt <<')';
      lastPhase = s.phase0;
    }
  }

  //-- display
  rai::ConfigurationViewer V;
  V.setConfiguration(komo.pathConfig, "optimized -- press 'q' to move on!", true);
  while(V.playVideo(C.frames.N, true));
}

//===========================================================================

enum Scenario{
  bookOnShelf,
  boxRearrangement,
  pushWithStick,
  pushWithStickFloat,
  walking,
  wrenchRing,
  liftRing,
  billards,
  forceGrasp,
  stableGrasp
};
template<> const char* rai::Enum<Scenario>::names [] = {
  "bookOnShelf",
  "boxRearrangement",
  "pushWithStick",
  "pushWithStickFloat",
  "walking",
  "wrenchRing",
  "liftRing",
  "billards",
  "forceGrasp",
  "stableGrasp"
};

void scenario(Scenario s){
  rai::Configuration C;
  C.addFile(STRING("../scenarios/" <<rai::Enum<Scenario>(s) <<".g"));
  C.watch();

  Skeleton S;
  StringA collisions;
  switch(s){
    case pushWithStick:{
       S = {
        {1., 3., SY_oppose, {"finger1", "finger2", "stick"} },
        {1., 4., SY_stable, {"gripper", "stick"} },
        {2., 4., SY_quasiStaticOn, {"table", "box"} },
        {2., 3.5, SY_contact, {"stick", "box"} },
        {3.5, 4., SY_poseEq, {"box", "target"} },
      };
      collisions = {"gripper", "box",
                    "gripper", "table",
                    "finger1", "box",
                    "finger1", "table",
                    "finger2", "box",
                    "finger2", "table",
                    "stick", "box",
                    "stick", "table"};

    } break;
    case pushWithStickFloat:{
       S = {
        {0., -1., SY_magic, {"gripper"} },
        {0., -1., SY_dampMotion, {"gripper"} },
        {1., 3., SY_oppose, {"finger1", "finger2", "stick"} },
        {1., 3.5, SY_stable, {"gripper", "stick"} },
        {2., 3.5, SY_quasiStaticOn, {"table", "box"} },
        {2., 3., SY_contact, {"stick", "box"} },
        {3., 3.5, SY_poseEq, {"box", "target"} },
      };
      collisions = {"gripper", "box",
                    "gripper", "table",
                    "gripper", "stick",
                    "finger1", "box",
                    "finger1", "table",
                    "finger1", "stick",
                    "finger2", "box",
                    "finger2", "table",
                    "finger2", "stick",
                    "stick", "box",
                    "stick", "table"};

    } break;
    case bookOnShelf:{
      S = {
        {1., 2., SY_contact, {"finger1", "box"} },
        {1., 2., SY_quasiStaticOn, {"shelf", "box"} },

        {2., 2.5, SY_poseEq, {"box", "target2"} },

        {3., 4., SY_oppose, {"finger1", "finger2", "box"} },
        {3., 4., SY_stable, {"gripper", "box"} },

        {4., 4.5, SY_poseEq, {"box", "target"} },
     };
      collisions = {"finger1", "box",
                    "finger2", "box",
                    "gripper", "box",
                    "finger1", "wall",
                    "finger2", "wall",
                    "gripper", "wall",
                    "panda_coll7", "wall",
                    "box", "shelf",
                    "box", "wall"};
    } break;
    case liftRing:{
      S = {
        {1., 4.5, SY_oppose, {"finger1", "finger2", "stick"} },
        {1., 4.5, SY_stable, {"gripper", "stick"} },
        { 2., 3., SY_dynamic, {"box"} },
//        { 2., 3., SY_quasiStatic, {"box"} },
//        { 1., 2. , SY_contactStick, {"table", "box"} },
        { 2., 4.5, SY_contact, {"stick", "ring4"} },
        { 4., 4.5, SY_stableOn, {"target", "box"} },
        { 4., 4.5, SY_above, {"target", "box"} },
     };
      collisions = {"finger1", "stick",
                    "finger2", "stick",
                    "gripper", "stick",
                    "ring4", "stick",
                    "ring1", "stick",
                    "ring2", "stick",
                    "ring3", "stick",
//                    "box", "target",
                    "box", "table",
                   };
    } break;
    case billards:{
      S = {
        {2., 4., SY_oppose, {"finger1", "finger2", "ball"} },
        {2., 4., SY_stable, {"gripper", "ball"} },
        { 5., 8., SY_dynamic, {"ball"} },
        { 6., 6., SY_bounce, {"boxBo", "ball"} },
        { 7., 7., SY_bounce, {"boxLe", "ball"} },
//        { 7., 7., SY_bounce, {"boxBo", "ball"} },
        { 8., 8., SY_touch, {"target", "ball"} }
     };
      collisions = {"finger1", "ball",
                    "finger2", "ball",
                    "finger1", "boxBo",
                    "finger2", "boxBo",
                   };
    } break;
    case forceGrasp:{
      S = {
        {1., 2., SY_oppose, {"finger1", "finger2", "stick"} },
        {1., 2., SY_contactStick, {"finger1", "stick"} },
        {1., 2., SY_contactStick, {"finger2", "stick"} },
        {1., 2., SY_dynamic, {"stick"} },
        {2., 2., SY_poseEq, {"stick", "target"} },
     };
      collisions = {"finger1", "stick",
                    "finger2", "stick",
                   };
    } break;
    case stableGrasp:{
      S = {
        {1., 2., SY_oppose, {"finger1", "finger2", "stick"} },
        {1., 2., SY_stable, {"gripper", "stick"} },
        {2., 2., SY_poseEq, {"stick", "target"} },
     };
      collisions = {"finger1", "stick",
                    "finger2", "stick",
                   };
    } break;
    default: NIY;
  }

  double maxPhase=0;
  for(const SkeletonEntry& s:S){
    if(s.phase0>maxPhase) maxPhase=s.phase0;
    if(s.phase1>maxPhase) maxPhase=s.phase1;
  }

  OptOptions options;

  KOMO komo;
  komo.setModel(C, false);
  // komo.sparseOptimization=true;
  if(s==billards){
    komo.setTiming(maxPhase, 20, .2);
    komo.addTimeOptimization();
    komo.add_qControlObjective({}, 2, 1e-2);
    komo.add_qControlObjective({}, 0, 1e-2);
    options.stopGTolerance = 1e-2; //more precision in constraintsq
  }else{
    komo.setTiming(maxPhase, 40, 1.);
    komo.add_qControlObjective({}, 2, 1e-1);
    komo.add_qControlObjective({}, 0, 1e-2);
  }


  komo.setSkeleton(S);


  if(s==bookOnShelf){
    komo.addObjective({3., -1.}, FS_scalarProductXY, {"gripper", "box"}, OT_sos, {1e1}, {-1.});
//    komo.addObjective({2.5, -1.}, FS_scalarProductXX, {"gripper", "box"}, OT_sos, {1e1});
//    komo.addObjective({2.5, -1.}, FS_scalarProductXZ, {"gripper", "box"}, OT_sos, {1e1});
  }


  for(uint i=0;i<collisions.N;i+=2){
    komo.addObjective({0,maxPhase+.5}, FS_distance, {collisions(i), collisions(i+1)}, OT_ineq, {1e1});
  }

  solve(C, komo, S, options);
}

//===========================================================================

int main(int argc,char **argv){
  rai::initCmdLine(argc, argv);

//  rnd.clockSeed();

  scenario(stableGrasp);
  scenario(bookOnShelf);
  scenario(billards);
  scenario(liftRing);
  scenario(forceGrasp);
  scenario(pushWithStick);
  for(uint k=0;k<10;k++){
    rnd.seed(k);
    scenario(pushWithStickFloat);
  }

  return 0;
}

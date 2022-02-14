#include "udf.h"
#include "math.h"
#include "sg.h"
#include "para.h"
#include "headfile1.h"
#include "materials.h"
#include "sg_mem.h"        /*  Wall flux  */

/* this file is created by Jindong, huojindong@gmail.com */
/* (rpsetvar 'uds/diffusion-long? #t)     */
/*  Below aims to define format */
#if RP_DOUBLE              
#  define REAL_FMT "%le"
#  define INT_FMT "%d"
#else
#  define REAL_FMT "%e"
#  define INT_FMT "%d"
#endif

#define Nband 6
#define Num_H 4
#define Num_Cpef 5
#define Num_Visc 6
#define Num_Leff 7
#define Num_Sigma 8
#define Num_MolWeight 9
#define LoadTableAtCaseStart 0

#define I_Pa66 0
#define I_Cu 1
#define I_Air 2
#define MolMassCu 63.0
#define MolMassAir 28.966
/* -------------------------------------------------------------------------------------------------------------------------------- */
#define cellzone_air 18
#define cellzone_anode 16
#define cellzone_anode_drop 17
#define cellzone_anode_ionlay 20
#define cellzone_cathode 14
#define cellzone_cathode_drop 15
#define cellzone_cathode_ionlay 19

#define wall_anode_anode_drop_f 43 
#define wall_cathode_cathode_drop_f 4

#define Ablative_wall_air 35  
#define Current_input_faceID 24  

#define X_PLASMA 0.0
#define Y_PLASMA 27e-3
#define Z_PLASMA 0.0
#define ARC_RADIUS 1.1e-3 

#define SIGMA_MIN 2e-6                     /* Min electrical conductivity (S/m) = the air Econd */
#define KB 1.38064903e-23	               /* Boltzmann constant                                */
#define Stefan_Boltzmann 5.670e-8          /* The blackbody radiation sigma                     */
#define TempMeltPa66 475
#define TempBoilPa66 1014                  /* Unit is K, This is Pa66 boil temperature, the wall temperature cannot large than it  */
#define TempBoilCu 2835                    /* Unit is K, Copper boiling temperature             */
#define TempAmbient 300                    /* Unit is K, Ambient temperature                    */
#define thermalcond_cu 387.56              /* Thermal conductivity of Copper, fluent default    */
#define CuSaturatedVaporP 101325.0         /* 101325.0                                          */
#define Cu_atomic_mass 1.0552e-25          /* the unit is kg for each atom      */
#define Avogadro_constant 6.02214179e23
#define h_pa66 2.6e6		               /* J/Kg  equivalent total enthalpy of vapour entering the arc plasma. From: "Ablation Controlled Arcs" C. B. RUCHTI AND L. NIEMEYER   */
#define h_cu 5.31012e6                     /* J/Kg  the sum of fusion and vaporization, cupper latent heat of fusion (0.207e6 J/Kg) , evaporation (4.694e6 J/Kg)                 */
#define h_cu_mole 3.374368855e5            /* J/mol   for copper, 63.4kg=1kmol   */
#define MW_pa66 224
#define MW_cu 63.546

#define T_num 21
#define J_num 27
#define Vi 7.9                            /* voltage for ionization                         */
#define PHIc 4.5
#define PHIa 4.5
#define Mu0 12.567e-7                     /* this is the vacuum permeability                */
#define Input_num 332

#define Shealth_Thickness 0.0001
#define Maximum_energy_source 4.0e13      /* Unit is the W/m3                               */
#define Ablation_Wall_thickness 0.002     /* Unit is meter.                                 */
#define Absorptivity_wall 0.5             /* Absorptivity_wall=1-exp(-WallThichness*Abs_coeff) , but he polymer absorption coefficient doesn't matter  */

#define CurrFreq 62.5
#define CurrAmpl 20.0
#define VdropC 10
#define VdropA 8

const char FileName[]="pa66_cu_air_table_v5.txt";
const char FileNameP1Cu[]="Cu_Avearge_Abs_coeff_P1_model_for_UDF.txt";
const char FileNameP1Air[]="AirAbsCoeffP1.txt";
/* const char FileNameAbs[]="FWB_Air_PA66_Cu_05152018.txt";  */
const double P0fl = 101325.0;                             
static int LoadFlag = 0;
static int LoadFlagP1Cu = 0;
static int LoadFlagP1Air =0;                              /* This three files will be read as the material property                                                     */
static double LangmuirConstantRate=0.0;                   /* Default initialized as 0.  Langmuir evaporation Rate at Cu boiling temperature 72.553886, unit is Kg/m2/s  */
static double Current_BC=CurrAmpl;                        /* This is I=4000*sin( M_PI*TimeSteps/5e-3+M_PI/90) when TimeSteps=0 unit is Ampere                           */
static double CurrentInput_area=0.0;

TABLE table;
P1_TABLE p1_table_cu;
P1_TABLE p1_table_air;

enum UDM
{
  UDM_Bx, UDM_By, UDM_Bz, UDM_Bmag, UDM_Jx, UDM_Jy, UDM_Jz, UDM_Jmag, UDM_Ex, UDM_Ey, UDM_Ez, UDM_Emag, UDM_Fx, UDM_Fy, UDM_Fz, UDM_Fmag, 
  UDM_ESCurrent, UDM_ESrootvap, UDM_ESrootcond, UDM_QWallFlux_F, UDM_QRadFlux_F, UDM_QCondFlux_F, UDM_Qe_F, UDM_Qion_F, UDM_Qecap_F, 
  UDM_Qcondcu_F, UDM_Qvapcu_F, UDM_Pa66_rate_F, UDM_ECond, UDM_Vdrop, UDM_SR_rate, UDM_Cu_mole, UDM_Vap2cond_F
};

/*
UDM_Bx			  	0	
UDM_By			  	1
UDM_Bz			  	2
UDM_Bmag		  	3
UDM_Jx			  	4	 
UDM_Jy			  	5
UDM_Jz			  	6
UDM_Jmag		  	7   
UDM_Ex              8   
UDM_Ey              9
UDM_Ez              10
UDM_Emag            11
UDM_Fx			    12   
UDM_Fy			  	13
UDM_Fz			  	14
UDM_Fmag            15

UDM_ESCurrent	    16
UDM_ESrootvap	    17
UDM_ESrootcond      18

UDM_QWallFlux_F     19
UDM_QRadFlux_F      20
UDM_QCondFlux_F	    21

UDM_Qe_F		  	22
UDM_Qion_F			23  
UDM_Qecap_F		  	24 
UDM_Qcondcu_F    	25
UDM_Qvapcu_F	  	26

UDM_Pa66_rate_F		27
UDM_ECond		    28
UDM_Vdrop           29 
UDM_SR_rate         30
UDM_Cu_mole         31
UDM_Vap2cond_F      32

*/

enum UDS                  
{
  UDS_V,UDS_Ax,UDS_Ay,UDS_Az,N_REQUIRED_UDS
};

double epsilon_w = 1.0;                                         /*  epsilon_w is the emissivity used for wall ablation. epsilon_w=1 means blackbody emissivity, we should give a small value   */
double JJ_Density[J_num] ={1,319.4244,1079.137,1798.561,3237.41,4316.547,5755.396,7913.669,9712.229,11510.79,13309.35,16546.76,19784.17,25179.86,31294.96,35971.22,42805.75,48920.86,55395.68,62230.21,69424.46,78417.27,90647.48,108273.4,122661.9,141726.6,150359.7};
double Catho_Vdrop[J_num]={0.0001,1.630436,4.891305,8.586957,13.58696,16.30435,18.91304,21.08696,22.06522,22.3913,22.3913,21.73913,20.76087,19.02174,17.28261,16.08696,14.56522,13.69565,12.82609,12.17391,11.63043,11.19565,10.76087,10.43478,10.21739,10.1087,10};
double Anode_Vdrop[J_num]={0.0001,0.5217395,1.530435,2.747826,4.347826,5.182609,6.017391,6.747826,7.06087,7.095652,7.165217,6.956522,6.643478,6.086957,5.530435,5.147826,4.66087,4.382609,4.104348,3.895652,3.721739,3.582609,3.443479,3.339131,3.269565,3.234783,3.2};

double tt[T_num]={300,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,12000,14000,16000,18000,20000,22000,24000,26000,28000,30000};
double Air_vis[T_num]={1.79E-05,0.0000418,0.0000648,0.0000858,0.000108,0.00013,0.000154,0.000186,0.000221,0.000246,0.000263,0.000263,0.000177,0.000096,0.00006,0.000054,0.000058,0.000063,0.000062,0.000053,0.000042};
double Air_thermal_cond[T_num]={0.0266,0.0609,0.121,0.383,0.5,0.881,2.13,3.52,1.97,1.12,1.12,1.62,2.07,2.07,2.08,2.29,2.74,3.24,3.83,4.34,4.84};
double Air_electrical_cond[T_num]={1.00E-5,1.00E-4,1.00E-3,2.14E-02,2.43E+00,2.94E+01,1.13E+02,3.90E+02,1.04E+03,1.90E+03,2.73E+03,4645.801432,6551.124541,8050.156495,9317.226225,10453.79022,11519.79473,12310.21044,12568.94746,12352.97971,12179.1};

double abs_coeff_1[100]={11768.7320783018,7563.90194595119,5065.00211603056,3756.83096003279,3124.18684945079,2730.71255600501,2455.42845232623,2256.35731842305,2096.63295450054,1945.62569382328,1786.93373701746,1629.64079305075,1485.37291773289,1363.78598687781,1273.38202577606,1220.77092519053,1201.24974963917,1205.89906959594,1225.79251991777,1252.54069882377,1279.44088121411,1300.12065757861,1308.47170663572,1302.89315505872,1285.56225980254,1258.77307657209,1224.81966107229,1185.73239200327,1142.39782257078,1095.39241941861,1045.42989824948,995.322312266162,949.520138071927,911.242298253555,876.638216234368,839.421078315892,794.281179382960,743.535756490322,693.078725937456,647.755928854023,604.783602012890,558.063706213588,504.836455356781,452.068633367578,406.577680365716,366.223004238701,326.379090457511,285.609902950518,245.564183795422,208.298267081963,176.277871624031,150.228210835341,126.794428553329,103.564964159717,83.7157167705716,70.3525143535835,61.1488726297525,52.6717719674430,44.0606476383760,35.9825342420536,29.2735084865929,24.5912269888330,21.3495989460552,18.5755781160346,15.7753659681178,13.1633955677485,11.0027328332067,9.37516848289255,8.18669814316423,7.31672910978262,6.62451828025735,5.97297222452299,5.32399933136911,4.73508372527399,4.26230894615081,3.90214451139072,3.61352196245725,3.36339262765902,3.14368047471867,2.95085206501672,2.78521008496730,2.65410567951175,2.56282144431669,2.49812317808024,2.44012508125635,2.38435602602910,2.34023056997680,2.31765831382161,2.32395375597064,2.35204325739448,2.38989494481438,2.42694813710077,2.46017765215028,2.48949836216033,2.52226893645895,2.57114081427161,2.64125800290208,2.72565074388008,2.81627282068547,2.90507801679812};
double abs_coeff_2[100]={950.800543433807,586.883369107950,495.977553749869,762.852351446332,1038.26362193951,1203.39618001080,1418.97635453303,1650.90926976159,1747.30097661112,1742.53628995567,1707.35383992895,1597.56812639862,1452.64356709070,1343.66685132006,1264.02015821514,1185.53806992937,1101.31467758896,1011.07532132304,917.842895098753,818.381060767341,704.415843790766,568.884959731993,431.774913100102,326.418625547035,241.095865886927,170.178236527013,127.637324412037,109.624600792925,97.6181882061246,93.5301236410778,95.9965122324302,99.5353940391636,101.199077985692,101.614709625766,101.626972891148,101.603492965442,101.611434758292,101.681980686027,101.526282160988,100.668060377960,98.6286666270114,95.2206804120119,91.7409524626423,88.9621707978519,85.1804005086514,78.6116933544250,70.2475866733935,62.5730365955897,56.7325830869118,51.5562118994314,46.0234372111237,40.6171843863791,36.0460316657908,32.0340434255431,27.9738130196596,23.9887746770573,20.6397278521977,17.9844059856091,15.5894250232575,13.2594898390470,11.3488037518875,10.0423474529109,8.97335814053705,7.81336896178492,6.66023094122007,5.70480008776549,4.99014770128782,4.39576323289120,3.84742922230473,3.36461963919376,2.96909697686447,2.64121257162534,2.34592755526045,2.06431604954351,1.81750519453656,1.62120214657924,1.46016929200094,1.31405812624398,1.17160138053142,1.04407990841624,0.945205823391865,0.873269941582428,0.814533368283225,0.759101451291864,0.707418361719162,0.661418658770478,0.622513132018726,0.591913323012163,0.569362717268780,0.550960535255149,0.532449154009059,0.515156572484785,0.506619697914170,0.513828680521673,0.531795584233730,0.547126580035186,0.554749205956553,0.559059247464419,0.564946922551319,0.577302449209790};
double abs_coeff_3[100]={155.106304777140,101.292386375504,76.8719940170152,57.6136199765101,51.8804590321046,50.9832306438297,52.5372829649740,58.8148588551607,65.3792778233825,70.0440412357383,70.7804426325221,63.8405520066652,51.5754857114319,37.6154015420970,26.2853143536133,19.9330624875105,15.6132498739071,11.1958443545719,9.01195812894711,7.34687420574105,5.92939180207643,5.26617972156636,5.23452668389831,5.74556963361689,6.60506360075530,7.49572994085728,8.25985919527352,9.83122693838410,11.7765074585468,13.4782471231811,14.8429308082171,15.9671433640890,17.2271811031036,18.9103235670474,20.9871883427799,22.6994276746547,23.6132752086408,24.4392210530847,25.4060409635874,26.4019481511787,26.9937124405553,26.7371616090869,25.8324430820234,24.9000676798713,24.1815603926375,23.5163221499770,22.6604135515170,21.2677820513827,19.2502417767867,17.1825651788824,15.5645130229936,14.1912289493972,12.7663243593899,11.3605067070934,10.1042052097315,8.94057909135589,7.80889728269583,6.82246219725162,6.07725351147773,5.36814823685694,4.53307279023674,3.94566241585498,3.49183561161746,2.96252437274568,2.59328063328639,2.29555278033217,2.01355977198307,1.77115907289003,1.56028634602361,1.36289893628632,1.18794361828309,1.05126691176245,0.943989812907977,0.850062154990044,0.763267162946389,0.684540586068198,0.614847646639364,0.554480504077034,0.503516770374833,0.461361265900861,0.425305866940255,0.392265436032573,0.360784989220427,0.331651459838179,0.305809562547664,0.283680952057226,0.264726217844759,0.248308541588640,0.233888921420965,0.221010345667801,0.209218337381434,0.198087891151168,0.187559994239798,0.177823359124246,0.169071380979016,0.161441223397985,0.154845123652523,0.149139087433375,0.144179120431287,0.139821228337003};
double abs_coeff_4[100]={64.1358801599406,41.8952038630826,30.6239528962595,29.5087945193646,31.0089173594454,33.3390433748053,37.0914070663736,42.0843208788208,46.3770063618939,46.4079345150738,41.0402779038909,31.2199437954794,20.2255392247903,12.1137427096706,7.29586867081893,4.14500603563445,2.32140335738645,1.64915823374025,1.30944463225625,0.963574126171436,0.682997757565818,0.490456581709513,0.369449065027833,0.282874807646021,0.234489416831792,0.234872969222347,0.248689196598861,0.289590261178757,0.357210885720668,0.438403995282204,0.530196401663691,0.642855972779668,0.786780545524846,0.959803675600237,1.14922757331561,1.34339736361406,1.53890365800198,1.73514390934921,1.92697432747014,2.10544025618374,2.26251911814823,2.40623877155787,2.55715922842947,2.72335866509862,2.86564665047025,2.93649713067877,2.92752998887212,2.86093076181956,2.75969654679872,2.64568678037466,2.52795839797991,2.40750508704559,2.28519470331115,2.16189494409322,2.03809886275690,1.91312276166913,1.78605250214736,1.65597394550903,1.52241209090667,1.38862898724214,1.25977539059293,1.14101663793036,1.03635795429595,0.944771921949381,0.863866766400940,0.791279812335937,0.725465298104544,0.665785533188788,0.611650256229697,0.562491961863629,0.517814620964879,0.477136201471953,0.439974892494539,0.405929403070524,0.374798368022650,0.346410923356081,0.320630057115690,0.297491605860209,0.277086430777809,0.259505416297927,0.244751995137024,0.232302680224786,0.221437593886356,0.211475066470336,0.202061416264216,0.193008726855715,0.184131126927337,0.175341887159304,0.166750443409013,0.158489282752601,0.150654377634730,0.143195641974155,0.136026475058153,0.129061313940308,0.122395108653202,0.116507700967162,0.111907060401872,0.108836753552613,0.107361383675208,0.107542171119145};
double abs_coeff_5[100]={0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000197122863818941,0.000354981349067595,0.000602511296069369,0.00138994757471589,0.00196453803821213,0.00213719243583554,0.00255070242617488,0.00289696712753675,0.00336126915686136,0.00393420397539607,0.00442448593661035,0.00476604732107348,0.00506052494909731,0.00542409977729402,0.00600950804913131,0.00699035333530123,0.00848645249805719,0.0105182996693395,0.0131297687299023,0.0169457019647613,0.0230769359731635,0.0320772887237335,0.0429657102665624,0.0545555188158214,0.0672483937698526,0.0833409711154367,0.104854161896041,0.130205093178669,0.155849531844359,0.179056993524257,0.201717629849685,0.227313889979766,0.258654144735573,0.293285806402890,0.326289143714910,0.352730247351557,0.368473117900405,0.374189374966214,0.372342507104071,0.365396086717778,0.355298848280040,0.342901783225582,0.328914936550398,0.314048353250484,0.298936700616869,0.283796726087327,0.268701159533838,0.253721537550833,0.238792211981792,0.223576097019207,0.207704210426460,0.190837889502721,0.173488917696075,0.157114423881235,0.143085300914156,0.131618472533842,0.122347645798265,0.114902025309635,0.108952910757168,0.104354213119831,0.101009349561813,0.0988217381501952,0.0976947969520574,0.0975197113809403,0.0981074856027715,0.0992368308588574,0.100686369688682,0.102292945558405,0.104190677984222,0.106608321397783,0.109762744881361,0.113736617630948,0.118528087313995,0.124131678424688,0.130359684402238,0.136713245082980,0.142663074728013,0.147760927660415,0.152189659875786,0.156428084544060,0.160956715626587,0.166237454376508,0.172210125913217,0.178234221979309,0.183638929268922,0.187764785825963,0.190379853485446,0.191799730857025,0.192375832047789,0.192459571164830,0.192402362315240};
double abs_coeff_6[100]={0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000100,0.000106071143091149,0.000147417137644518,0.000275048792474063,0.000406436770019764,0.000587902260417402,0.000959244332053151,0.00158642029708544,0.00249771961733381,0.00472020135912516,0.00937029205331210,0.0162664404135633,0.0269142935579098,0.0449109076870519,0.0714021948208209,0.110485289293005,0.167228579019925,0.233455199766415,0.312226026120499,0.441641192783902,0.636661085359467,0.868454232634700,1.11874273441747,1.39394083589940,1.70184170475805,2.04058903906123,2.40307478267286,2.78058423715366,3.15867040844519,3.52156490837837,3.85354453612031,4.13944731975213,4.36449119352017,4.51522444796655,4.59308551490310,4.60889090786103,4.57360349081093,4.49805946663524,4.39239278569507,4.26649539812608,4.13025589676981,3.99020213106841,3.84310951935907,3.68399449229554,3.50787348053158,3.31028897458194,3.09814902398425,2.88933391775311,2.70217131026588,2.55361882266805,2.43968941162820,2.34004216906020,2.23390529099875,2.10836276631699,1.97517351064404,1.85092848651346,1.74878739564657,1.66702490193733,1.59988031395936,1.54159286668952,1.48664186857074,1.43248798356990,1.37860983117335,1.32452417585588,1.26981871719154,1.21607084491383,1.16706966374259,1.12671977430487,1.09892201754544,1.08536381588792,1.08179394709468,1.08297556519293,1.08367182420984,1.07864587817261,1.06268092371714,1.03545684852199,1.00791531801678,0.991867541692858,0.989390555760633,0.995483438322247,1.00487906940307,1.01000747643067,1.00107552736176,0.968199446438494};
double temperature[100]={300,600,900,1200,1500,1800,2100,2400,2700,3000,3300,3600,3900,4200,4500,4800,5100,5400,5700,6000,6300,6600,6900,7200,7500,7800,8100,8400,8700,9000,9300,9600,9900,10200,10500,10800,11100,11400,11700,12000,12300,12600,12900,13200,13500,13800,14100,14400,14700,15000,15300,15600,15900,16200,16500,16800,17100,17400,17700,18000,18300,18600,18900,19200,19500,19800,20100,20400,20700,21000,21300,21600,21900,22200,22500,22800,23100,23400,23700,24000,24300,24600,24900,25200,25500,25800,26100,26400,26700,27000,27300,27600,27900,28200,28500,28800,29100,29400,29700,30000};

double input_time[Input_num]={3.06e-06,3.32e-04,3.58e-04,3.83e-04,4.09e-04,4.34e-04,4.60e-04,4.86e-04,5.11e-04,5.37e-04,5.62e-04,5.88e-04,6.14e-04,6.39e-04,6.65e-04,6.90e-04,7.16e-04,7.42e-04,7.67e-04,7.93e-04,8.18e-04,8.44e-04,8.70e-04,8.95e-04,9.21e-04,9.46e-04,9.72e-04,9.98e-04,1.02e-03,1.05e-03,1.07e-03,1.10e-03,1.13e-03,1.15e-03,1.18e-03,1.20e-03,1.23e-03,1.25e-03,1.28e-03,1.30e-03,1.33e-03,1.36e-03,1.38e-03,1.41e-03,1.43e-03,1.46e-03,1.48e-03,1.51e-03,1.54e-03,1.56e-03,1.59e-03,1.61e-03,1.64e-03,1.66e-03,1.69e-03,1.71e-03,1.74e-03,1.77e-03,1.79e-03,1.82e-03,1.84e-03,1.87e-03,1.89e-03,1.92e-03,
	1.94e-03,1.97e-03,2.00e-03,2.02e-03,2.05e-03,2.07e-03,2.10e-03,2.12e-03,2.15e-03,2.18e-03,2.20e-03,2.23e-03,2.25e-03,2.28e-03,2.30e-03,2.33e-03,2.35e-03,2.38e-03,2.41e-03,2.43e-03,2.46e-03,2.48e-03,2.51e-03,2.53e-03,2.56e-03,2.58e-03,2.61e-03,2.64e-03,2.66e-03,2.69e-03,2.71e-03,2.74e-03,2.76e-03,2.79e-03,2.82e-03,2.84e-03,2.87e-03,2.89e-03,2.92e-03,2.94e-03,2.97e-03,2.99e-03,3.02e-03,3.05e-03,3.07e-03,3.10e-03,3.12e-03,3.15e-03,3.17e-03,3.20e-03,3.22e-03,3.25e-03,3.28e-03,3.30e-03,3.33e-03,3.35e-03,3.38e-03,3.40e-03,3.43e-03,3.46e-03,3.48e-03,3.51e-03,3.53e-03,3.56e-03,
	3.58e-03,3.61e-03,3.63e-03,3.66e-03,3.69e-03,3.71e-03,3.74e-03,3.76e-03,3.79e-03,3.81e-03,3.84e-03,3.86e-03,3.89e-03,3.92e-03,3.94e-03,3.97e-03,3.99e-03,4.02e-03,4.04e-03,4.07e-03,4.10e-03,4.12e-03,4.15e-03,4.17e-03,4.20e-03,4.22e-03,4.25e-03,4.27e-03,4.30e-03,4.33e-03,4.35e-03,4.38e-03,4.40e-03,4.43e-03,4.45e-03,4.48e-03,4.50e-03,4.53e-03,4.56e-03,4.58e-03,4.61e-03,4.63e-03,4.66e-03,4.68e-03,4.71e-03,4.74e-03,4.76e-03,4.79e-03,4.81e-03,4.84e-03,4.86e-03,4.89e-03,4.91e-03,4.94e-03,4.97e-03,4.99e-03,5.02e-03,5.04e-03,5.07e-03,5.09e-03,5.12e-03,5.14e-03,5.17e-03,5.20e-03,
	5.22e-03,5.25e-03,5.27e-03,5.30e-03,5.32e-03,5.35e-03,5.38e-03,5.40e-03,5.43e-03,5.45e-03,5.48e-03,5.50e-03,5.53e-03,5.55e-03,5.58e-03,5.61e-03,5.63e-03,5.66e-03,5.68e-03,5.71e-03,5.73e-03,5.76e-03,5.78e-03,5.81e-03,5.84e-03,5.86e-03,5.89e-03,5.91e-03,5.94e-03,5.96e-03,5.99e-03,6.02e-03,6.04e-03,6.07e-03,6.09e-03,6.12e-03,6.14e-03,6.17e-03,6.19e-03,6.22e-03,6.25e-03,6.27e-03,6.30e-03,6.32e-03,6.35e-03,6.37e-03,6.40e-03,6.42e-03,6.45e-03,6.48e-03,6.50e-03,6.53e-03,6.55e-03,6.58e-03,6.60e-03,6.63e-03,6.66e-03,6.68e-03,6.71e-03,6.73e-03,6.76e-03,6.78e-03,6.81e-03,6.83e-03,
	6.86e-03,6.89e-03,6.91e-03,6.94e-03,6.96e-03,6.99e-03,7.01e-03,7.04e-03,7.06e-03,7.09e-03,7.12e-03,7.14e-03,7.17e-03,7.19e-03,7.22e-03,7.24e-03,7.27e-03,7.30e-03,7.32e-03,7.35e-03,7.37e-03,7.40e-03,7.42e-03,7.45e-03,7.47e-03,7.50e-03,7.53e-03,7.55e-03,7.58e-03,7.60e-03,7.63e-03,7.65e-03,7.68e-03,7.70e-03,7.73e-03,7.76e-03,7.78e-03,7.81e-03,7.83e-03,7.86e-03,7.88e-03,7.91e-03,7.94e-03,7.96e-03,7.99e-03,8.01e-03,8.04e-03,8.06e-03,8.09e-03,8.11e-03,8.14e-03,8.17e-03,8.19e-03,8.22e-03,8.24e-03,8.27e-03,8.29e-03,8.32e-03,8.34e-03,8.37e-03,8.40e-03,8.42e-03,8.45e-03,8.47e-03,
	8.50e-03,8.52e-03,8.55e-03,8.58e-03,8.60e-03,8.63e-03,8.65e-03,8.68e-03,8.70e-03,8.73e-03,8.75e-03,8.78e-03};

double input_current[Input_num]={40.0,293.0,332.2,371.4,410.6,449.8,489.0,529.0,562.0,589.0,611.0,627.8,640.4,651.0,660.0,669.0,676.1,683.1,689.4,696.9,707.5,717.3,726.3,733.7,742.4,753.7,765.9,779.2,792.9,804.7,816.9,827.8,836.9,844.7,854.5,864.3,875.7,886.7,895.3,903.5,911.4,919.2,929.0,937.6,945.9,958.0,969.0,980.0,990.6,1000.8,1011.0,1022.0,1031.0,1040.8,1052.5,1065.1,1077.6,1089.8,1099.2,1109.8,1118.8,1127.8,1136.9,1144.3,1152.9,1162.4,1170.2,1176.5,1180.8,1185.5,1189.0,1192.5,1194.1,1192.5,1188.6,1182.4,1174.1,1166.7,1160.0,1155.3,1151.8,1150.6,1152.5,1153.7,1156.5,1160.4,1167.5,1175.3,1183.5,1189.8,1194.9,
	1196.1,1194.9,1191.4,1185.1,1174.9,1164.3,1153.3,1140.4,1126.3,1114.1,1105.1,1097.3,1090.2,1084.3,1077.6,1069.8,1062.4,1057.3,1053.3,1049.0,1047.1,1046.3,1043.1,1040.0,1038.0,1038.0,1038.4,1039.2,1041.6,1045.9,1049.4,1053.7,1058.4,1062.4,1066.7,1071.8,1074.5,1077.6,1075.7,1071.8,1066.3,1059.2,1051.8,1043.5,1034.1,1025.1,1014.5,1003.9,994.1,984.3,974.9,967.8,962.7,959.2,955.3,950.6,944.3,937.6,930.2,922.4,912.9,903.9,893.7,882.7,871.4,861.2,850.6,841.2,830.2,820.0,810.2,800.8,793.7,787.5,782.0,777.6,774.9,773.3,770.2,767.1,762.7,757.6,753.3,748.2,742.0,734.5,727.1,718.8,708.6,697.3,
	684.7,671.0,657.6,644.7,632.2,619.6,606.7,592.9,578.4,562.4,547.1,531.8,515.7,500.0,485.1,470.6,456.5,442.7,429.0,417.6,409.0,401.2,394.5,387.5,380.8,374.5,368.6,362.7,356.5,350.2,343.9,338.4,332.2,325.5,318.8,311.4,305.5,298.8,292.9,288.6,283.1,278.0,273.7,269.0,264.3,257.3,251.4,245.5,241.2,236.9,234.1,232.2,230.6,230.6,230.6,229.4,227.8,227.8,227.8,226.3,223.9,220.0,215.7,211.4,206.3,199.6,192.9,185.1,179.2,172.5,164.7,157.6,151.4,145.5,140.8,135.7,130.6,124.7,119.2,114.5,111.4,107.1,103.9,100.8,98.4,96.1,94.1,91.4,89.4,88.2,86.7,85.1,83.1,80.8,78.8,77.6,76.5,75.7,74.9,73.7,74.9,
	74.9,76.1,76.9,78.0,79.2,80.0,79.6,78.4,78.4,77.3,76.5,76.5,74.5,71.4,68.6,67.1,65.1,64.3,64.3,65.5,67.5,68.2,67.1,66.7,66.7,67.8,70.2,72.5,73.3,74.9,74.5,72.2,68.6,65.1,62.7,60.4,58.4,56.5,53.3,50.2,45.5,42.0,38.0,36.1,35.8,35.9,36.0,36.1,36.2,36.3};

/*

k1_V97_0.01_1.8_2.2_J5e3_4e4_8e4_4g

*/

double Cathode_Drop(const double j)
{
  double Vc0=VdropC*0.2;
  double Vc_ig=VdropC*2.2;
  double jc_0=5e2, jc_ig=3e4, jcf=0.6e5;
  double c0, c1, c2;

  c0=(Vc_ig-Vc0)/(pow(jc_0,3)*(2*jc_ig-jc_0));
  c1=(Vc_ig-Vc0)/((jc_ig-jc_0)*(jc_ig-jc_0/2));
  c2=pow(jcf-jc_ig,2)/log(1000);
  
  if(j<=jc_0)
    return c0*pow(j,4)+Vc0;
  else if(j<=jc_ig)
  	return -c1*pow(jc_ig-j,2) + Vc_ig;
  else
  	return (Vc_ig-VdropC) * exp(-pow(j-jc_ig,2)/c2) + VdropC;
}

double Anode_Drop(const double j)
{
  double Va0=VdropA*0.2;
  double Va_ig=VdropA*3.1;
  double ja_0=5e2, ja_ig=3e4, jaf=0.6e5;
  double a0, a1, a2;
  
  a0=(Va_ig-Va0)/(pow(ja_0,3)*(2*ja_ig-ja_0));
  a1=(Va_ig-Va0)/((ja_ig-ja_0)*(ja_ig-ja_0/2));
  a2=pow(jaf-ja_ig,2)/log(1000);

  if (j<=ja_0)
    return a0*pow(j,4)+Va0;
  else if(j<=ja_ig)
  	return -a1*pow(ja_ig-j,2) + Va_ig;
  else 
  	return (Va_ig-VdropA)*exp(-pow(j-ja_ig,2)/a2) + VdropA;
}

double GetArcSpotRadius(double PowerIn, double EvapRate)
{
	 double arc_radius;
	 arc_radius = sqrt(pow(thermalcond_cu*(TempBoilCu-TempAmbient)/EvapRate/h_cu_mole, 2) + PowerIn/(EvapRate*h_cu_mole*M_PI))-thermalcond_cu*(TempBoilCu-TempAmbient)/(EvapRate*h_cu_mole);
	 return arc_radius;
}

/* Interpolation. based on u inpt, find it position i in x[], we try to find the y[i] value by interpolating as the output */
double interp(const double xx[], const double yy[], const int n, const double input)
{
  if(input >= xx[n-1])
  	   return yy[n-1];
  else if (input <= xx[0])
   	   return yy[0];
  else
  {
  	   int i=0,j=n-1,a;
	   while (j-i!=1)
	   {
		  a=(i+j)/2;               /* 2.95 will be 2 */
		  if (input < xx[a]) j=a;
		  else i=a;
	   }
	   return (yy[i]+(input-xx[i])*(yy[j]-yy[i])/(xx[j]-xx[i]));
  }
}

double InterpolateAbsorptionCoefficient(double T, double P, double muCu, double muAir, double muPa66, int NumBand)
{
	double ret;
	double aCu;
	double aAir;
	double nuCu;
	double nuAir;
	double MixMolMass;
	double Ysh[NumberOfArguments];
	Ysh[0] = muPa66 * 100.0;  
	Ysh[1] = muCu * 100.0;  
	Ysh[2] = T;
	if(Ysh[2]>30000.0) Ysh[2]=30000.0;
	Ysh[3] = (P + P0fl) / 101325.0;                                              
	MixMolMass = FindApproximation(Ysh,&table,Num_MolWeight);                   
	nuCu  = muCu * MixMolMass / MolMassCu;                                     
	nuAir = muAir * MixMolMass / MolMassCu;                                    
	aCu  = 100.0 * Approximate_P1_TABLE( &p1_table_cu, Ysh[3], Ysh[2], NumBand); 
	aAir = Approximate_P1_TABLE( &p1_table_air, Ysh[3], Ysh[2], NumBand);
	ret  = nuCu * aCu + nuAir * aAir;
	return ret;
}

DEFINE_EXECUTE_AFTER_CASE(load_fluentdata, libname)                                      
{   
	int ret;  /*Useless*/
	Message("Test initial DEFINE_EXECUTE_AFTER_CASE. mark1 \n");    

	if(Init_TABLE(&table,100)==1)
	{
		Message("Can not allocate initial memory for mixture properties table during start of the case.\n");
		return;
	}
	else
	{
		Message("Initial memory for mixture properties table was allocated.\n");
		LoadFlag = 1;
	}

	if( Init_P1_TABLE(Nband,1,&p1_table_cu) == 1 )
	{
		Message("Can not allocate initial memory for P1 model table with absorption coefficients for copper  during start of the case.\n");
		return;
	}
	else
	{
		Message("Initial memory for P1 model table with absorption coefficients for copper was allocated.\n");
		LoadFlagP1Cu = 1;
	}

	if( Init_P1_TABLE(Nband,1,&p1_table_air)==1 )
	{
		Message("Can not allocate initial memory for P1 model table with absorption coefficients for air during start of the case.\n");
		return;
	}
	else
	{
		Message("Initial memory for P1 model table with absorption coefficients for air was allocated.\n");
		LoadFlagP1Air = 1;
	}

#if LoadTableAtCaseStart!=0     
	ret = FileRead(FileName,&table);
	if(ret==1)
	{
		Message("Reading of mixture properties table failed.\n");
	}
	else
	{
		Message("Reading of mixture properties table done.\n");
	}
#endif                        
#if LoadTableAtCaseStartCu!=0
     ret = Read_P1_TABLE(FileNameP1Cu,&p1_table_cu);
	 if(ret==1)
	{
		Message("Reading of P1 model table with absorption coefficients for copper failed.\n");
	}
	else
	{
		Message("Reading of P1 model table with absorption coefficients for copper done.\n");
	}
#endif                            
#if LoadTableAtCaseStartAir!=0
     ret = Read_P1_TABLE(FileNameP1Air,&p1_table_air);
	 if(ret==1)
	{
		Message("Reading of P1 model table with absorption coefficients for air failed.\n");
	}
	else
	{
		Message("Reading of P1 model table with absorption coefficients for air done.\n");
	}
#endif 
}

DEFINE_EXECUTE_AT_EXIT(unload_fluentdata)  
{  
	Message("Test initial DEFINE_EXECUTE_AT_EXIT. Fluent exits soon.\n"); 
	/*
	if(LoadFlag!=0)    Remove_TABLE(&table);
	if(LoadFlagP1Air!=0) Delete_P1_TABLE(&p1_table_air);
	if(LoadFlagP1Cu!=0)  Delete_P1_TABLE(&p1_table_cu);  */
}

DEFINE_ON_DEMAND(A_mixture_property_table)   
{
	#if !RP_HOST
	int ret;	
	Message("Starting load  mixture properties table. Mark3 \n");	
	if(LoadFlag!=0)
	{
		Remove_TABLE(&table);
	    LoadFlag = 0;
	}
	if(Init_TABLE(&table,100)==1)
	{
		Message("Can not allocate initial memory for mixture properties table  during load on demand.\n");
		Message("Failed.\n");
		return;
	}
	else
	{
		LoadFlag = 1;
	}
   	ret = FileRead(FileName,&table);    
	
	if(ret==1)
	{
		Message("Loading of mixture properties table was failed.\n");
	}
	else
	{
		Message("Loading of mixture properties table was done.\n");
	}	
	#endif
}

DEFINE_ON_DEMAND(B_p1_Cu_absorption_coeff_table)   
{
	#if !RP_HOST
	int ret;	
	Message("Starting load P1 model table with absorption coefficients for copper. Mark4\n");
	
	if(LoadFlagP1Cu!=0)
	{
		Delete_P1_TABLE(&p1_table_cu);
	    LoadFlagP1Cu = 0;
	}
	if(Init_P1_TABLE(Nband,1,&p1_table_cu)==1)
	{
		Message("Can not allocate initial memory for P1 model table with absorption coefficients for copper  during load on demand.\n");
		Message("Failed.\n");
		return;
	}
	else
	{
		LoadFlagP1Cu = 1;
	}
	ret = Read_P1_TABLE(FileNameP1Cu,&p1_table_cu);   
	if(ret==1)
	{
		Message("Loading of P1 model table with absorption coefficients for copper was failed.\n");
	}
	else
	{
		Message("Loading of P1 model table with absorption coefficients for copper was done.\n\n");
	}    
	#endif
}

DEFINE_ON_DEMAND(C_p1_Air_absorption_coeff_table)    
{
	#if !RP_HOST
	int ret;	
	Message("Starting load P1 model table with absorption coefficients for air. Mark5\n");

	if(LoadFlagP1Air!=0)
	{
		Delete_P1_TABLE(&p1_table_air);
	    LoadFlagP1Air = 0;
	}
	if(Init_P1_TABLE(Nband,1,&p1_table_air)==1)
	{
		Message("Can not allocate initial memory for P1 model table with absorption coefficients for air during load on demand.\n");
		Message("Failed.\n");
		return;
	}
	else
	{
		LoadFlagP1Air = 1;
	}
	ret = Read_P1_TABLE(FileNameP1Air,&p1_table_air);
	if(ret==1)
	{
		Message("Loading of P1 model table with absorption coefficients for air was failed.\n");
	}
	else
	{
		Message("Loading of P1 model table with absorption coefficients for air was done.\n\n");
	}   
	#endif
}

DEFINE_INIT(A_init_TempPressure,d)       
{                                   
	#if !RP_HOST
	cell_t cell;
	Thread *thread;
	double xc[ND_ND];            
	thread_loop_c(thread,d)
	{		
		if (FLUID_THREAD_P(thread))
		{	
			begin_c_loop(cell,thread)              
			{ 
				C_CENTROID(xc,cell,thread);    
				if (xc[0]>=-10e-3 && xc[0]<=10e-3)  
				{
				  C_T(cell,thread) = 300 + 50*exp(-(pow((xc[1]-Y_PLASMA),2)+pow(xc[2]-Z_PLASMA,2))/(2*pow(ARC_RADIUS,2)))/(ARC_RADIUS*sqrt(2*M_PI));   			
				  C_P(cell,thread) = 300 + 10*exp(-(pow((xc[1]-Y_PLASMA),2)+pow(xc[2]-Z_PLASMA,2))/(2*pow(ARC_RADIUS,2)))/(ARC_RADIUS*sqrt(2*M_PI));   
				}				 
			}
			end_c_loop(cell,thread)
		}
	}
	#endif

	#if RP_NODE
    Message("Initialize T and P. nodeID=%d \n", myid);
	#endif
}

DEFINE_INIT(B_cu_pa66_species_initialize, d)      
{                                                     
	#if !RP_HOST
	cell_t cell;
	face_t f;
	Thread *ct;
	Thread *tf = Lookup_Thread(d, Current_input_faceID);
	double Cu0 = 0.0;
	double Pa660 = 0.0;	
	LangmuirConstantRate = CuSaturatedVaporP/sqrt(2*M_PI*Cu_atomic_mass*KB*TempBoilCu)/Avogadro_constant; 
    /*  the unit is mol/m2/s for langmuirConstantRate  */
	thread_loop_c(ct, d)
	{		 
		if (FLUID_THREAD_P(ct))                             
		{   			
			begin_c_loop_int(cell, ct)
			{
				C_YI(cell, ct, I_Pa66) = Pa660;
				C_YI(cell, ct, I_Cu)   = Cu0;
				C_YI(cell, ct, I_Air)  = 1.0 - Cu0 - Pa660;   
			}
			end_c_loop_int(cell, ct)
		}		
	}
    /* calculate the current input area */
	begin_f_loop(f,tf)        
    {
      double A[ND_ND];	
      if (PRINCIPAL_FACE_P(f,tf))
	   {  
		  F_AREA(A,f,tf);
		  CurrentInput_area += NV_MAG(A);   
	   }	  	
	}
	end_f_loop(f,tf)
	# if RP_NODE
	  CurrentInput_area = PRF_GRSUM1(CurrentInput_area);    
	#endif  

    #endif   
    node_to_host_real_1(CurrentInput_area);    
}

DEFINE_EXECUTE_AT_END(A_Sigma_update) 
{
    #if !RP_HOST                        /* to update the UDM_ECond, executed at the end of an time step in a steady-state run, or at the end of a time step in a transient run.*/
    Domain *d;
    double Ysh[NumberOfArguments];
	Thread *thread;
	cell_t cell;

	d = Get_Domain(1);                  /* Get the domain using ANSYS FLUENT utility, 1 is for the mixture domain, just only fluid domain, without solid domain */		
	thread_loop_c(thread,d)
	{
		if (THREAD_ID(thread)==cellzone_cathode_drop)			                                                                   
		{  
			begin_c_loop(cell,thread)
			{
			    C_UDMI(cell,thread,UDM_Vdrop)= Cathode_Drop(C_UDMI(cell,thread,UDM_Jmag));  
			/* 	C_UDMI(cell,thread,UDM_Vdrop)= interp(JJ_Density,Catho_Vdrop,J_num,C_UDMI(cell,thread,UDM_Jmag)); */
				C_UDMI(cell,thread,UDM_ECond)= C_UDMI(cell,thread,UDM_Jmag)*Shealth_Thickness/C_UDMI(cell,thread,UDM_Vdrop);   
			}
			end_c_loop(cell,thread)
		}
		else if (THREAD_ID(thread)==cellzone_anode_drop)
		{ 
			begin_c_loop(cell,thread)
			{
                C_UDMI(cell,thread,UDM_Vdrop)= Anode_Drop(C_UDMI(cell,thread,UDM_Jmag));     
             /*   C_UDMI(cell,thread,UDM_Vdrop)= interp(JJ_Density,Anode_Vdrop,J_num,C_UDMI(cell,thread,UDM_Jmag)); */
                C_UDMI(cell,thread,UDM_ECond)= C_UDMI(cell,thread,UDM_Jmag)*Shealth_Thickness/C_UDMI(cell,thread,UDM_Vdrop); 
			}
			end_c_loop(cell,thread)
		}
		else if (THREAD_ID(thread)==cellzone_air || THREAD_ID(thread)==cellzone_anode_ionlay || THREAD_ID(thread)==cellzone_cathode_ionlay)
		{            
			begin_c_loop (cell,thread)
			{
				Ysh[0]=C_YI(cell,thread,I_Pa66) * 100.0; 
				Ysh[1]=C_YI(cell,thread,I_Cu) * 100.0;
				Ysh[2]=C_T(cell,thread);
				if(Ysh[2]>30000.0) Ysh[2]=30000.0;
				Ysh[3]=(C_P(cell,thread)+P0fl)/101325.0;
				C_UDMI(cell,thread,UDM_ECond) = FindApproximation(Ysh,&table,Num_Sigma);
			}
			end_c_loop (cell,thread)
		}
	}
	#endif
	/* This happen early than difine_profile.  Due to the symmetry model, thus we need divide it by 2  */   
	/*Current_BC=ABS(CurrAmpl*sin(2*M_PI*CurrFreq*CURRENT_TIME + M_PI/6.0))+0.001*CurrAmpl;            */          
	Current_BC=interp(input_time,input_current,Input_num,CURRENT_TIME)/2;   
	if (Current_BC<CurrAmpl)  Current_BC=CurrAmpl;
	#if RP_HOST
    Message("\n Step %d starts next, StepSize=%.7lf, FlowTime=%.7lf. Current=%.8lf, CurrentInArea =%.8lf \n", N_TIME, CURRENT_TIMESTEP, CURRENT_TIME, Current_BC, CurrentInput_area);
    #endif
}

DEFINE_EXECUTE_AT_END(B_mole_fraction) 
{
    #if !RP_HOST
    Domain *d;
    Thread *tf;	
	cell_t c;
	Material *mix_mat = mixture_material(Get_Domain(1));
    Material *spe_mat=NULL;
    double all_mass_fracts[MAX_SPE_EQNS]; 
    double all_mole_fracts[MAX_SPE_EQNS];

	d = Get_Domain(1);
	thread_loop_c(tf,d)                                                                                                                                                                       
    {
       if(THREAD_ID(tf)==cellzone_anode_drop || THREAD_ID(tf)==cellzone_cathode_drop || THREAD_ID(tf)==cellzone_anode_ionlay || THREAD_ID(tf)==cellzone_cathode_ionlay)
		{
            begin_c_loop(c,tf)
            {  				
            	int i=-1;
		        mixture_species_loop(mix_mat, spe_mat, i)
		        {
		        	all_mass_fracts[i] =C_YI(c,tf,i);
		        }
	            Mole_Fraction(mix_mat, all_mass_fracts, all_mole_fracts);
                C_UDMI(c,tf,UDM_Cu_mole)=all_mole_fracts[I_Cu];  
            }
            end_c_loop(c,tf)
         }
    }
	#endif
}

DEFINE_ADJUST(A_uds_number_check,domain)
{                                              /*  Domain is passed by the ANSYS Fluent solver to your UDF.                            */
	if (n_uds < N_REQUIRED_UDS)                /*  N_UDS  access the number of UDS  equations that have been specified in ANSYS Fluent */
    	Internal_Error("not enough user-defined scalars allocated \n");
}

DEFINE_ADJUST(B_Maxwell_Equation,d) 
{   /*  Calculate: E, J, B, F(Ampere force). But the UDS(V, Ax, Ay, Az) should be known.*/
    #if !RP_HOST
	Thread *thread;
	cell_t cell;
	if (! Data_Valid_P())  return;      /*  returns 1 (true) if the data that is passed as an argument is valid, and 0 (false) if it is not. */
	                                    /*  check that the cell values of the variables that appear in your UDF are accessible. Unilt it is accessible, the code will continue.*/  
	thread_loop_c (thread, d)           /*  loop at zone in one domain, since it is a single domain problem.*/
	{  
		if(NULL != THREAD_STORAGE(thread,SV_UDS_I(0)) && NULL != T_STORAGE_R_NV(thread,SV_UDSI_G(0)))  /*  0 represent the uds0, the voltage.*/
		{  
			begin_c_loop (cell,thread)   
			{
				C_UDMI(cell,thread,UDM_Ex)=-C_UDSI_G(cell,thread,UDS_V)[0];                                        /*  get the electric field intensity  E=-delte V    */
				C_UDMI(cell,thread,UDM_Ey)=-C_UDSI_G(cell,thread,UDS_V)[1];
				C_UDMI(cell,thread,UDM_Ez)=-C_UDSI_G(cell,thread,UDS_V)[2];
				C_UDMI(cell,thread,UDM_Emag)=sqrt(pow(C_UDMI(cell,thread,UDM_Ex),2)+pow(C_UDMI(cell,thread,UDM_Ey),2)+pow(C_UDMI(cell,thread,UDM_Ez),2));  
				
				C_UDMI(cell,thread,UDM_Jx)=C_UDSI_DIFF(cell,thread,UDS_V)*C_UDMI(cell,thread,UDM_Ex);              /*  J=sigma*E. but C_UDSI_DIFF is sigma (not -sigma).*/
				C_UDMI(cell,thread,UDM_Jy)=C_UDSI_DIFF(cell,thread,UDS_V)*C_UDMI(cell,thread,UDM_Ey);            
				C_UDMI(cell,thread,UDM_Jz)=C_UDSI_DIFF(cell,thread,UDS_V)*C_UDMI(cell,thread,UDM_Ez);       
				C_UDMI(cell,thread,UDM_Jmag)=sqrt(pow(C_UDMI(cell,thread,UDM_Jx),2)+pow(C_UDMI(cell,thread,UDM_Jy),2)+pow(C_UDMI(cell,thread,UDM_Jz),2));       
                				
				C_UDMI(cell,thread,UDM_Bx)=C_UDSI_G(cell,thread,UDS_Az)[1]-C_UDSI_G(cell,thread,UDS_Ay)[2];        /*  B=Delta x A  */
				C_UDMI(cell,thread,UDM_By)=C_UDSI_G(cell,thread,UDS_Ax)[2]-C_UDSI_G(cell,thread,UDS_Az)[0];                      
				C_UDMI(cell,thread,UDM_Bz)=C_UDSI_G(cell,thread,UDS_Ay)[0]-C_UDSI_G(cell,thread,UDS_Ax)[1];                      
				C_UDMI(cell,thread,UDM_Bmag)=sqrt(pow(C_UDMI(cell,thread,UDM_Bx),2)+pow(C_UDMI(cell,thread,UDM_By),2)+pow(C_UDMI(cell,thread,UDM_Bz),2));  
				
				C_UDMI(cell,thread,UDM_Fx)=C_UDMI(cell,thread,UDM_Jy)*C_UDMI(cell,thread,UDM_Bz)-C_UDMI(cell,thread,UDM_Jz)*C_UDMI(cell,thread,UDM_By);    /* F=J x B, F is the Lorentz force, as momentum source */
				C_UDMI(cell,thread,UDM_Fy)=C_UDMI(cell,thread,UDM_Jz)*C_UDMI(cell,thread,UDM_Bx)-C_UDMI(cell,thread,UDM_Jx)*C_UDMI(cell,thread,UDM_Bz);
				C_UDMI(cell,thread,UDM_Fz)=C_UDMI(cell,thread,UDM_Jx)*C_UDMI(cell,thread,UDM_By)-C_UDMI(cell,thread,UDM_Jy)*C_UDMI(cell,thread,UDM_Bx);	
				C_UDMI(cell,thread,UDM_Fmag)=sqrt(pow(C_UDMI(cell,thread,UDM_Fx),2)+pow(C_UDMI(cell,thread,UDM_Fy),2)+pow(C_UDMI(cell,thread,UDM_Fz),2));  
			}
			end_c_loop (cell,thread)			
		}		
	}
	#endif
}

/* huojindong@gmail.com */
DEFINE_ADJUST(C_ArcRoot_Energy,d)
{
	#if !RP_HOST 
	double CellV,CellV_shadow,AREA,Area2V,Area2V_shadow;
	double ji, je, jn, Local_vdrop;
	double energyin, R, Evap_rate;
	double A0[ND_ND],A1[ND_ND]; 
	Thread *tf, *tf_shadow;
	face_t f, f_shadow; 	          

	if (! Data_Valid_P())  return;                                              /* Make sure need variable is already exited. Return means the function stop here and return to main function.*/                                                
	thread_loop_f (tf, d)                                                       /* Use thread_loop_f when you want to loop over all face threads (like a geometry face) in a given domain.*/
	{
		if (THREAD_ID(tf)==wall_cathode_cathode_drop_f )                        /* Cathode    */
		{                                                                       /* BOUNDARY_FACE_THREAD_P make sure it is BC face  */
		    begin_f_loop (f,tf)                                                 /* begin_f_loop and end_f_loop loop over all faces in a given face thread */
			{
				if (PRINCIPAL_FACE_P(f,tf))
			    {		
					F_AREA(A0,f,tf);                                            /*  to get the face Area vector        */                
			     	f_shadow=F_SHADOW(f,tf);
	                tf_shadow=THREAD_SHADOW(tf);	

					AREA=NV_MAG(A0);
					CellV=C_VOLUME(F_C0(f,tf),THREAD_T0(tf));
		            CellV_shadow=C_VOLUME(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow));                                           /* tf_shadow is face thread, THREAD_T0(tf_shadow) is the Cell thread*/
					Area2V=AREA/CellV;              				                                                                /* Area/Volumn  */
		            Area2V_shadow=AREA/CellV_shadow;
					ji=0.3*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jmag);           /* ji is the ion current density       */
					je=0.7*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jmag);           /* je is the electron current density  */
					jn=ABS((A0[0]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jx)+A0[1]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jy)+A0[2]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jz))/AREA);
					
	                Local_vdrop= C_UDMI(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow),UDM_Vdrop);
				    energyin=0.3*Current_BC*(Local_vdrop+Vi+0.645);                                                                 /* Unit for energyin is W, 0.645=5Kb/2e*Tc              */
		            Evap_rate=LangmuirConstantRate*(1.0-C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Cu_mole));                              /* partial pressure = pressure(1 bar) X mole fraction   */

			        R= GetArcSpotRadius(energyin, Evap_rate);
	                F_UDMI(f,tf,UDM_Vap2cond_F)=R*Evap_rate*h_cu_mole/(2*thermalcond_cu*(TempBoilCu-TempAmbient));

					F_UDMI(f,tf,UDM_Qion_F)   = ji*(Local_vdrop+Vi+0.645);                                                          /* 0.645=5Kb/2e*Tc*/         				
					F_UDMI(f,tf,UDM_Qe_F)     = je*PHIc;                                                                            /* jouling heat divided into two part: Qe and Qion. */
	                F_UDMI(f,tf,UDM_Qvapcu_F) = F_UDMI(f,tf,UDM_Qion_F)*F_UDMI(f,tf,UDM_Vap2cond_F)/(1+F_UDMI(f,tf,UDM_Vap2cond_F));
	                F_UDMI(f,tf,UDM_Qcondcu_F)= F_UDMI(f,tf,UDM_Qion_F)/(1+F_UDMI(f,tf,UDM_Vap2cond_F));                            /* this part of energy should added into solid      */			  

			    	/* Message("Surface ID= %d, UDM_Qion_F = %f, Jmag=%f  \n", THREAD_ID(tf),F_UDMI(f,tf,UDM_Qion_F),C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jmag));                        */
	                C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_ESrootvap) = F_UDMI(f,tf,UDM_Qvapcu_F)*Area2V;                              /* evaporation will absorb some heat, so add the same amount heat source*/
					C_UDMI(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow),UDM_ESrootcond)= F_UDMI(f,tf,UDM_Qcondcu_F)*Area2V_shadow; /* source term for the solid domain                 */    
				}						
            }
            end_f_loop (f,tf)            
		}

		if (THREAD_ID(tf)==wall_anode_anode_drop_f)       /*  Anode  */
		{
			begin_f_loop (f,tf)         
			{
				if (PRINCIPAL_FACE_P(f,tf))
			    {	
					F_AREA(A1,f,tf);
			     	f_shadow=F_SHADOW(f,tf);
	                tf_shadow=THREAD_SHADOW(tf);

					CellV=C_VOLUME(F_C0(f,tf),THREAD_T0(tf));
					AREA=NV_MAG(A1);
					Area2V=AREA/CellV;
	                CellV_shadow=C_VOLUME(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow));                         /* tf_shadow is face thread, THREAD_T0(tf_shadow) is the Cell thread  */
	                Area2V_shadow=AREA/CellV_shadow;
					jn=ABS((A1[0]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jx)+A1[1]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jy)+A1[2]*C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jz))/AREA);  /*  For normal to the surface, Jmag is too big          */

	                Local_vdrop= C_UDMI(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow),UDM_Vdrop);
				    energyin=jn/C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Jmag)*Current_BC*(Local_vdrop+PHIa+0.645);   /* unit is J/s                            */
	                Evap_rate=LangmuirConstantRate*(1.0-C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_Cu_mole));           /* assume the pressure is constant 1 bar  */
	                R= GetArcSpotRadius(energyin, Evap_rate);                
	                F_UDMI(f,tf,UDM_Vap2cond_F)=R*Evap_rate*h_cu_mole/(2*thermalcond_cu*(TempBoilCu-TempAmbient));

					F_UDMI(f,tf,UDM_Qecap_F)   = jn*(Local_vdrop+PHIa+0.645);                                     /* 0.645=5Kb/2e*(Tg-Ta)                        */   
	                F_UDMI(f,tf,UDM_Qvapcu_F)  = F_UDMI(f,tf,UDM_Qecap_F)*F_UDMI(f,tf,UDM_Vap2cond_F)/(1+F_UDMI(f,tf,UDM_Vap2cond_F));
	                F_UDMI(f,tf,UDM_Qcondcu_F) = F_UDMI(f,tf,UDM_Qecap_F)/(1+F_UDMI(f,tf,UDM_Vap2cond_F));        /* this part of energy should added into solid */
				
	                C_UDMI(F_C0(f,tf),THREAD_T0(tf),UDM_ESrootvap) = F_UDMI(f,tf,UDM_Qvapcu_F)*Area2V;
					C_UDMI(F_C0(f_shadow,tf_shadow),THREAD_T0(tf_shadow),UDM_ESrootcond)= F_UDMI(f,tf,UDM_Qcondcu_F)*Area2V_shadow;
				}
			}
			end_f_loop (f,tf)
		}
	}
	#endif
}

DEFINE_ADJUST(D_Wall_ablation,d)             
{   /* help/flu_udf/flu_udf_udf_uds_appl.html#flu_udf_udf_uds_p1radiation_application*/
    #if !RP_HOST

	double A[ND_ND],At,Rad_absorbed;
	double dr0[ND_ND],es[ND_ND],ds,A_by_es;   
	Thread *tf;                          /*= Lookup_Thread(d, Ablative_wall_air);  From the zone_ID(this is a number, not the thread), to find the thread point. */
    face_t f;
	thread_loop_f (tf, d)                /* Use thread_loop_f when you want to loop over all face threads (like a geometry face) in a given domain.*/
	{	                                 /* Semi-transport is not BOUNDARY_FACE_THREAD_P */ 
       if (THREAD_ID(tf)==Ablative_wall_air)   
       {
			begin_f_loop(f,tf)
			{
			  if (PRINCIPAL_FACE_P(f,tf))
			   {			        /* Semi-transparent no internal emssion*/
			        BOUNDARY_FACE_GEOMETRY(f,tf,A,ds,es,A_by_es,dr0);   
					At=NV_MAG(A); 
					F_UDMI(f,tf,UDM_QWallFlux_F) = -F_STORAGE_R(f,tf,SV_HEAT_FLUX)/At;     /* wall flux = RadFlux + conduction  */
					F_UDMI(f,tf,UDM_QRadFlux_F) = -F_STORAGE_R(f,tf,SV_RAD_HEAT_FLUX)/At;  /* This is transmitted part.         */

                    Rad_absorbed = Absorptivity_wall*F_UDMI(f,tf,UDM_QRadFlux_F);            
			        if (Rad_absorbed>1e7) Rad_absorbed=1e7;     

	                F_UDMI(f,tf,UDM_QCondFlux_F)= F_UDMI(f,tf,UDM_QWallFlux_F)-F_UDMI(f,tf,UDM_QRadFlux_F); 
	                if (F_UDMI(f,tf,UDM_QCondFlux_F)>1e5) F_UDMI(f,tf,UDM_QCondFlux_F)=1e5; 	        

			        F_UDMI(f,tf,UDM_Pa66_rate_F)=(F_UDMI(f,tf,UDM_QCondFlux_F)+Rad_absorbed)/h_pa66;
					if (F_UDMI(f,tf,UDM_Pa66_rate_F)<0)  F_UDMI(f,tf,UDM_Pa66_rate_F)=0.0;  /* No ablation,  unit is kg/m2/s.  WALL_TEMP_OUTER is the temperature inside the wall */  	 
			    }
			 }  
			end_f_loop(f,tf)			
		}	
	}
	#endif	
}

DEFINE_PROFILE(Current_input,tf,position)   
{	
	face_t f;
	begin_f_loop(f,tf)
	{
		if (PRINCIPAL_FACE_P(f,tf))  F_PROFILE(f,tf,position)=Current_BC/CurrentInput_area;	   
	}
	end_f_loop(f,tf) 
}

DEFINE_PROPERTY(air_visc_GEtable,cell,thread)                            
{   /*  specify a custom material property in ANSYS Fluent for single-phase and multiphase flows. */   
    double apr;
	double Ysh[NumberOfArguments];
	Ysh[0]=C_YI(cell,thread,I_Pa66) * 100.0; 
	Ysh[1]=C_YI(cell,thread,I_Cu) * 100.0;  
	Ysh[2]=C_T(cell,thread);
	if(Ysh[2]>30000.0) Ysh[2]=30000.0;
	Ysh[3]=(C_P(cell,thread)+P0fl)/101325.0;                          
	apr=FindApproximation(Ysh,&table,Num_Visc);
	return apr;
}

DEFINE_PROPERTY(air_thermal_conductivity_GEtable,cell,thread)            
{
	double apr;
	double Ysh[NumberOfArguments];
	Ysh[0]=C_YI(cell,thread,I_Pa66) * 100.0; 
	Ysh[1]=C_YI(cell,thread,I_Cu) * 100.0;   
	Ysh[2]=C_T(cell,thread);
	if(Ysh[2]>30000.0) Ysh[2]=30000.0;
	Ysh[3]=(C_P(cell,thread)+P0fl)/101325.0;
	apr=FindApproximation(Ysh,&table,Num_Leff);                         
	return apr;
}

DEFINE_SPECIFIC_HEAT(cp_GEtable,T,Tref,h,yi)     
{
    /* yi get the mass fraction of species. */
    /* Ysh[3] is the pressure. */    
	 double apr;
	 double Ysh[NumberOfArguments];
	 Ysh[0] = yi[I_Pa66] * 100.0;   
	 Ysh[1] = yi[I_Cu] * 100.0;
	 Ysh[2]=T;
	 if(Ysh[2]>30000.0) Ysh[2]=30000.0;
	 Ysh[3]=table.InitialArg[3];   
	 apr = 1000.0*FindApproximation(Ysh,&table,Num_Cpef);
	 *h =  1.0e6*FindApproximation(Ysh,&table,Num_H);
	 return apr;
	/*  MJ->J, *h this is sensible enthalpy, see the fluent manual
	double cp=2000;
    *h = cp*(T-Tref);
    return cp;     */
}

DEFINE_DIFFUSIVITY(Air_Econd,cell,thread,i)                           
{
    double Ysh[NumberOfArguments];
	Ysh[0]=C_YI(cell,thread,I_Pa66) * 100.0; 
	Ysh[1]=C_YI(cell,thread,I_Cu) * 100.0; 
	Ysh[2]=C_T(cell,thread);
	if(Ysh[2]>30000.0) Ysh[2]=30000.0;
	Ysh[3]=(C_P(cell,thread)+ P0fl )/101325.0;
	C_UDMI(cell,thread,UDM_ECond)= FindApproximation(Ysh,&table,Num_Sigma);
	if(C_UDMI(cell,thread,UDM_ECond)<SIGMA_MIN) return SIGMA_MIN;
    return C_UDMI(cell,thread,UDM_ECond);    
}

DEFINE_DIFFUSIVITY(Mixture_Econd,cell,thread,i)                       
{                                                                                  /* i is the index that identifies the species or user-defined scalar */
	if(C_UDMI(cell,thread,UDM_ECond)<SIGMA_MIN) return SIGMA_MIN;
	return C_UDMI(cell,thread,UDM_ECond);                             /* this was calculated by the B_Sigma_update */
}

/* UDS Source Term */
DEFINE_SOURCE(Ax_source,cell,thread,dS,eqn)          
{ /* (d/dxi) (-Fk dphi/dxi)=source, see the Maxwell function and the UDS: No negative sign before the Mu0 */
	dS[eqn]=0;
	return Mu0*C_UDMI(cell,thread,UDM_Jx);
}

DEFINE_SOURCE(Ay_source,cell,thread,dS,eqn)
{
	dS[eqn]=0;
	return Mu0*C_UDMI(cell,thread,UDM_Jy);
}

DEFINE_SOURCE(Az_source,cell,thread,dS,eqn)
{
	dS[eqn]=0;
	return Mu0*C_UDMI(cell,thread,UDM_Jz);
}

/*Momentum source Fx, Fy, Fz */
DEFINE_SOURCE(Lorentz_Fx,cell,thread,dS,eqn)
{
	dS[eqn]=0;
	return C_UDMI(cell,thread,UDM_Fx);
}

DEFINE_SOURCE(Lorentz_Fy,cell,thread,dS,eqn)
{ 
	dS[eqn]=0;
	return C_UDMI(cell,thread,UDM_Fy);
}

DEFINE_SOURCE(Lorentz_Fz,cell,thread,dS,eqn)
{
	dS[eqn]=0;
	return C_UDMI(cell,thread,UDM_Fz);
}

DEFINE_SOURCE(ES_solid_copper,cell,thread,dS,eqn)         
{   /* ESion ESe ESecap was not defined in solid region. */
	C_UDMI(cell, thread, UDM_ESCurrent)=pow(C_UDMI(cell,thread,UDM_Jmag), 2.)/C_UDSI_DIFF(cell,thread,UDS_V)+C_UDMI(cell,thread,UDM_ESrootcond); 
	dS[eqn]=0;
	return C_UDMI(cell, thread, UDM_ESCurrent);
}

DEFINE_SOURCE(ES_Joule_heating,cell,thread,dS,eqn)                
{   
	C_UDMI(cell,thread,UDM_ESCurrent) = pow(C_UDMI(cell,thread,UDM_Jmag), 2.)/C_UDSI_DIFF(cell,thread,UDS_V);     /* C_UDSI_DIFF is to  electrical conductivity*/	
	dS[eqn]=0;
	if (C_UDMI(cell,thread,UDM_ESCurrent)>Maximum_energy_source)  return Maximum_energy_source;
    return C_UDMI(cell,thread,UDM_ESCurrent);
}

DEFINE_SOURCE(ES_anode_drop,cell,thread,dS,eqn)           
{   
	C_UDMI(cell, thread, UDM_ESCurrent)=C_UDMI(cell,thread,UDM_ESrootvap);
	dS[eqn]=0;
	if (C_UDMI(cell,thread,UDM_ESCurrent)>Maximum_energy_source)  return Maximum_energy_source;
	return 0.0;
}

DEFINE_SOURCE(ES_cathode_drop,cell,thread,dS,eqn)        
{                                                                        
    C_UDMI(cell, thread, UDM_ESCurrent) = C_UDMI(cell,thread,UDM_ESrootvap);	
	dS[eqn]=0;
	if (C_UDMI(cell,thread,UDM_ESCurrent)>Maximum_energy_source)  return Maximum_energy_source;
	return 0.0;
}

DEFINE_GRAY_BAND_ABS_COEFF(gray_band_abs_air,cell,thread,nb)                             
{                                                   /* For the gray band absorption coefficient as a function of temperature, used with a non-gray discrete ordinates model. */
	double abs_coeff = 0.0;
	switch (nb)
	{
		case 0 : abs_coeff = interp(temperature,abs_coeff_1,100,C_T(cell, thread)); break;  /* Based on the T to find the absorption coefficient, for non-gray band. */
		case 1 : abs_coeff = interp(temperature,abs_coeff_2,100,C_T(cell, thread)); break;  
		case 2 : abs_coeff = interp(temperature,abs_coeff_3,100,C_T(cell, thread)); break;  
		case 3 : abs_coeff = interp(temperature,abs_coeff_4,100,C_T(cell, thread)); break;
		case 4 : abs_coeff = interp(temperature,abs_coeff_5,100,C_T(cell, thread)); break;
		case 5 : abs_coeff = interp(temperature,abs_coeff_6,100,C_T(cell, thread)); break;
	}
	return abs_coeff;
}

DEFINE_GRAY_BAND_ABS_COEFF(gray_band_abs_GEtable,cell,thread,nb)
{  
	return InterpolateAbsorptionCoefficient(C_T(cell, thread), C_P(cell, thread), C_YI(cell,thread,I_Cu),C_YI(cell,thread,I_Air),C_YI(cell,thread,I_Pa66),nb);
}

DEFINE_SR_RATE(SR_cu_pa66,f,tf,r,mw,yi,rr)                                
{
    #if !RP_HOST	
    if (STREQ(r->name, "pa66-reaction"))
    {
      F_UDMI(f,tf,UDM_SR_rate) = F_UDMI(f,tf,UDM_Pa66_rate_F)/mw[0];    /* unit of SR_rate is kmol/m2-s for the surface reactions  help/flu_udf/flu_udf_sec_define_net_reaction_rate.html*/
      *rr =  0.0;                                                       /* F_UDMI(f,tf,UDM_SR_rate); 	*/
    }
    else if(STREQ(r->name, "cu-reaction"))
    {
	  F_UDMI(f,tf,UDM_SR_rate) = F_UDMI(f,tf,UDM_Qvapcu_F)/h_cu/mw[1];	 
      *rr = F_UDMI(f,tf,UDM_SR_rate);    	
    }
    else
    {
      F_UDMI(f,tf,UDM_SR_rate) = 0.0;
      *rr = 0.0;	
    }
    #endif 
}

DEFINE_DOM_DIFFUSE_REFLECTIVITY(Do_Semitransport_Wall_Diffuse,t,nband,n_a,n_b,diff_ref_a,diff_tran_a,diff_ref_b,diff_tran_b)
{
    /*Message("This is DEFINE_DOM_DIFFUSE_REFLECTIVITY. \n"); */
	/*if (THREAD_ID(tf)==Ablative_wall_air  )
	{ } */ 
	*diff_ref_a=0.0;
	*diff_tran_a=1.0;
	if (N_TIME<=6)                         /* N_TIME is the interger number of time steps. CURRENT_TIME<=1e-5 */
	{
	Message("Refractive index of medium a=%f, the band num=%d \n", n_a, nband);
    Message("diff_ref_a=%f diff_tran_a=%f \n", *diff_ref_a, *diff_tran_a);
    Message("Refractive index of medium b=%f, the band num=%d \n", n_b, nband);  
    Message("diff_ref_b=%f diff_tran_b=%f \n", *diff_ref_b, *diff_tran_b);
	}
} 

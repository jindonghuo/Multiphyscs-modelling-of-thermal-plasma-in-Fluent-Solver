import subprocess
import sys
import os
import time
import datetime
import fileinput
import shutil
start_time = time.time()

NameSuffix = str(input('\n Please input the name suffix for fluent job: '))
paralle_cores  = str(input('\n Please input the number of cores: '))
if sys.platform.startswith('win'):
   print('\n Python will run on Windows system!')
   GUI = input('\n Would you like to run with GUI?(Y/N): ')

# created by Jindong, huojindong@gmail.com
initdirectory = os.getcwd()
originalfiledir = os.path.join(os.getcwd(), 'src_resources')
meshfile    = os.path.join(originalfiledir, 'geometry_mesh.cas')
udf_c_code1 = 'main_udf.c'                                           
udf_c_code2 = 'functionfile1.c'
udf_h_code1 = 'headfile1.h'
#py_code1    = 'batch_video.py'
py_code2    = 'video_from_image.py'
ge_table1   = 'pa66_cu_air_table_v5.txt'
ge_table2   = 'Cu_Avearge_Abs_coeff_P1_model_for_UDF.txt'
ge_table3   = 'AirAbsCoeffP1.txt'
journal     = r'TUIjournal.jou'

if not os.path.isabs(originalfiledir):
    print ('\n This is not a absolute work directory:' + originalfiledir + '. Please check it. \n')
    input ('\n Press anykey to exit...')
    sys.exit()    
if not os.path.exists(meshfile):
    print ('\n There is no meshfile in original directory, please check it.                    \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(initdirectory, udf_c_code1)):
    print ('\n There is no ' + udf_c_code1 + ' file in original directory, please check it.    \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, udf_c_code2)):
    print ('\n There is no ' + udf_c_code2 + ' file in original directory, please check it.    \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, udf_h_code1)):
    print ('\n There is no ' + udf_h_code1 + ' file in original directory, please check it.    \n')
    input('\n Press anykey to exit...')
    sys.exit()
#if not os.path.exists(os.path.join(originalfiledir, py_code1)):
#    print ('\n There is no ' + py_code1 + ' file in original directory, please check it.       \n')
#    input('\n Press anykey to exit...')
#    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, py_code2)):
    print ('\n There is no ' + py_code2 + ' file in original directory, please check it.       \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, ge_table1)):
    print ('\n There is no ' + ge_table1 + ' file in original directory, please check it.      \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, ge_table2)):
    print ('\n There is no ' + ge_table2 + ' file in original directory, please check it.      \n')
    input('\n Press anykey to exit...')
    sys.exit()
if not os.path.exists(os.path.join(originalfiledir, ge_table3)):
    print ('\n There is no ' + ge_table3 + ' file in original directory, please check it.      \n')
    input('\n Press anykey to exit...')
    sys.exit()

timestr = time.strftime("%Y%m%d_%H%M%S_")
computingfolder  ='e' + str(timestr) + NameSuffix
if not os.path.isdir(os.path.join(os.getcwd(), computingfolder)):
    os.mkdir(computingfolder)
os.chdir(os.path.join(os.getcwd(), computingfolder))             # change current directory to the subfolder

shutil.copy(os.path.join(initdirectory, udf_c_code1), os.getcwd())
shutil.copy(os.path.join(originalfiledir, udf_c_code2), os.getcwd())
shutil.copy(os.path.join(originalfiledir, udf_h_code1), os.getcwd())
shutil.copy(os.path.join(originalfiledir, py_code1), os.getcwd())
shutil.copy(os.path.join(originalfiledir, py_code2), os.getcwd())
shutil.copy(os.path.join(originalfiledir, ge_table1), os.getcwd())
shutil.copy(os.path.join(originalfiledir, ge_table2), os.getcwd())
shutil.copy(os.path.join(originalfiledir, ge_table3), os.getcwd())
time.sleep(0.2)         # leave some time for copy
       
casdat_path =  os.path.join(os.getcwd(), 'casdat')
if not os.path.isdir(casdat_path):  os.makedirs(casdat_path)
fig_path = os.path.join(os.getcwd(), 'figure')   
if not os.path.isdir(fig_path):  os.makedirs(fig_path)

###############################################       sbatch summit script      ######################################################### 
if sys.platform.startswith('linux'):
    print('\n Python will run on Linux system!')    
    sbatchfilename = 'submit_script.sh'
    partitions = ['SkylakePriority','debug','general','HaswellPriority','serial','general_requeue','parallel'] # to reduce abaqus interface 

    fb=open(sbatchfilename,'w')
    fb.write('#!/bin/bash \n')
    fb.write('#SBATCH --partition='+partitions[0]+'\n')
    fb.write('#SBATCH --ntasks='+str(int(paralle_cores)+0)+'\n')                #### +1 is for the host
    fb.write('#SBATCH --nodes=1\n')    
    fb.write('#SBATCH --exclusive\n')            
    fb.write('#SBATCH --job-name=' + NameSuffix +'\n')                
    fb.write('#SBATCH -o output_fluent_%J.dat\n')
    fb.write('#SBATCH -e output_fluent_%J.dat\n')
    fb.write('\n')
    fb.write('\n')
    fb.write('ulimit -s unlimited\n')
    fb.write('fluent 3ddp -t'+paralle_cores+' -gu -driver null -i '+journal+'  \n')    
    fb.close()    
    time.sleep(0.1)

############################################################################ Write journal ###################################################################### 
# Fluent is good at pressure-based solver
# follow GUI button sequence
f = open(journal,'w')  
#f.write('file read-case '+ mslcas   + '             \n')
f.write('file read-case '+ meshfile + '              \n')
f.write('mesh check                                  \n')
f.write('/mesh/repair-improve/report-poor-elements   \n')  # To show the low quality mesh which must be reqaired
f.write('/mesh/repair-improve/allow-repair-at-boundaries yes \n') # Repair-Improve is the simplest way to correct mesh problems identified by the mesh check
f.write('/mesh/repair-improve/repair                 \n')  # If no problems reported by mesh/check, this command is useless
f.write('/mesh/smooth-mesh "quality based" 4 0.1     \n')
f.write('mesh check                                  \n')
f.write('                                            \n')

#models
f.write('/define/set-unit-system SI                         \n')  # change the unit system to SI
f.write('/define/models/solver/pressure-based yes           \n') 
f.write('/define/models/unsteady-1st-order? yes             \n') 
#f.write('/define/models/unsteady-2nd-order-bounded? yes    \n')
#f.write('/define/models/solver/density-based-explicit yes  \n')  # write this after the 
#f.write('/define/models/energy? yes no yes yes yes         \n')
f.write('/define/models/energy? yes yes yes                 \n')  # Enable?  viscous energy dissipation?  diffusion at inlets? 
f.write('/define/models/viscous/ke-realizable? yes          \n')
f.write('/define/models/viscous/near-wall-treatment/enhanced-wall-treatment? yes          \n')
f.write('/define/models/viscous/user-defined "none" "none" "none" "none" "none"           \n')
f.write('                      \n')

#define the raditaion    
f.write('/define/models/radiation/discrete-ordinates? yes 2 2 3 3         \n')
f.write('/define/models/radiation/solar? no                               \n')
f.write('/define/models/radiation/radiation-model-parameters 5            \n')  # every 2 times update the radiation data. Because the rad PDF is independent.
f.write('/define/models/radiation/non-gray-model-parameters 6             \n')
f.write('"band-0" 0.010    0.085         \n')
f.write('"band-1" 0.085    0.102         \n')
f.write('"band-2" 0.102    0.113         \n')
f.write('"band-3" 0.113    0.180         \n')
f.write('"band-4" 0.180    1.200         \n')
f.write('"band-5" 1.200    4.500         \n')
f.write('                                \n')
#
f.write('/define/models/species/species-transport? yes mixture-template              \n')
f.write('/define/models/species/volumetric-reactions? yes                            \n')
f.write('/define/models/species/wall-surface-reactions? yes                          \n')
f.write('/define/models/species/heat-of-surface-reactions? no                        \n') # ignore the surface reaction heat source term, thus the enthalphy for cu and cu-solid don't matter.
f.write('/define/models/species/mass-deposition-source? yes                          \n')
f.write('/define/models/species/surf-reaction-aggressiveness-factor? 0.5             \n')    
f.write('/define/models/species/inlet-diffusion?  yes                                \n')
f.write('/define/models/species/diffusion-energy-source? yes                         \n')
#f.write('/define/models/dpm/numerics/tracking-parameters 500 no 5                   \n')
f.write('                                 \n')

##########################################################################################################################################################################################
# Add new material 
f.write('/define/materials/change-create air pa66       no no no no no no no no no no no no no no no        \n')
f.write('/define/materials/change-create air pa66-solid no no no no no no no no no no no no no no no        \n')
f.write('/define/materials/change-create air cu         no no no no no no no no no no no no no no no        \n')
f.write('/define/materials/change-create air cu-solid   no no no no no no no no no no no no no no no        \n')
f.write('                                \n')

# change the mixture gas.    Must defined before UDF compiled.
f.write('/define/materials/change-create \n')
f.write('mixture-template                \n')
f.write('mix-gas                         \n')
f.write('yes 3 pa66 cu air 2 pa66-solid cu-solid 0              \n')
f.write('yes 2 "pa66-reaction" yes no no 1 pa66-solid 1 1  1 pa66 1 0 1e12 0 0.5 no no 4 0.5 "cu-reaction" yes no no 1 cu-solid 1 1 1 cu 1 0 1e12 0 0.5 no no 4 0.5  \n')
f.write('yes "pa66-mechanism" wall-surface 1 "pa66-reaction" 0  \n')             # define the 1st mechansim
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('no                      \n')
f.write('yes                     \n')
f.write('                        \n')


# compile udf
f.write('/define/user-defined/compiled-functions compile "libudf" yes "'+udf_c_code1+'" "'+ udf_c_code2 +'" "" "'+ udf_h_code1 +'" ""     \n')
f.write('/define/user-defined/compiled-functions load "libudf"                                     \n')
f.write('                \n')

# execute-on-demand 
f.write('/define/user-defined/execute-on-demand "A_mixture_property_table::libudf"                 \n')
f.write('/define/user-defined/execute-on-demand "B_p1_Cu_absorption_coeff_table::libudf"           \n')
f.write('/define/user-defined/execute-on-demand "C_p1_Air_absorption_coeff_table::libudf"          \n')
f.write('                \n')

# udm and uds
f.write('/define/user-defined/user-defined-memory 33                                 \n')                                                                                  ############### UDM 
f.write('/define/user-defined/user-defined-scalars 4 yes yes no no yes "none" "none" no no yes "none" "none" no no yes "none" "none" no no yes "none" "none"        \n')   ############### UDS -distance-weighted diffusivity

# material property should be after UDF compiled
f.write('/define/materials/change-create    \n')
f.write('mix-gas                            \n')
f.write('mix-gas                            \n')
f.write('no                                 \n')
f.write('no                                 \n')
f.write('yes "cu-mechanism" wall-surface 1 "cu-reaction" 0       \n')                  # define the 2nd mechansim
f.write('yes ideal-gas                                           \n')  
f.write('yes mixing-law                                          \n')                  # f.write('yes constant 1006.43 \n') # Specific heat
f.write('yes user-defined "air_thermal_conductivity_GEtable::libudf"             \n')  # Thermal Conductivity
f.write('yes user-defined "air_visc_GEtable::libudf"                             \n')  # Viscosity
f.write('yes constant-dilute-appx 1 1.0e-02                                      \n')  # Mass diffusivity
f.write('yes user-defined-gray-band "gray_band_abs_air::libudf"                  \n')  # Absorption coefficient               gray_band_abs_GEtable    
f.write('yes constant 0.                                                         \n')  # Scattering coefficient
f.write('yes isotropic                                                           \n')  # Scattering phase function
f.write('yes constant 1.000293                                                   \n')  # Refractive index  for air              #### mixture Refractive index 
f.write('yes defined-per-uds 0 user-defined "Air_Econd::libudf" 1 constant 1. 2 constant 1. 3 constant 1. -1         \n')
f.write('no                    \n')
f.write('                      \n')

# Solid
f.write('/define/materials/copy solid copper copper \n')
f.write('/define/materials/change-create copper copper     no no no no no no no yes defined-per-uds 0 constant 5.9e7 1 constant 1. 2 constant 1. 3 constant 1. -1  \n') # change the condutivity.
f.write('/define/materials/change-create aluminum aluminum no no no no no no no yes defined-per-uds 0 constant 3.7e7 1 constant 1. 2 constant 1. 3 constant 1. -1  \n')
f.write('/define/materials/change-create aluminum airwall yes constant 1.29 yes constant 8   yes constant 1.5 yes constant 50 yes constant 0. yes isotropic yes constant 1.000293 yes defined-per-uds 0 constant 1e-09 -1 no \n')
f.write('/define/materials/change-create aluminum polymer yes constant 1000 yes constant 871 yes constant 1.5 yes constant 50 yes constant 0. yes isotropic yes constant 1.000293 yes defined-per-uds 0 constant 1e-09 -1 no \n')
f.write('                      \n')                                                                                     # 0.0001 absorption coefficient                   1.000293 refraction index

# Species properties update within the mixture
f.write('/define/materials/change-create pa66       mix-gas yes constant 1888.47 yes 18.1974 yes -109378202.3 no yes 300 no  \n')   # h=2.6e6 J/kg=5.824e8 J/kgmol. Intentional changed to 0.0, because no heat was abosrbed at the ablative wall
f.write('/define/materials/change-create pa66-solid mix-gas yes constant 1888.47 yes 18.1974 yes -109378202.3 no yes 300 no  \n')   # give the same enthalpy as the pa66
f.write('/define/materials/change-create cu         mix-gas yes constant 327.104 yes 63.546  yes 337436885.5  no yes 300 no  \n')   # h=5.31012e6 J/kg=3.374368855 e8 J/kgmol
f.write('/define/materials/change-create cu-solid   mix-gas yes constant 327.104 yes 63.546  yes 0            no yes 300 no  \n')   # Solid enthalpy is set to zero
f.write('/define/materials/change-create air        mix-gas yes constant 1009.92 yes 28.8914 yes 53980.98067  no yes 300 no  \n')   # If you are using the finite-rate model with reversible reactions, must define the standard state entropy. Otherwise no need
f.write('                      \n')

# specific heat
f.write('/define/materials/change-create mix-gas mix-gas     \n')
f.write('no no no no                                         \n')
f.write('yes user-defined "cp_GEtable::libudf"               \n')
f.write('no no no no no no no no no                          \n')
f.write('                                                    \n')

# Delete useless one
f.write('/define/materials/delete nitrogen          \n')
f.write('/define/materials/delete water-vapor       \n')
f.write('/define/materials/delete oxygen            \n')
f.write('                                           \n')
#
##################################################################################################################################################################################
# BCs Solid zone
f.write('/define/boundary-conditions/solid anode   \n')                                                                 # Anode 
f.write('yes copper                                \n') # change material
f.write('yes                                       \n') # activate source term
f.write('1 no yes "ES_solid_copper::libudf"        \n') # energy source
f.write('0                                         \n') # usd0
f.write('1 no yes "Ax_source::libudf"              \n') # usd1
f.write('1 no yes "Ay_source::libudf"              \n') # usd2
f.write('1 no yes "Az_source::libudf"              \n') # usd3
f.write('no                                        \n')  # Specify fixed values
f.write('no                                        \n')  # Frame Motion
f.write('no 0. no 0. no 0.                         \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                         \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                        \n') # Mesh Motion
f.write('no                                        \n') # Participates in radiation
f.write('no                                        \n') # Deactivated Thread
f.write('                                          \n') #

#
f.write('/define/boundary-conditions/solid cathode    \n')                                                                # Cathode        
f.write('yes copper                                   \n') # change material
f.write('yes                                          \n') # activate source term
f.write('1 no yes "ES_solid_copper::libudf"           \n') # energy source
f.write('0                                            \n') # usd0
f.write('1 no yes "Ax_source::libudf"                 \n') # usd1
f.write('1 no yes "Ay_source::libudf"                 \n') # usd2
f.write('1 no yes "Az_source::libudf"                 \n') # usd3
f.write('no                                           \n') # Specify fixed values
f.write('no                                           \n') # Frame Motion
f.write('no 0. no 0. no 0.                            \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                            \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                           \n') # Mesh Motion
f.write('no                                           \n') # Participates in radiation
f.write('no                                           \n') # Deactivated Thread

# Fluid Zone    
f.write('/define/boundary-conditions/fluid gas     \n')                                                                    # gas 
f.write('mixture yes mix-gas                       \n') # change material
f.write('yes                                       \n') # activate source term
f.write('0                                         \n') # mass source term
f.write('1 no yes "Lorentz_Fx::libudf"             \n') # X momentum source
f.write('1 no yes "Lorentz_Fy::libudf"             \n') # Y momentum source
f.write('1 no yes "Lorentz_Fz::libudf"             \n') # Z momentum source
f.write('0                                         \n') # Turbulent Kinetic Energy source
f.write('0                                         \n') # Turbulent Dissipation Rate source
f.write('0                                         \n') # pa66 sources
f.write('0                                         \n') # cu sources
f.write('1 no yes "ES_Joule_heating::libudf"       \n') # Energy Source
f.write('0                                         \n') # usd0
f.write('1 no yes "Ax_source::libudf"              \n') # usd1
f.write('1 no yes "Ay_source::libudf"              \n') # usd2
f.write('1 no yes "Az_source::libudf"              \n') # usd3
f.write('no                                        \n') # Specify fixed values
f.write('no                                        \n') # Frame Motion
f.write('no 0. no 0. no 0.                         \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                         \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                        \n') # Mesh Motion
f.write('yes                                       \n') # Participates in radiation
f.write('no                                        \n') # Deactivated Thread
f.write('no                                        \n') # LES zone
f.write('no                                        \n') # Laminar zone
f.write('no                                        \n') # Porous zone
f.write('no                                        \n') # 3D Fan Zone
f.write('no yes                                    \n') # Reaction Mechanism
f.write('no                                        \n') # Activate reaction mechanisms
f.write('0.                                        \n') # Surface-Volume-Ratio
f.write('                                          \n')
#
f.write('/define/boundary-conditions/fluid anode_ionlay     \n')                                                           # anode_ionlayer
f.write('mixture yes mix-gas                       \n') # change material
f.write('yes                                       \n') # activate source term
f.write('0                                         \n') # mass source term
f.write('1 no yes "Lorentz_Fx::libudf"             \n') # X momentum source
f.write('1 no yes "Lorentz_Fy::libudf"             \n') # Y momentum source
f.write('1 no yes "Lorentz_Fz::libudf"             \n') # Z momentum source
f.write('0                                         \n') # Turbulent Kinetic Energy source
f.write('0                                         \n') # Turbulent Dissipation Rate source
f.write('0                                         \n') # pa66 sources
f.write('0                                         \n') # cu sources
f.write('1 no yes "ES_Joule_heating::libudf"       \n') # Energy Source
f.write('0                                         \n') # usd0
f.write('1 no yes "Ax_source::libudf"              \n') # usd1
f.write('1 no yes "Ay_source::libudf"              \n') # usd2
f.write('1 no yes "Az_source::libudf"              \n') # usd3
f.write('no                                        \n') # Specify fixed values
f.write('no                                        \n') # Frame Motion
f.write('no 0. no 0. no 0.                         \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                         \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                        \n') # Mesh Motion
f.write('yes                                       \n') # Participates in radiation
f.write('no                                        \n') # Deactivated Thread
f.write('no                                        \n') # LES zone
f.write('no                                        \n') # Laminar zone
f.write('no                                        \n') # Porous zone
f.write('no                                        \n') # 3D Fan Zone
f.write('no yes                                    \n') # Reaction Mechanism
f.write('no                                        \n') # Activate reaction mechanisms
f.write('0.                                        \n') # Surface-Volume-Ratio
f.write('                                          \n')
#
f.write('/define/boundary-conditions/fluid cathode_ionlay     \n')                                                           # cathode_ionlay 
f.write('mixture yes mix-gas                       \n') # change material
f.write('yes                                       \n') # activate source term
f.write('0                                         \n') # mass source term
f.write('1 no yes "Lorentz_Fx::libudf"             \n') # X momentum source
f.write('1 no yes "Lorentz_Fy::libudf"             \n') # Y momentum source
f.write('1 no yes "Lorentz_Fz::libudf"             \n') # Z momentum source
f.write('0                                         \n') # Turbulent Kinetic Energy source
f.write('0                                         \n') # Turbulent Dissipation Rate source
f.write('0                                         \n') # pa66 sources
f.write('0                                         \n') # cu sources
f.write('1 no yes "ES_Joule_heating::libudf"       \n') # Energy Source
f.write('0                                         \n') # usd0
f.write('1 no yes "Ax_source::libudf"              \n') # usd1
f.write('1 no yes "Ay_source::libudf"              \n') # usd2
f.write('1 no yes "Az_source::libudf"              \n') # usd3
f.write('no                                        \n') # Specify fixed values
f.write('no                                        \n') # Frame Motion
f.write('no 0. no 0. no 0.                         \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                         \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                        \n') # Mesh Motion
f.write('yes                                       \n') # Participates in radiation
f.write('no                                        \n') # Deactivated Thread
f.write('no                                        \n') # LES zone
f.write('no                                        \n') # Laminar zone
f.write('no                                        \n') # Porous zone
f.write('no                                        \n') # 3D Fan Zone
f.write('no yes                                    \n') # Reaction Mechanism
f.write('no                                        \n') # Activate reaction mechanisms
f.write('0.                                        \n') # Surface-Volume-Ratio
f.write('                                          \n')
#
f.write('/define/boundary-conditions/fluid anode_drop   \n')                                                           # anode_drop 
f.write('mixture yes mix-gas                            \n') # change material
f.write('yes                                            \n') # activate source term
f.write('0                                              \n')  # mass source term
f.write('1 no yes "Lorentz_Fx::libudf"                  \n') # X momentum source
f.write('1 no yes "Lorentz_Fy::libudf"                  \n') # Y momentum source
f.write('1 no yes "Lorentz_Fz::libudf"                  \n') # Z momentum source
f.write('0                                              \n') # Turbulent Kinetic Energy source
f.write('0                                              \n') # Turbulent Dissipation Rate source
f.write('0                                              \n') # pa66 sources
f.write('0                                              \n') # cu sources
f.write('1 no yes "ES_anode_drop::libudf"               \n') # Energy Source
f.write('0                                              \n')  # usd0
f.write('1 no yes "Ax_source::libudf"                   \n')  # usd1
f.write('1 no yes "Ay_source::libudf"                   \n')  # usd2
f.write('1 no yes "Az_source::libudf"                   \n')  # usd3
f.write('no                                             \n')  # Specify fixed values
f.write('no                                             \n')  # Frame Motion
f.write('no 0. no 0. no 0.                              \n')  # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                              \n')  # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                             \n')  # Mesh Motion
f.write('yes                                            \n')  # Participates in radiation
f.write('no                                             \n')  # Deactivated Thread
f.write('no                                             \n') # LES zone
f.write('no                                             \n') # Laminar zone
f.write('no                                             \n') # Porous zone
f.write('no                                             \n') # 3D Fan Zone
f.write('no yes                                         \n') # Reaction Mechanism
f.write('no                                             \n') # Activate reaction mechanisms
f.write('0.                                             \n') # Surface-Volume-Ratio
f.write('                                               \n')

#
f.write('/define/boundary-conditions/fluid cathode_drop  \n')                                                         # cathode_drop 
f.write('mixture yes mix-gas                             \n') # change material
f.write('yes                                             \n') # activate source term
f.write('0                                               \n')  # mass source term
f.write('1 no yes "Lorentz_Fx::libudf"                   \n') # X momentum source
f.write('1 no yes "Lorentz_Fy::libudf"                   \n') # Y momentum source
f.write('1 no yes "Lorentz_Fz::libudf"                   \n') # Z momentum source
f.write('0                                               \n') # Turbulent Kinetic Energy source
f.write('0                                               \n') # Turbulent Dissipation Rate source
f.write('0                                               \n') # pa66 sources
f.write('0                                               \n') # cu sources
f.write('1 no yes "ES_cathode_drop::libudf"              \n') # Energy Source
f.write('0                                               \n') # usd0
f.write('1 no yes "Ax_source::libudf"                    \n') # usd1
f.write('1 no yes "Ay_source::libudf"                    \n') # usd2
f.write('1 no yes "Az_source::libudf"                    \n') # usd3
f.write('no                                              \n')  # Specify fixed values
f.write('no                                              \n')  # Frame Motion
f.write('no 0. no 0. no 0.                               \n') # Reference Frame X Y Z-Origin
f.write('no 0. no 0. no 1.                               \n') # Reference Frame X Y Z-Component of Rotation-Axis
f.write('no                                              \n') # Mesh Motion
f.write('yes                                             \n') # Participates in radiation
f.write('no                                              \n') # Deactivated Thread
f.write('no                                              \n') # LES zone
f.write('no                                              \n') # Laminar zone
f.write('no                                              \n') # Porous zone
f.write('no                                              \n') # 3D Fan Zone
f.write('no yes                                          \n') # Reaction Mechanism
f.write('no                                              \n') # Activate reaction mechanisms
f.write('0.                                              \n') # Surface-Volume-Ratio
f.write('                                                \n') #

#
#f.write('/define/operating-conditions/gravity no                                \n')
f.write('/define/operating-conditions/gravity yes 0.0 -20.0 0.0                  \n')
f.write('/define/operating-conditions/operating-density? yes 1.23                \n')  # warning: for compressible (ideal and real) gas models with buoyancy, it is recommended to use a specified operating density value of zero.
f.write('/define/operating-conditions/operating-pressure 101325.                 \n')
#
f.write('/define/operating-conditions/reference-pressure-location 0. 0. 0.       \n')  # If pressure boundaries are involved, this will be ignored
f.write('/define/boundary-conditions/bc-settings/pressure-outlet 0. 65           \n')  # Pressure blending factor and  Number of bins
f.write('                \n')
# SET reference & Batch option    
f.write('/report/reference-values/zone gas                                       \n')
f.write('/file/set-batch-options yes no no                                       \n')
f.write('                \n')

##########################################################################################################################################################################################################
# FG0: inlet wall      
f.write('/define/boundary-conditions/zone-type top_bd                   pressure-outlet  \n')
f.write('/define/boundary-conditions/pressure-outlet top_bd                              \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')

#
#f.write('/define/boundary-conditions/zone-type rear_bd                  pressure-outlet  \n')
#f.write('/define/boundary-conditions/pressure-outlet rear_bd                             \n')
#f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('/define/boundary-conditions/wall rear_bd                          \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')

'''
f.write('/define/boundary-conditions/wall left_bd-cathode_drop             \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')
f.write('/define/boundary-conditions/wall left_bd-cathode_ionlay           \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')
f.write('/define/boundary-conditions/wall left_bd-gas                      \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')

f.write('/define/boundary-conditions/wall right_bd-anode_drop              \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')
f.write('/define/boundary-conditions/wall right_bd-anode_ionlay            \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')
f.write('/define/boundary-conditions/wall right_bd-gas                     \n')
f.write('0.001 no 0. yes airwall yes mixed no 100 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')
'''

f.write('/define/boundary-conditions/zone-type left_bd-cathode_drop       pressure-outlet  \n')
f.write('/define/boundary-conditions/zone-type left_bd-cathode_ionlay     pressure-outlet  \n')
f.write('/define/boundary-conditions/zone-type left_bd-gas                pressure-outlet  \n')
f.write('/define/boundary-conditions/pressure-outlet left_bd-cathode_drop                  \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('/define/boundary-conditions/pressure-outlet left_bd-cathode_ionlay                \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('/define/boundary-conditions/pressure-outlet left_bd-gas                           \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('                                  \n')

#
f.write('/define/boundary-conditions/zone-type right_bd-anode_drop    pressure-outlet  \n') 
f.write('/define/boundary-conditions/zone-type right_bd-anode_ionlay  pressure-outlet  \n')
f.write('/define/boundary-conditions/zone-type right_bd-gas           pressure-outlet  \n')
f.write('/define/boundary-conditions/pressure-outlet right_bd-anode_drop           \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('/define/boundary-conditions/pressure-outlet right_bd-anode_ionlay         \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('/define/boundary-conditions/pressure-outlet right_bd-gas                  \n')
f.write('yes no 0. no 300. no yes no no yes 0.05 10. yes no 1. no no 0. no 0. yes yes yes yes no 0. no 0. no 0. no 0. yes no yes no no no    \n')
f.write('                \n')

# Bottom Wall
f.write('/define/boundary-conditions/wall bottom_bd                               \n')
f.write('0.001 no 0. yes airwall yes mixed no 0.0 no 300. no no no no 1 no 300. no 1. no 1. no 1. no 1. no 1. no 1. yes semi-transparent no 1. no 0. no 0. 1.744e-08 1.744e-08 no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. no 0. 1. 1. 1. 1. 1. 1. yes no no no yes yes yes yes yes yes no 0. no 0. no 0. no 0. no 1.  \n')

# FG5: Current in and out (copper, wall thickness 0.002 m), voltage is zero for curr_out    
f.write('/define/boundary-conditions/wall current_in    \n') # change the in and out in order to match the experiment
f.write('0.002 no 0. yes copper yes mixed no 15. no 300. no no 1. no 300. no 1. no 1. no 1. no 1. no 1. no 1. no 1. 1. 1. 1. 1. 1. no no yes yes yes yes yes yes "udf" "Current_input::libudf" no 0. no 0. no 0. no 1.  \n')
f.write('/define/boundary-conditions/wall current_out     \n')
f.write('0.002 no 0. yes copper yes mixed no 15. no 300. no no 1. no 300. no 1. no 1. no 1. no 1. no 1. no 1. no 1. 1. 1. 1. 1. 1. no no no yes yes yes yes no 0. no 0. no 0. no 0. no 1.                               \n')
f.write('                \n')

# FG6: wall of electrode and splitter
f.write('/define/boundary-conditions/wall anodewall      \n')
f.write('0.001 no 0. yes copper yes mixed no 15. no 300. no no 1. no 300. no 1. no 1. no 1. no 1. no 1. no 1. no 1 1 1 1 1 1 no no yes yes yes yes no 0. no 0. no 0. no 0. no 1.    \n')
f.write('/define/boundary-conditions/wall cathodewall    \n')
f.write('0.001 no 0. yes copper yes mixed no 15. no 300. no no 1. no 300. no 1. no 1. no 1. no 1. no 1. no 1. no 1 1 1 1 1 1 no no yes yes yes yes no 0. no 0. no 0. no 0. no 1.    \n')
f.write('                \n')

#FG7: shealth layer wall with Cu SR (gas side)  coupled +-no
f.write('/define/boundary-conditions/wall  wall-anode-anode_drop-shadow              \n')
f.write('0. no 0. yes copper yes coupled no no no no 1. no 1. no 1. no 1. no 1. no 1. no 1. 1. 1. 1. 1. 1. no no yes no no yes no no yes no no yes no no yes no no yes 1. no 1.   \n')
f.write('/define/boundary-conditions/wall  wall-cathode-cathode_drop                 \n')
f.write('0. no 0. yes copper yes coupled no no no no 1. no 1. no 1. no 1. no 1. no 1. no 1. 1. 1. 1. 1. 1. no no yes no no yes no no yes no no yes no no yes no no yes 1. no 1.   \n')
f.write('                \n')

# shadow of FG7 (solid side)   
f.write('/define/boundary-conditions/wall wall-anode-anode_drop                      \n')
f.write('0. no 0. yes copper yes coupled no no 1 no 1 no 1 no 1 no 1 no 1 no 1 1 1 1 1 1 no no no no yes no no yes no no yes no no yes no 1  \n')
f.write('/define/boundary-conditions/wall wall-cathode-cathode_drop-shadow           \n')
f.write('0. no 0. yes copper yes coupled no no 1 no 1 no 1 no 1 no 1 no 1 no 1 1 1 1 1 1 no no no no yes no no yes no no yes no no yes no 1  \n')
f.write('                \n')

#########################################################################################################################################################################################
# UDF HOOKS
f.write('/define/user-defined/function-hooks/initialization         \n')
f.write('"A_init_TempPressure::libudf"                              \n')
f.write('"B_cu_pa66_species_initialize::libudf"                     \n')
f.write('""              \n')
f.write('                \n')

f.write('/define/user-defined/function-hooks/adjust                 \n')
f.write('"A_uds_number_check::libudf"                               \n')
f.write('"B_Maxwell_Equation::libudf"                               \n')
f.write('"C_ArcRoot_Energy::libudf"                                 \n')
#f.write('"D_Wall_ablation::libudf"                                 \n')
f.write('""              \n')
f.write('                \n')

f.write('/define/user-defined/function-hooks/execute-at-end         \n')
f.write('"A_Sigma_update::libudf"                                   \n')
f.write('"B_mole_fraction::libudf"                                  \n')
f.write('""              \n')
f.write('                \n')

f.write('/define/user-defined/function-hooks/execute-at-exit        \n')
f.write('"unload_fluentdata::libudf"                                \n')
f.write('""                                                         \n')
f.write('                \n') 

f.write('/define/user-defined/function-hooks/surface-reaction-rate  \n')
f.write('"SR_cu_pa66::libudf"                                       \n')
f.write('                \n')
#f.write('/define/user-defined/function-hooks/do-diffuse-reflectivity \n')
#f.write('"Do_Semitransport_Wall_Diffuse::libudf"                     \n')
f.write('                \n')

####################################################################### solver ########################################################################################################
#Solution Methods
f.write('/solve/set/p-v-coupling 24                           \n')
f.write('/solve/set/gradient-scheme yes                       \n') 
f.write('/solve/set/discretization-scheme/density 1           \n')  
f.write('/solve/set/discretization-scheme/disco 1             \n')   # changed to second order upwind
f.write('/solve/set/discretization-scheme/epsilon 0           \n')
f.write('/solve/set/discretization-scheme/k 0                 \n')
f.write('/solve/set/discretization-scheme/mom 1               \n')
f.write('/solve/set/discretization-scheme/pressure 14         \n')   # If you are using the pressure-based solver, selecting PRESTO! for Pressure under Spatial Discretization is recommended.
f.write('/solve/set/discretization-scheme/species-0 1         \n')
f.write('/solve/set/discretization-scheme/species-1 1         \n')
f.write('/solve/set/discretization-scheme/temperature 1       \n')
f.write('/solve/set/discretization-scheme/uds-0 2             \n')
f.write('/solve/set/discretization-scheme/uds-1 2             \n')
f.write('/solve/set/discretization-scheme/uds-2 2             \n')
f.write('/solve/set/discretization-scheme/uds-3 2             \n')
f.write('              \n')


#set general parameters
f.write('/solve/set/numerics no yes no no no 1.                          \n')
f.write('/solve/set/slope-limiter-set/ 0 yes yes                         \n')           # f.write('/solve/set/slope-limiter-set/ 0 yes no                         \n')
f.write('/solve/set/poor-mesh-numerics/enable? yes 1                     \n')
f.write('/solve/set/stiff-chemistry no                                   \n')
f.write('/solve/set/warped-face-gradient-correction/enable? yes yes      \n')
f.write('/solve/set/expert no no no no                                   \n')
#f.write('/solve/set/high-order-term-relaxation/enable? yes              \n')
#f.write('/solve/set/high-order-term-relaxation/options/relaxation-factor 0.75  \n')
#f.write('/solve/set/high-order-term-relaxation/options/variables/select 2      \n')    # 2 all variable, 1 only flow variable
f.write("(rpsetvar 'amg/protective-enhanced? #t)                                \n")


#Solution Controls  
f.write('/solve/set/p-v-controls 200 0.75 0.75               \n')
f.write('/solve/set/under-relaxation/density 0.95            \n')
f.write('/solve/set/under-relaxation/body-force 0.95         \n')
f.write('/solve/set/under-relaxation/k 0.6                   \n')
f.write('/solve/set/under-relaxation/epsilon 0.6             \n')
f.write('/solve/set/under-relaxation/turb-viscosity 0.6      \n')
f.write('/solve/set/under-relaxation/species-0 0.95          \n')
f.write('/solve/set/under-relaxation/species-1 0.95          \n')
f.write('/solve/set/under-relaxation/temperature 0.95        \n')
f.write('/solve/set/under-relaxation/disco 0.98              \n')
f.write('/solve/set/under-relaxation/uds-0 1.0               \n')  # the usd0 relaxation factor must be 1
f.write('/solve/set/under-relaxation/uds-1 1.0               \n')
f.write('/solve/set/under-relaxation/uds-2 1.0               \n')
f.write('/solve/set/under-relaxation/uds-3 1.0               \n')  
f.write('/solve/set/reactions? yes 0.99                      \n') 

# Equations    
f.write('/solve/set/equations/uds-0 yes            \n')
f.write('/solve/set/equations/uds-1 yes            \n')
f.write('/solve/set/equations/uds-2 yes            \n')
f.write('/solve/set/equations/uds-3 yes            \n')
f.write('/solve/set/equations/disco no             \n')
f.write('/solve/set/equations/species-0 no         \n')
f.write('/solve/set/equations/species-1 no         \n')
f.write('/solve/set/equations/flow no              \n')
f.write('/solve/set/equations/temperature no       \n')
f.write('/solve/set/equations/ke no                \n') 
f.write('/solve/set/limits 1 5e10 1 50000 1e-14 1e-20 100000  \n') #set limits for variables  
f.write('/solve/monitors/residual/criterion-type 0            \n')
f.write('/solve/monitors/residual/convergence-criteria 1e-16 1e-4 1e-4 1e-4 \n')
f.write('                \n')

#f.write('/solve/set/multi-grid-controls/temperature      3 1e-03 1   \n')                          # do not use W cycle in parallel 
f.write('/solve/set/multi-grid-controls/uds-0             3 1e-03 1   \n')                          # 1 is selective, 0 is aggregative
f.write('/solve/set/multi-grid-controls/uds-1             0 0.1 0.7   \n')
f.write('/solve/set/multi-grid-controls/uds-2             0 0.1 0.7   \n')
f.write('/solve/set/multi-grid-controls/uds-3             0 0.1 0.7   \n')
#f.write('/solve/set/multi-grid-amg 40 8 1 3 30 "ilu" 40 8 1 3 30 "ilu" 90 150 0 \n')      # increasing the pre-relaxation sweep value to 1 or 2 can improve convergence
f.write('/solve/set/multi-grid-amg 40 2 0 1 30 "gauss-seidel" 40 8 0 3 10 "ilu" 30 50 0  \n')  
#f.write('/solve/set/amg-options/conservative-amg-coarsening? yes               \n')               # choose yes. improves convergence for difficult problems by tuning multigrid coarsening based on coefficient strengths and, in parallel computations, the partitioning   
f.write('/solve/set/amg-options/aggressive-amg-coarsening? yes yes              \n') 
f.write('                 \n') 
# Note that for the parallel solver, efficiency may deteriorate with a W cycle. If you are using the parallel solver, try increasing the maximum number of cycles by increasing the value of Max Cycles in the Multigrid Controls panel under Fixed Cycle Parameters.


###################################################################################################  report definition  ###############################################################################################################
# max_temperature
f.write('/solve/report-definitions/add max_temp volume-max    \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps                         
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay () \n')
f.write('field temperature            \n')
f.write('quit                         \n')

# max_voltage 
f.write('/solve/report-definitions/add max_voltage volume-max      \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names anode cathode ()  \n')
f.write('field uds-0-scalar           \n')
f.write('quit                         \n')

# min_voltage 
f.write('/solve/report-definitions/add min_voltage volume-min     \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  . 1 means the current value
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names anode cathode ()  \n')
f.write('field uds-0-scalar           \n')
f.write('quit                         \n')

# max_pressure
f.write('/solve/report-definitions/add max_pressure volume-max    \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  . 1 means the current value
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()                 \n')
f.write('field pressure               \n')
f.write('quit                         \n')

# max_currdensity
f.write('/solve/report-definitions/add max_currd volume-max                                    \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  . 1 means the current value
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names anode cathode gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()   \n')
f.write('field udm-7                  \n')
f.write('quit                         \n')

# max_lorentzFy
f.write('/solve/report-definitions/add max_lorentzFy volume-max                          \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  . 1 means the current value
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()           \n')
f.write('field udm-13                 \n')
f.write('quit                         \n')

# avg_lorentzFy  
f.write('/solve/report-definitions/add avg_lorentzFy volume-average                      \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps. 1 means the current value
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()           \n')
f.write('field udm-13                 \n')
f.write('quit                         \n')

# Fmag 
f.write('/solve/report-definitions/add max_PinchForce volume-max                         \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  
f.write('per-zone? no                 \n')  #whether or not the chosen variable is calculated from all cell zones combined (default) or individually on each cell zones
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()           \n')
f.write('field udm-15                 \n')
f.write('quit                         \n')

# cu_mass_rate
f.write('/solve/report-definitions/add max_cu_concen volume-max                         \n')
f.write('average-over 1               \n')  #The value reported is averaged over the last 1 iterations/time steps  
f.write('per-zone? no                 \n')
f.write('zone-names gas anode_drop anode_ionlay cathode_drop cathode_ionlay ()           \n')                                          #----------------------Geometry
f.write('field molef-cu               \n')
f.write('quit                         \n')
f.write('                             \n')

###################################################################################################  report output  #######################################################################################################################################
# Temperature
f.write('/solve/report-files/add temperature_max \n')
f.write('active? yes                             \n')
f.write('file-name "'+ os.path.join(casdat_path, 'temp_max.txt') + '" \n')
f.write('report-defs flow-time max_temp ()       \n')  
f.write('frequency 1                             \n') # every 1 time step
f.write('frequency-of time-step                  \n') #time-step iteration
f.write('itr-index "1"                           \n') #Full File Name is created with a run index and iteration/time step index, appended to the base name
f.write('run-index "0"                           \n')
f.write('print? no                               \n')
f.write('quit                                    \n')
                                                              
# voltage min and max 
f.write('/solve/report-files/add voltage         \n')
f.write('active? yes                             \n')
f.write('file-name "'+ os.path.join(casdat_path, 'voltage_min_max.txt') + '" \n')  
f.write('report-defs flow-time min_voltage max_voltage ()    \n')
f.write('frequency 1                             \n')
f.write('frequency-of time-step                  \n') #time-step iteration
f.write('itr-index "1"                           \n') #Full File Name is created with a run index and iteration/time step index, appended to the base name.
f.write('run-index "0"                           \n')
f.write('print? no                               \n')
f.write('quit                                    \n')

# max_currdensity
f.write('/solve/report-files/add currentdensity  \n')
f.write('active? yes                             \n')
f.write('file-name "'+ os.path.join(casdat_path, 'currdensity_max.txt') + '" \n')  
f.write('report-defs flow-time max_currd ()      \n')
f.write('frequency 1                             \n')
f.write('frequency-of time-step                  \n') #time-step iteration
f.write('itr-index "1"                           \n')
f.write('run-index "0"                           \n')
f.write('print? no                               \n')
f.write('quit                                    \n')

# LorentzY avg and max
f.write('/solve/report-files/add LorentzY        \n')
f.write('active? yes                             \n')
f.write('file-name "'+ os.path.join(casdat_path, 'lorentzY_avg_max_pinch.txt') + '" \n')  
f.write('report-defs flow-time avg_lorentzFy max_lorentzFy max_PinchForce ()    \n')
f.write('itr-index "1"                           \n')
f.write('frequency 1                             \n')
f.write('frequency-of time-step                  \n') #time-step iteration
f.write('run-index "0"                           \n')
f.write('print? no                               \n')
f.write('quit                                    \n')

# Species pa66 and Cu
f.write('/solve/report-files/add copper_mrate    \n')
f.write('active? yes                             \n')
f.write('file-name "'+ os.path.join(casdat_path, 'species_cu_concentration.txt') + '" \n')  
f.write('report-defs flow-time max_cu_concen ()       \n')         # unit is kg/s at different time
f.write('itr-index "1"                           \n')
f.write('frequency 1                             \n')
f.write('frequency-of time-step                  \n')         # time-step iteration
f.write('run-index "0"                           \n')
f.write('print? no                               \n')
f.write('quit                                    \n')
f.write('                                        \n')

################################################################################################### 
# AMG Setting
if int(paralle_cores)>=2: 
    #
    f.write('/parallel/partition/method metis '+ paralle_cores +'         \n')   # Must place in first.       Metis    
    #f.write('/parallel/partition/set/across-zones yes                    \n')   # Fluent 19 not available, because, Metis method must across the zone
    f.write('/parallel/partition/set/laplace-smoothing yes 3.6            \n')   #   Laplace Smoothing used to prevent partition through areas of higher aspect ratio cells.
    f.write('/parallel/partition/set/smooth yes 0                         \n')   # Iterations set to 0, the optimization scheme will be applied until completion
    f.write('/parallel/partition/set/merge yes 0                          \n')   
    f.write('/parallel/partition/set/verbosity 2                          \n')   # the content for report
    f.write('/parallel/partition/method metis '+ paralle_cores +'         \n')   # Must place in first.     metis
    #f.write('/parallel/partition/reorder-partitions                      \n')   # Reverse Cuthill-Mckee    (for these two reorder type it is hard to say which one is better. Always, one is divergence, another is ok)
    f.write('/parallel/partition/reorder-partitions-to-architecture       \n')   # Architecture Aware.      default option and it accounts for the system architecture and network topology in remapping the partitions to the processors.
    ''' 
    f.write('/parallel/partition/method cartesian-x '+ paralle_cores +'   \n')   # principal-axes  cartesian-axes  cartesian-y cartesian-x
    f.write('/parallel/partition/set/across-zones yes                     \n')    
    f.write('/parallel/partition/set/smooth yes 0                         \n')      
    f.write('/parallel/partition/set/pre-test yes                         \n')   # only available for Principal Axes   
    f.write('/parallel/partition/set/merge yes 0                          \n')  
    f.write('/parallel/partition/set/solid-thread-weight yes yes 0.2      \n')   # doesn't work with metis          
    f.write('/parallel/partition/set/verbosity 2                          \n')  
    f.write('/parallel/partition/method cartesian-x '+ paralle_cores +'   \n')     
    #f.write('/parallel/partition/reorder-partitions                      \n')   # Reverse Cuthill-Mckee    
    f.write('/parallel/partition/reorder-partitions-to-architecture       \n')   # Architecture Aware       default 
    ''' 
    f.write('/parallel/partition/use-stored-partitions                    \n')
    f.write('            \n')

f.write('/solve/initialize/set-defaults/pressure 300.0           \n')
f.write('/solve/initialize/set-defaults/x-velocity 0.0           \n')
f.write('/solve/initialize/set-defaults/y-velocity 0.0           \n')
f.write('/solve/initialize/set-defaults/z-velocity 0.0           \n')
f.write('/solve/initialize/set-defaults/k 1.                     \n')
f.write('/solve/initialize/set-defaults/epsilon 1.               \n')
f.write('/solve/initialize/set-defaults/species-0 0.             \n')
f.write('/solve/initialize/set-defaults/species-1 0.             \n')
f.write('/solve/initialize/set-defaults/temperature 300.         \n')
f.write('/solve/initialize/set-defaults/uds-0 0.                 \n')
f.write('/solve/initialize/set-defaults/uds-1 0.                 \n')
f.write('/solve/initialize/set-defaults/uds-2 0.                 \n')
f.write('/solve/initialize/set-defaults/uds-3 0.                 \n')
f.write('/solve/initialize/init-flow-statistics                  \n')
f.write('/solve/initialize/initialize-flow ok                    \n')
f.write('             \n')

##################################################################################################################################################   Output Data
#Autosave 
f.write('/file/auto-save/save-data-file-every time-step                           \n')
f.write('/file/auto-save/case-frequency if-case-is-modified                       \n')
f.write('/file/auto-save/data-frequency 2299                                      \n')
f.write('/file/auto-save append-file-name-with time-step 6                        \n')
f.write('/file/auto-save/root-name "'+os.path.join(casdat_path, 'arc.gz')+ '"     \n')
#f.write('/define/solution-strategy/enable-strategy? no                 \n')                                                                # automatic initialization and case modification
f.write('             \n')
f.write('/file/transient-export/cgns '+os.path.join(casdat_path, 'jacres1 '))
f.write('x-velocity y-velocity z-velocity uds-0-scalar udm-0 udm-1 udm-2 udm-3 udm-4 udm-5 udm-6 udm-7 udm-8 udm-9 udm-10 udm-11 udm-12 udm-13 udm-14 udm-15 udm-16 udm-17 udm-18 udm-28 udm-29 udm-30 udm-31 molef-cu temperature quit no "rescgns" "time-step" 10 time-step \n')                       # do not need to specify the zone or surface names
f.write('             \n')

f.write('/file/transient-export/ascii '+os.path.join(casdat_path, 'XYZ_Temp_J ') + '() udm-7 temperature quit yes no "export_asc" "time-step" 20 time-step  \n') # () surfaces 1; "time-step"
f.write('             \n')

####################################################################################################################################################

f.write('/display/set-window 7                               \n')
f.write('/display/set/picture/landscape yes                  \n')
f.write('/display/set/picture/x-resolution 4400              \n')
f.write('/display/set/picture/y-resolution 5000              \n')
f.write('/display/set/picture/color-mode color               \n')
f.write('/display/mesh-partition-boundary                    \n')
f.write('/display/views/default-view                         \n')
f.write('/display/views/camera/zoom-camera 1.14              \n')
f.write('/display/views/camera/position 0.14 0.20 0.52       \n')         # camear point
f.write('/display/save-picture '+ os.path.join(fig_path, 'AMPI_partition1.png')  + '\n')
# set contour output
f.write('            \n')        
f.write('/display/set/windows/logo-color white          \n')
f.write('/display/set/picture/x-resolution 1700         \n')
f.write('/display/set/picture/y-resolution 2000         \n')
f.write('/display/set/picture/driver png                \n')
f.write('/display/set/contours/n-contour 80             \n')
f.write('/display/set/contours/coloring yes 80          \n')
f.write('/display/set/colors/skip-label 5               \n')              # how many notation text beside the legend bar
f.write('/display/set/contours/surfaces sym-anode sym-anode_drop sym-anode_ionlay sym-cathode sym-cathode_drop sym-cathode_ionlay sym-gas top_bd right_bd-anode_drop right_bd-anode_ionlay right_bd-gas anodewall current_in () \n')
f.write('/display/set/contours/filled-contours yes      \n')
f.write('/display/set/contours/global-range? no         \n')
f.write('/display/set/contours/auto-range? yes          \n')
f.write('/display/set/contours/node-values? yes         \n')
f.write('/display/set/contours/render-mesh? no          \n')
f.write('/display/set/contours/line-contours? no        \n')
f.write('/display/contour temperature _ _               \n')        # must plotting, then auto-scale, otherwise autoscale fails.
#f.write('/display/views/camera/position 0.14 0.20 0.52      \n')   # camear point
#
f.write('/display/set/windows/scale/visible? yes        \n') # the legned is visible or not, this command will reset to default, then must place at 1st 
f.write('/display/set/windows/scale/format "%0.1e"      \n') #
f.write('/display/set/windows/scale/alignment left      \n') # legend on the left side
f.write('/display/set/windows/scale/font-size 0.02      \n') # for the legend word
f.write('/display/set/windows/scale/left -1.7           \n') # the legend right border location
f.write('/display/set/windows/scale/right -0.6          \n') # the legend right border location (just the legend, not include the font)
f.write('/display/set/windows/scale/top 0.8             \n') #
f.write('/display/set/windows/scale/bottom -0.8         \n') #
f.write('/display/set/windows/scale/clear? yes          \n') # transparent
f.write('/display/set/windows/scale/margin 0.17         \n') # large margin gives narrow legend bar
#
f.write('/display/views/save-view figureoutput          \n')
f.write('            \n')

if int(paralle_cores)>=2:
    f.write('/display/contour cell-partition-active _ _   \n')
    f.write('/display/save-picture '+ os.path.join(fig_path, 'AMPI_partition2.png')  + '\n')

# set output temp current density 
f.write('/display/contour uds-0-scalar _ _      \n')
f.write('/display/save-picture '+ os.path.join(fig_path, 'voltage_%t.png')      +      '\n')
f.write('/display/contour temperature _ _       \n')
f.write('/display/save-picture '+ os.path.join(fig_path, 'temperature_%t.png')     +   '\n')
f.write('/display/contour udm-7 _ _             \n')
f.write('/display/save-picture '+ os.path.join(fig_path, 'currD_%t.png')    +          '\n')
f.write('/display/contour molef-cu _ _          \n')
f.write('/display/save-picture '+ os.path.join(fig_path, 'cuu-mol_%t.png')      +      '\n')
f.write('            \n')

# auto figure save  uds-0-scalar
f.write('/solve/execute-commands/add-edit command-0  1 "time-step" "/display/views/restore-view figureoutput"         \n')
f.write('/solve/execute-commands/add-edit command-1  1 "time-step" "/display/contour udm-7 _ _"                       \n')
f.write('/solve/execute-commands/add-edit command-2  1 "time-step" "/display/save-picture  '+ os.path.join(fig_path, 'currD_%t.png') + '"         \n')
f.write('/solve/execute-commands/add-edit command-3  4 "time-step" "/display/contour udm-13 _ _ "                 \n')
f.write('/solve/execute-commands/add-edit command-4  4 "time-step" "/display/save-picture  '+ os.path.join(fig_path, 'MagFy_%t.png') + '"         \n')
f.write('/solve/execute-commands/add-edit command-5  2 "time-step" "/display/contour uds-0-scalar _ _ "           \n')                                             # /display/contour molef-cu _ _ 
f.write('/solve/execute-commands/add-edit command-6  2 "time-step" "/display/save-picture  '+ os.path.join(fig_path, 'voltage_%t.png') + '"       \n')
f.write('/solve/execute-commands/add-edit command-7  1 "time-step" "/display/contour molef-cu _ _"                \n')
f.write('/solve/execute-commands/add-edit command-8  1 "time-step" "/display/save-picture  '+ os.path.join(fig_path, 'cuu-mol_%t.png') + '"       \n')          #  uds-0-scalar   voltage_%t.png
f.write('/solve/execute-commands/add-edit command-9  1 "time-step" "/display/contour temperature _ _"             \n')  # _ _ is auto_range.
f.write('/solve/execute-commands/add-edit command-10  1 "time-step" "/display/save-picture  '+ os.path.join(fig_path, 'temperature_%t.png') + '"  \n')
f.write('/solve/execute-commands/add-edit command-11  8 "time-step" "/display/contour pressure _ _  "             \n')
f.write('/solve/execute-commands/add-edit command-12  8 "time-step" "/display/save-picture '+ os.path.join(fig_path, 'P_%t.png') + '"             \n')
f.write('/solve/execute-commands/add-edit command-13  8 "time-step" "/display/contour density _ _  "             \n')
f.write('/solve/execute-commands/add-edit command-14  8 "time-step" "/display/save-picture '+ os.path.join(fig_path, 'Den_%t.png') + '"           \n')
f.write('                    \n')
#                                                                               ###################################################################### 1st
f.write('/solve/set/time-step 2e-6                  \n')  
f.write('/solve/dual-time-iterate 8 20              \n')  
# all eqations
f.write('/solve/set/equations/uds-0 yes             \n')
f.write('/solve/set/equations/uds-1 yes             \n')
f.write('/solve/set/equations/uds-2 yes             \n')
f.write('/solve/set/equations/uds-3 yes             \n')
f.write('/solve/set/equations/disco yes             \n')
f.write('/solve/set/equations/species-0 no          \n')
f.write('/solve/set/equations/species-1 yes         \n')
f.write('/solve/set/equations/flow yes              \n')
f.write('/solve/set/equations/temperature yes       \n')
f.write('/solve/set/equations/ke yes                \n')
f.write('/solve/monitors/residual/convergence-criteria 0.001 0.001 0.001 0.001 1e-06 0.001 0.001 1e-06 0.001 1e-16 0.001 0.001 0.001   \n') # remove pa66 residual criteria [continutiy Vx Vy Vz Energy K Epsilon Do Cu UDS0 UDS1 UDS2 UDS3]
f.write('                    \n')

#change to Mixture_Econd run
f.write('/define/materials/change-create                     \n')
f.write('mix-gas mix-gas                                     \n')
f.write('no no no no no no no no no no no no                 \n')
f.write('yes defined-per-uds 0 user-defined "Mixture_Econd::libudf" -1        \n')
f.write('no                  \n')

f.write('/solve/set/time-step 2.0e-6                         \n')   # the capacitor discharge inside jacob's ladder is about 5 ms.
f.write('/solve/dual-time-iterate 20 42                      \n')
f.write('/solve/set/time-step 3.0e-6                         \n')
f.write('/solve/dual-time-iterate 20 30                      \n')
f.write('/solve/set/time-step 4e-6                           \n')
f.write('/solve/dual-time-iterate 200 30                     \n')
#
f.write('/file/transient-export/edit rescgns CGNS '+os.path.join(casdat_path, 'jacres1 '))
f.write('x-velocity y-velocity z-velocity uds-0-scalar udm-0 udm-1 udm-2 udm-3 udm-4 udm-5 udm-6 udm-7 udm-8 udm-9 udm-10 udm-11 udm-12 udm-13 udm-14 udm-15 udm-16 udm-17 udm-18 udm-28 udm-29 udm-30 udm-31 molef-cu temperature quit no "time-step" 5 time-step \n') # do not need to specify the zone or surface names.
#
f.write('/solve/set/time-step 4e-6                            \n')
f.write('/solve/dual-time-iterate 1000 30                     \n')
#
f.write('/file/transient-export/edit rescgns CGNS '+os.path.join(casdat_path, 'jacres1 '))
f.write('x-velocity y-velocity z-velocity uds-0-scalar udm-0 udm-1 udm-2 udm-3 udm-4 udm-5 udm-6 udm-7 udm-8 udm-9 udm-10 udm-11 udm-12 udm-13 udm-14 udm-15 udm-16 udm-17 udm-18 udm-28 udm-29 udm-30 udm-31 molef-cu temperature quit no "time-step" 20 time-step \n') # do not need to specify the zone or surface names.
#
f.write('/solve/set/time-step 4e-6                           \n')
f.write('/solve/dual-time-iterate 500 30                     \n')
#
f.write('file write-case-data "'+ os.path.join(casdat_path, 'arc_final')+ '"     \n')    # write both cas and data   
f.write('(display "The Final run is successfully finished!")                     \n')
f.write('(display "Termination")                                                 \n')
f.write('quit               \n')
f.write('exit               \n')
f.write('ok                 \n')
#
f.close()
time.sleep(1)
################################################################### submit the job ########################################################################
if sys.platform.startswith('win'):
    if GUI=='Y' or GUI=='y':
        #cmd = 'fluent 3ddp -t'+paralle_cores+' -i '+ journal         #cmd = 'fluent 3ddp -t2  -gu -driver null  -i '+ journal    #  cmd window
        cmd = 'fluent 3ddp -t'+paralle_cores+' -i '+ journal
        print('\n '+cmd)
    else:
        cmd = 'fluent 3ddp -t'+paralle_cores+' -gu -driver null -i '+ journal                                              #cmd = 'fluent 3ddp -g -i '+ journal        #  cmd window
        print('\n '+cmd)
    subprocess.call(cmd)
elif sys.platform.startswith('linux'):
    cmd='sbatch '+sbatchfilename
    p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # p = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE)
    out, err = p.communicate()
    out = out.decode('utf8')
    job = out.strip().split()[-1]   # this is the current jobID
    print ('\n The '+sbatchfilename+' with jobID '+ str(job).strip(r'b\'')+' has been submitted. \n')

    while True:
        prefixed = [file for file in os.listdir('.') if file.startswith("output_fluent_")]
        if prefixed: break 
        time.sleep(0.1)
    #
    print(' File:'+prefixed[0]+' will be opened and printed on the screen...')
    time.sleep(2)    
    outputs=open(prefixed[0],'r')
    def readflow(filename):
        filename.seek(0,2)            # read the file from the tail
        while True:
            line = filename.readline()
            if not line:
                time.sleep(0.1)
                continue
            yield line                # this is very tricky
    lines=readflow(outputs)
    for line in lines:  
        line=line.rstrip()
        if line: print(line)          # end default is /n, which will cause am empty line. if the line is empty, line is false
        if 'Termination' in line or 'Error' in line or 'CANCELLED' in line:   
            print("\n Fluent Job is done or Error or CANCELLED. Please check it!")
            break
    outputs.close()        
    time.sleep(1)       
else:
    print ('\n Unable to check the operation system, exit soon...')
    time.sleep(1)  
    sys.exit()
time.sleep(1)


'''
os.remove('*./.sh')
os.remove(udf_c_code2)
os.remove(udf_h_code1)
'''
# if there is any problem, email huojindong@gmail.com

timeseconds2 = time.time() - start_time
hour   = int(timeseconds2/3600)
minute = int((timeseconds2-hour*3600)/60)
second = int(timeseconds2-3600*hour-60*minute)
print ('\n Fluent job totally takes : %02d:%02d:%02d time. \n'  %(hour, minute, second))
input ('\n Press anykey to exit...')
sys.exit()

# Multiphyscs modelling of thermal plasma: Scripting + Fluent Solver + UDF


## Contributor
Jindong Huo, main contributor. Great thanks to Eirc Lab, UConn.

## Reference
* Please refer to below papers for more
    * https://aip.scitation.org/doi/10.1063/5.0012159
    * https://doi.org/10.1093/pnasnexus/pgac129
    * https://iopscience.iop.org/article/10.1088/1361-6463/abc64b/meta
    * https://doi.org/10.1063/1.5090867


* For thermally non-equilibrium state, additional electron energy equation is needed to solve electron temperature which is different from heavy particles temperature. 
* For chemical non-equilibrium, additional species transport equations are needed to capture the gas dissociation/ionization process (normally for argon, nitrogn, not for air because air has too many reaction species).

## Description
This project can launch in both Windows and Linux System. It is script-based modelling setup, instead of GUI, to have an effecient debugging as well as parameter study/optimization. Debugging for Physics-based modelling requires calibration from experimental results, not just the Syntax issues. The scripting-based modelling should be executed in batch mode if need.
* The UDF code, main_udf.c, is parallized.
* The python script first checks the platform, Linux or Windoes, then writes the Fluent journal file, and then run fluent journal file to compile UDF and setup the model.
* The fluent journal file will read the mesh, load UDF, import/interpolate the gas transport/thermodynamic properties and finally submit the fluent job.
* During the running, the script will export the simulation results as figures, like current denstiy, temperature and potential field, at end of each timestep.
* After the job is done, we can combine the results figures into a video by running video_from_image.py, which is very convenient, becasue hardly can a 3rd-party post-process software load simulaiton results more than 100GB (too slow).  

## Physcial process
For the mulitphyscis modelling of thermal plasma, we did secondery development for Fluent Solver by UDF, which includes the following assumptions/physcis.

* Assumption: Fully ionizated plasma in both local thermodynamic and chemcial equilbrium.
* Collision-dominated collective behavior: continuum thermodynamics/fluid description.
* Electromagnetics field: charge conservation equation + magnetic vector poisson equation
* Surface chemical reaction and Species tranprot
* Heat transfer and Thermal radiation based on DOM method
* 3D transient

## Requirements
* Please ensure you have properly installed Fluent (we tested it in fluent 2021R1) and python 3 in your platform.
* In windows, it needs the C compiler installed. It seems latest ANSYS Fluent has in-built compiler, thus no additional compiler is needed. 
* In windows, modify the environment by clicking C:\Program Files\ANSYS Inc\v211\fluent\ntbin\win64\setenv.exe to add fluent to PATH 
* In linux, please export gcc as compiler
* Suggest shared-memory for parallel computing (multiple cores/threads with single virtual memory address space)

## Steps to run:
* please download the input data from the links in udfcode/download1.txt and udfcode/download2.txt. And put them in the udfcode directory. Ignore the warnning "too big to load" or "login dropbox account", and you can download without any issues.
* IF you want to create your own geometry mesh in whatever format, please use Fluent open it, check the cell zone or surface name/ID and them save as .cas file, and put this file into udfcode folder
* If you have your own geometry, keep in mind to modify the python_script.py file and replace the cell zone and boundary names you defined
* Test the code by running a few time-steps: like "/solve/dual-time-iterate 2 30"
* If initialization works well, then run it for more time-steps
* In summary, the one commond to run the whole simualtion in both linux and windows CMD/PowerShell is:
  
  **$ python python_script.py**

If you have any questions, please leave comments or email huojindong@gmail.com

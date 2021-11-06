# Multiphyscs-modelling-of-thermal-plasma-in-Fluent-Solver

## Description
This code can run in both Windows or Linux. This is script based modelling, without GUI, to have effecient debugging and parameter study.
* The UDF code is parallized
* The python script first check the platform, Linux or Windoes, then write the Fluent journal file, and then run fluent journal file.
* The fluent journal file will read the meah, compile UDF, import the gas properties data and finally submit the fluent job.
* During the running, the script will export the simulation results, like current denstiy distribution contour, at end of each timestep.


This code project is for the mulitphyscis modelling of thermal plasma, as the secondery development for Fluent Solver, which includes the following physcis


* Assumed fully ionizated plasma in both local thermodybnamics and chemcial equilbrium. 
* Gas dynamics
* Electromagnetics field: charge conservation equation + magnetic vector possion equation
* Surface chemical reaction and Species tranprot
* Heat transfer and Thermal radiation based on DOM method

## Requirement
* Please ensure you have properly installed Fluent and python 3 in your platform.
* In windows, it needs the C compiler installed. 
* In windows, modify the environment by clicking C:\Program Files\ANSYS Inc\v211\fluent\ntbin\win64\setenv.exe 
* In linux, please export gcc as compiler
* The author suggests shared-memory for parallel computing (single node/cpu with multiple cores)

## Executing program
* Create your geometry mesh in whatever format that can be read by Fluent, then open by fluent, modify the python_script.py file and replace the cell zone and boundary names
* Then save the mesh file into geometry_mesh.cas, and put this file into src_resources folder
* Test the code by running 3 steps: like /solve/dual-time-iterate 2 30, which means run 2 steps and each step with maximum 30 iterations
* If initialization works well, it can run for much longer time
* In summary, the one commond to run the whole simualtion is: **python python_script.py**

## Authors
Jindong Huo

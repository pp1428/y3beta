#!/bin/bash
#
# SCRIPT: buildResourceFile
# AUTHOR: Lupescu Grigore
# DATE:   Nov 2010
# REV:    0.0.4v
#
# PLATFORM: Linux
#
# PURPOSE: searches for cpu/gpu units firstly by aticonfig & nvidia-config (for gpus)
#         	/alternatively by searching lspci (for gpus) and /proc/cpuinfo (for cpus)
#			and then comparing against known models. If a model is known it displays all information
#			it has regarding that model. At this point only Intel CPUs and NVIDIA/AMD(ATI) GPUs are in database. 
#          
 
##########################################################
########### DEFINE FILES AND VARIABLES HERE ##############
##########################################################


######## general files & notations

folderCPU_INTEL="cpu_intel"
folderCPU_AMD="cpu_amd"

folderGPU_NVIDIA="gpu_nvidia"
folderGPU_AMD="gpu_amd"

folderCPU=""
folderGPU=""

genericModelList="family"
fileModelList=""
fileCPU=""
fileGPU=""

######### CPU variables & filenames

cpuInfo=""
cpuVendor=""
cpuFamily=""
cpuModel=""
cpuGflops="5"
cpuCores=""


######### GPU variables & filenames


gpuInfo=""
gpuVendor=""
gpuFamily=""
gpuModel=""
gpuGflops="0"
gpuCores=""
gpuMemBand=""
gpuMemInt=""
gpuCodename=""
gpuOpenCL=""

##########################################################
############### DEFINE FUNCTIONS HERE ####################
##########################################################




##########################################
# get CPU
##########################################

function getCPU
{

#newline
	
########## test for INTEL/AMD CPU 

	cpuInfo_INTEL=$(cat /proc/cpuinfo | grep -i INTEL | head -n 1)
	cpuInfo_AMD=$(cat /proc/cpuinfo | grep -i AMD | head -n 1)
	cpuCores=$(cat /proc/cpuinfo | grep -i cores | head -n 1| sed 's/cpu[ \t]*cores[ \t]*://g' )


########## find family Model

	if [ -n "$cpuInfo_INTEL" ]; then
		fileModelList="${folderCPU_INTEL}/$genericModelList"
		folderCPU=$folderCPU_INTEL
		cpuVendor="INTEL"

	elif [ -n "$cpuInfo_AMD" ]; then
		fileModelList="${folderCPU_AMD}/$genericModelList"
		folderCPU=$folderCPU_AMD
		cpuVendor="AMD"
	else 
		cpuVendor="N/A"
	fi


########## find specific model

	while read fileLine
		do
			cpuModel=$(cat /proc/cpuinfo | grep name |grep -i $fileLine | head -n 1)
			if [ -n "$cpuModel" ]; then
				fileCPU="${folderCPU}/${fileLine}.csv"
				cpuFamily=$fileLine
				cpuInfo=$cpuModel
				break	#found it
			fi
			:
		done < $fileModelList


########## extract characteristics

		if [ -n "$cpuFamily" ]; then
				while read fileLine
					do
						cpuModelLine=$(echo $fileLine | cut -d\, -f1)
						for infoModel in $cpuModelLine
							do
								if [ -n "$(echo $cpuInfo | grep $infoModel)" ]; then
									cpuModel=$infoModel
									cpuGflops=$(echo $fileLine | cut -d\, -f2)
								else 
									cpuModel=""
									break
								fi
							done
		
						if [ -n "$cpuModel" ]; then
							cpuModel=$cpuModelLine
							cpuGflops=$(echo $fileLine | cut -d\, -f2)
							break
						fi
							:
					done < $fileCPU
		else
			cpuFamily="N/A"
			cpuModel="N/A"
		fi



########## display characteristics


	# no arguments / all info displayed
	if [ $# -eq 0 ]; then 
		if [ -n "$cpuModel" ]; then
			echo "CPU Vendor : $cpuVendor"
			echo "CPU Family : $cpuFamily"
			echo "CPU Model : $cpuModel"
			echo "CPU Gflops : $cpuGflops"
			echo "CPU Cores : $cpuCores"
		else
			echo "CPU Vendor : $cpuVendor"
			echo "CPU Family : $cpuFamily"
			echo "CPU Model : N/A"
			echo "CPU Gflops : N/A"
			echo "CPU Cores : $cpuCores"
		fi
		echo ""

	elif [ "$@" == "-gui" ]; then
		echo -e "\n#UNIT"
		echo "$cpuVendor"
		echo "$cpuModel"
		echo "$cpuCores"
		echo "$cpuGflops"
		echo "yes"
	
	elif [ "$@" == "-cstd" ]; then
		echo "$cpuVendor $cpuModel $cpuGflops"
	
	elif [ "$@" == "-cvendor" ]; then
		echo $cpuVendor
	
	elif [ "$@" == "-cmodel" ]; then
		echo $cpuModel
	
	elif [ "$@" == "-cgflops" ]; then
		echo $cpuGflops

	elif [ "$@" == "-ccores" ]; then
		echo $cpuCores

	fi

	
}




##########################################
# get GPU
##########################################

function getGPU
{
	
########## retrieve NVIDIA/AMD GPU 

	# get model by NVIDIA/AMD GPU driver utility (drivers installed)
	gpuInfo_NVIDIA_D=$( nvidia-xconfig --query-gpu-info 2>/dev/null | grep -i NVIDIA | head -n 1)
	gpuInfo_ATI_D=$( aticonfig --list-adapters 2>/dev/null | grep ATI | head -n 1)

	# get model by query with lspci
	gpuInfo_NVIDIA=$( lspci | grep -i NVIDIA 2>/dev/null | head -n 1)
	gpuInfo_AMD=$( lspci | grep -i AMD 2>/dev/null | head -n 1)
	gpuInfo_ATI=$( lspci | grep ATI 2>/dev/null | head -n 1)



########## find family Model

	if [ -n "$gpuInfo_NVIDIA_D" ]; then
		folderGPU=$folderGPU_NVIDIA
		fileModelList="${folderGPU_NVIDIA}/${genericModelList}"
		gpuInfo=$gpuInfo_NVIDIA_D
		gpuVendor="NVIDIA"
		

	elif [ -n "$gpuInfo_NVIDIA" ]; then
		folderGPU=$folderGPU_NVIDIA
		fileModelList="${folderGPU_NVIDIA}/${genericModelList}"
		gpuInfo=$gpuInfo_NVIDIA
		gpuVendor="NVIDIA"
		

	elif [ -n "$gpuInfo_ATI_D" ]; then
		folderGPU=$folderGPU_AMD
		fileModelList="${folderGPU_AMD}/${genericModelList}"
		gpuInfo=$gpuInfo_ATI_D
		gpuVendor="AMD/ATI"
		

	elif [ -n "$gpuInfo_ATI" ]; then
		folderGPU=$folderGPU_AMD
		fileModelList="${folderGPU_AMD}/${genericModelList}"
		gpuInfo=$gpuInfo_ATI
		gpuVendor="AMD/ATI"

	elif [ -n "$gpuInfo_AMD" ]; then
		folderGPU=$folderGPU_AMD
		fileModelList="${folderGPU_AMD}/${genericModelList}"
		gpuInfo=$gpuInfo_AMD
		gpuVendor="AMD/ATI"
		
	fi


########## find specific model

	while read fileLine
		do
			gpuModel=$( echo $gpuInfo | grep -i $fileLine | head -n 1)
			if [ -n "$gpuModel" ]; then
				fileGPU="${folderGPU}/${fileLine}.csv"
				gpuFamily=$fileLine
				gpuInfo=$gpuModel
				break	#found it
			fi
			:
		done < $fileModelList


########## extract characteristics

		if [ -n "$gpuFamily" ]; then

				# read each line from gpu family file (eg mobility)
				while read fileLine
					do
						gpuModelLine=$(echo $fileLine | cut -d\, -f1)

						# for each word in a line try and match with system info
						for infoModel in $gpuModelLine
							do
								if [ -n "$(echo $gpuInfo | grep $infoModel)" ]; then
									gpuModel=$infoModel
									gpuGflops=$(echo $fileLine | cut -d\, -f2)
								else 
									gpuModel=""
									break
								fi
							done
		
						#if perfect match (all words match) model is considered found
						if [ -n "$gpuModel" ]; then
							gpuModel=$gpuModelLine
							break
						fi
							:
					done < $fileGPU

				#if model found fill in characteristics
				if [ -n "$gpuModel" ]; then
					gpuGflops=$(echo $fileLine | cut -d\, -f2)
					gpuCodename=$(echo $fileLine | cut -d\, -f3)
					gpuCores=$(echo $fileLine | cut -d\, -f4)
					gpuMemBand=$(echo $fileLine | cut -d\, -f5)
					gpuMemInt=$(echo $fileLine | cut -d\, -f6)
					gpuOpenCL=$(echo $fileLine | cut -d\, -f7)

				else
					gpuModel="N/A"
					gpuGflops="0"
					gpuCodename="N/A"
					gpuCores="N/A"
					gpuMemBand="N/A"
					gpuMemInt="N/A"
				fi

		else
			gpuFamily="N/A"
			gpuModel="N/A"
		fi



########## display characteristics


	if [ $# -eq 0 ]; then
		if [ -n "$gpuModel" ]; then
			echo "GPU Vendor : $gpuVendor"
			echo "GPU Family : $gpuFamily"
			echo "GPU Model : $gpuModel"
			echo "GPU Codename : $gpuCodename"
			echo "GPU Gflops : $gpuGflops"
			echo "GPU OpenCL : $gpuOpenCL"
			echo "GPU Cores-TMU-ROP : $gpuCores"
			echo "GPU Mem Bandwidth: $gpuMemBand"
			echo "GPU Mem Interface: $gpuMemInt"
		else
			echo "GPU Vendor : $gpuVendor"
			echo "GPU Family : $gpuFamily"
			echo "GPU Model : N/A"
			echo "GPU Gflops: N/A"
			echo "GPU OpenCL : N/A"
		fi
	echo ""

	elif [ "$@" == "-gui" ]; then
		echo -e "\n #UNIT"
		echo "$gpuVendor"
		echo "$gpuModel"
		echo "$gpuCores"
		echo "$gpuGflops"
		echo "$gpuOpenCL"
	
	elif [ "$@" == "-gstd" ]; then
		echo "$gpuVendor $gpuModel $gpuGflops"
	
	elif [ "$@" == "-gvendor" ]; then
		echo $gpuVendor
	
	elif [ "$@" == "-gmodel" ]; then
		echo $gpuModel
	
	elif [ "$@" == "-ggflops" ]; then
		echo $gpuGflops

	elif [ "$@" == "-gcores" ]; then
		echo $gpuCores
	
	elif [ "$@" == "-gopencl" ]; then
		echo $gpuOpenCL

	fi
		
}


##########################################
# display info
##########################################

function information
{
	if [ "$@" == "-info" ]; then
			echo ""
			echo "clinfo @2010 Lupescu Grigore"
			echo "cpu-gpu OpenCL basic information"
			echo "Command List"
			echo ""
			echo "CPU/GPU complete info:"
			echo "./clinfo "
			echo ""
			echo "CPU/GPU GUI info:"
			echo "./clinfo -gui"
			echo ""
			echo "CPU info :"
			echo "./clinfo -cstd"
			echo "./clinfo -cinfo"
			echo "./clinfo -cvendor"
			echo "./clinfo -cgflops"
			echo "./clinfo -ccores"
			echo ""
			echo "GPU info :"
			echo "./clinfo -gstd"
			echo "./clinfo -ginfo"
			echo "./clinfo -gvendor"
			echo "./clinfo -ggflops"
			echo "./clinfo -gcores"
		fi
}




##########################################################
################ BEGINNING OF MAIN #######################
##########################################################

#examine CPU(s)
if [ $# -eq 0 ]; then
	getCPU
	getGPU
else
	information $@
	getCPU $@
	getGPU $@
fi


# End of script

#!/bin/sh

export CROSS_COMPILE=/os/armcc/bin/aarch64-none-linux-gnu-
export ARCH=arm64

KPATH=$2

build() {
	case $1 in
		cfg)
			cd $KPATH
			make menuconfig
			echo "------------------------------ Move Configuration ------------------------------"
			if [ -d "out" ];then
				mv .config out/
			else
				mkdir out
				mv .config out/
			fi
			echo "------------------------------ Done Configuration ------------------------------"
			;;
		out)
			cd $KPATH

			echo "------------------------------- Start Compalition ------------------------------"
			start=$(date +%s)
			make -j88 O=out/
			end=$(date +%s)
			total=$(($end-$start))
			echo "--------------------------------------------------------------------------------"
			echo "Done compiling kernel in $(($total/60)) min $(($total%60)) sec"
			echo "------------------------------- Done Compalition -------------------------------"
			;;
		tags)
			cd $KPATH

			echo "--------------------------- Start Generating tags ------------------------------"
			start=$(date +%s)
			ctags=$(command -v ctags)
			if [ -z $ctags ];then
				echo "ctags ain't installed yet"
				TAGS=
			else
				TAGS=tags
			fi
			cscope=$(command -v cscope)
			if [ -z $cscope ];then
				echo "cscope ain't installed yet"
				CSCOPE=
			else
				CSCOPE=cscope
			fi
			echo "--------------------------------------------------------------------------------"
			make -j88 $TAGS $CSCOPE
			end=$(date +%s)
			total=$(($end-$start))
			echo "--------------------------------------------------------------------------------"
			echo "Done generating tags in $(($total/60)) min $(($total%60)) sec"
			echo "--------------------------------------------------------------------------------"
			;;
		mrp)
			echo "------------------------ Remove Previous Configuration -------------------------"
			cd $KPATH
			start=$(date +%s%N)
			make mrproper
			end=$(date +%s%N)
			echo "mrproper took $(($(($end-$start))/1000000)) ms"
			echo "------------------------------ Done Configuration ------------------------------"
			;;
		cln)
			echo "----------------------------------- Make Clean ---------------------------------"
			cd $KPATH
			start=$(date +%s)
			make clean
			end=$(date +%s)
			total=$(($end-$start))
			echo "Done cleaning in $(($total/60)) min $(($total%60)) sec"
			echo "----------------------------------- Done Clean ---------------------------------"
			;;
		*)
			echo "Usage: ./compilation <option> [path]"
			echo "--------------------------------------------------------------------------------"
			echo "[0] generate tags:      ./compilation.sh tags [path]"
			echo "[1] create .config:     ./compilation.sh cfg  [path]"
			echo "[2] compile the kernel: ./compilation.sh out  [path]"
			echo "[3] mrproper:           ./compilation.sh mrp  [path]"
			echo "[4] clean:              ./compilation.sh cln  [path]"
			echo "--------------------------------------------------------------------------------"
			;;
	esac
}

build $1 $2

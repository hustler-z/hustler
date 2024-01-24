export CROSS_COMPILE=/conic/armcc/bin/aarch64-none-linux-gnu-
export ARCH=arm64

KPATH=$2

build() {
	case $1 in
		config)
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
		kernel)
			cd $KPATH

			echo "------------------------------- Start Compalition ------------------------------"
			start=$(date +%s)
			make -j88 O=out/
			end=$(date +%s)
			total=$[ $end-$start ]
			echo "--------------------------------------------------------------------------------"
			echo "The Compilation Time of the kernel: $(($total/60)) min $(($total%60)) sec"
			echo "------------------------------- Done Compalition -------------------------------"
			;;
        mrproper)
			echo "------------------------ Remove Previous Configuration -------------------------"
            cd $KPATH
			make mrproper
			echo "------------------------------ Done Configuration ------------------------------"
            ;;
        clean)
			echo "----------------------------------- Make Clean ---------------------------------"
            cd $KPATH
			make clean
			echo "----------------------------------- Done Clean ---------------------------------"
            ;;
		*)
			echo "./compilation.sh [config/kernel/mrproper/clean] [path]"
			;;
	esac
}

build $1 $2

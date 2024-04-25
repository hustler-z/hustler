file(REMOVE_RECURSE
  "../../radaxzero3/armv8/FPGA/libRADAXZERO3.a"
  "../../radaxzero3/armv8/FPGA/libRADAXZERO3.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang ASM C)
  include(CMakeFiles/RADAXZERO3.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()

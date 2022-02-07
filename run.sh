#!/bin/zsh
set -e
set -x
function die {
	echo "$1" >&2
	exit 1
}

#update file locations:
file=/yotta/pbwt/durbin.txt

function task {
	echo "Threshold: $threshold"
	# ./pbwt.x -t "$threshold" -i /yotta/pbwt/durbin.txt
	./pbwt.x -i "$file" -t "$threshold" -d $file.$threshold.divarray -h $file.$threshold.grammar.txt -m $file.$threshold.matrix -n $file.$threshold.intervals
	#-w 2000
	~/code/bigrepair/bigrepair $file.$threshold.matrix
	~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.matrix -f Bigrepair -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.matrix.slp
	~/code/bigrepair/bigrepair -i $file.$threshold.divarray
	# ~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.divarray -f Bigrepair -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.divarray.slp
	cargo run --manifest-path=transform_grammar/Cargo.toml -- -i $file.$threshold.grammar.txt  -o $file.$threshold.grammar.bin
	~/code/shapedSlp/build/SlpEncBuild -i $file.$threshold.grammar.bin -f solca -e ShapedSlpV2_Sd_SdMcl -o $file.$threshold.grammar.slp
}


make

mkdir -p log

for threshold in 0.1 0.08 0.05 0.03 0.01; do
		task 2>&1 | tee log/durbin.$threshold.log  &
done
wait


# PlainSlp_32Fblc PlainSlp_FblcFblc PlainSlp_IblcFblc SelfShapedSlpV2_SdSd_Sd SelfShapedSlp_SdSd_Mcl SelfShapedSlp_SdSd_Sd ShapedSlpV2_Sd_SdMcl ShapedSlp_SdMclSd_SdMcl ShapedSlp_SdSdSd_SdMcl ShapedSlp_Status_SdMclSd_SdMcl
